#include "track3dviewport.h"

#include "cascadeassembler.h"
#include "mcdriverobj.h"
#include "trackdatachannel.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <unordered_map>

#include <QDebug>
#include <QHideEvent>
#include <QMouseEvent>
#include <QOpenGLShaderProgram>
#include <QShowEvent>
#include <QSurfaceFormat>
#include <QWheelEvent>
#include <QtMath>

// scene vertex: pos (loc 0) + rgba color (loc 1), interleaved
static const int kSceneFloats = 7;
static const int kStride = kSceneFloats * sizeof(float);

static const int kTrackVboBytes = 96 * 1024 * 1024;
static const int kTrackVboVerts = kTrackVboBytes / int(sizeof(TrackVertex));
static const double kPlaybackSeconds = 2.0; // wall time for one 0->1 sweep

static const char *kVertSrc = R"(#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec4 aColor;
uniform mat4 uMvp;
out vec4 vColor;
void main() {
    gl_Position = uMvp * vec4(aPos, 1.0);
    vColor = aColor;
}
)";

static const char *kFragSrc = R"(#version 330 core
in vec4 vColor;
out vec4 fragColor;
void main() { fragColor = vColor; }
)";

static void putVertex(std::vector<float> &v, const QVector3D &p, float r, float g, float b, float a)
{
    v.push_back(p.x());
    v.push_back(p.y());
    v.push_back(p.z());
    v.push_back(r);
    v.push_back(g);
    v.push_back(b);
    v.push_back(a);
}

// 8 corners of an axis-aligned box, indexed by (x,y,z) bits
static void corners(const QVector3D &lo, const QVector3D &hi, QVector3D c[8])
{
    c[0] = { lo.x(), lo.y(), lo.z() };
    c[1] = { hi.x(), lo.y(), lo.z() };
    c[2] = { hi.x(), hi.y(), lo.z() };
    c[3] = { lo.x(), hi.y(), lo.z() };
    c[4] = { lo.x(), lo.y(), hi.z() };
    c[5] = { hi.x(), lo.y(), hi.z() };
    c[6] = { hi.x(), hi.y(), hi.z() };
    c[7] = { lo.x(), hi.y(), hi.z() };
}

static void appendBoxEdges(std::vector<float> &v, const QVector3D &lo, const QVector3D &hi,
                           const QColor &col)
{
    QVector3D c[8];
    corners(lo, hi, c);
    static const int e[12][2] = { { 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 0 }, { 4, 5 }, { 5, 6 },
                                  { 6, 7 }, { 7, 4 }, { 0, 4 }, { 1, 5 }, { 2, 6 }, { 3, 7 } };
    for (auto &ei : e) {
        putVertex(v, c[ei[0]], col.redF(), col.greenF(), col.blueF(), col.alphaF());
        putVertex(v, c[ei[1]], col.redF(), col.greenF(), col.blueF(), col.alphaF());
    }
}

static void appendBoxFaces(std::vector<float> &v, const QVector3D &lo, const QVector3D &hi,
                           const QColor &col)
{
    QVector3D c[8];
    corners(lo, hi, c);
    static const int f[6][4] = { { 0, 1, 2, 3 }, { 4, 5, 6, 7 }, { 0, 1, 5, 4 },
                                 { 3, 2, 6, 7 }, { 0, 3, 7, 4 }, { 1, 2, 6, 5 } };
    const float r = col.redF(), g = col.greenF(), b = col.blueF(), a = col.alphaF();
    for (auto &fi : f) {
        putVertex(v, c[fi[0]], r, g, b, a);
        putVertex(v, c[fi[1]], r, g, b, a);
        putVertex(v, c[fi[2]], r, g, b, a);
        putVertex(v, c[fi[0]], r, g, b, a);
        putVertex(v, c[fi[2]], r, g, b, a);
        putVertex(v, c[fi[3]], r, g, b, a);
    }
}

Track3DViewport::Track3DViewport(McDriverObj *driver, QWidget *parent)
    : QOpenGLWidget(parent), driver_(driver)
{
    QSurfaceFormat fmt;
    fmt.setVersion(3, 3);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    fmt.setDepthBufferSize(24);
    fmt.setSamples(4);
    setFormat(fmt);

    setMinimumSize(320, 240);
    setFocusPolicy(Qt::StrongFocus);

    // capture follows the 3D tab; flush publishes the tail cascade after a run
    channel_ = new TrackDataChannel(this);
    driver_->setEventHandler(TrackDataChannel::onEvent, TrackDataChannel::eventMask(), channel_);
    connect(channel_, &TrackDataChannel::cascadeReady, this, &Track3DViewport::onCascadeReady,
            Qt::QueuedConnection);

    connect(driver_, &McDriverObj::simulationStarted, this,
            [this](bool running) {
                if (!running)
                    channel_->flush();
            },
            Qt::QueuedConnection);

    connect(driver_, &McDriverObj::configChanged, this, &Track3DViewport::refreshScene);
    connect(driver_, &McDriverObj::simulationCreated, this, &Track3DViewport::refreshScene);
}

Track3DViewport::~Track3DViewport()
{
    // don't touch driver_: it is deleteLater'd with the runner thread before
    // this widget dies, and the handler needs no unregistering (sim is stopped)
    if (prog_ || boxVbo_.isCreated()) {
        makeCurrent();
        delete prog_;
        prog_ = nullptr;
        delete trackProg_;
        trackProg_ = nullptr;
        boxVbo_.destroy();
        regVbo_.destroy();
        boxVao_.destroy();
        regVao_.destroy();
        trackVbo_.destroy();
        trackVao_.destroy();
        doneCurrent();
    }
}

void Track3DViewport::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0.10f, 0.10f, 0.12f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    prog_ = new QOpenGLShaderProgram(this);
    if (!prog_->addShaderFromSourceCode(QOpenGLShader::Vertex, kVertSrc)
        || !prog_->addShaderFromSourceCode(QOpenGLShader::Fragment, kFragSrc)
        || !prog_->link())
        qWarning() << "Track3DViewport: shader build failed:" << prog_->log();

    boxVao_.create();
    regVao_.create();
    boxVbo_.create();
    regVbo_.create();

    trackProg_ = new QOpenGLShaderProgram(this);
    if (!trackProg_->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/track.vert")
        || !trackProg_->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/track.frag")
        || !trackProg_->link())
        qWarning() << "Track3DViewport: track shader build failed:" << trackProg_->log();

    trackVao_.create();
    trackVbo_.create();
    trackVao_.bind();
    trackVbo_.bind();
    trackVbo_.allocate(kTrackVboBytes);
    const int stride = int(sizeof(TrackVertex));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride,
                          reinterpret_cast<void *>(offsetof(TrackVertex, x)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, stride,
                          reinterpret_cast<void *>(offsetof(TrackVertex, energy)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, stride,
                          reinterpret_cast<void *>(offsetof(TrackVertex, t)));
    glEnableVertexAttribArray(3);
    glVertexAttribIPointer(3, 1, GL_SHORT, stride,
                           reinterpret_cast<void *>(offsetof(TrackVertex, rid)));
    trackVbo_.release();
    trackVao_.release();

    readSceneFromConfig();
    sceneDirty_ = true;
}

void Track3DViewport::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
}

void Track3DViewport::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (sceneDirty_)
        buildSceneBuffers();
    if (tracksDirty_) {
        rebuildTrackBuffer();
        tracksDirty_ = false;
    }
    if (!prog_ || !prog_->isLinked())
        return;

    prog_->bind();
    prog_->setUniformValue("uMvp", mvp());

    // transparent regions first, without writing depth
    if (regVertices_ > 0) {
        glDepthMask(GL_FALSE);
        regVao_.bind();
        glDrawArrays(GL_TRIANGLES, 0, regVertices_);
        regVao_.release();
        glDepthMask(GL_TRUE);
    }

    if (boxVertices_ > 0) {
        boxVao_.bind();
        glDrawArrays(GL_LINES, 0, boxVertices_);
        boxVao_.release();
    }

    prog_->release();

    if (trackProg_ && trackProg_->isLinked() && !first_.empty()) {
        trackProg_->bind();
        trackProg_->setUniformValue("uMvp", mvp());
        trackProg_->setUniformValue("uTime", playbackTime());
        trackVao_.bind();
        glMultiDrawArrays(GL_LINE_STRIP, first_.data(), count_.data(), int(first_.size()));
        trackVao_.release();
        trackProg_->release();
    }

    if (playing_)
        update();
}

void Track3DViewport::showEvent(QShowEvent *e)
{
    QOpenGLWidget::showEvent(e);
    viewActive_ = true;
    updateCapture();
    readSceneFromConfig();
    if (!viewInitialized_) {
        viewInitialized_ = true;
        homeView();
    } else {
        update();
    }
}

void Track3DViewport::hideEvent(QHideEvent *e)
{
    QOpenGLWidget::hideEvent(e);
    viewActive_ = false;
    updateCapture();
}

void Track3DViewport::updateCapture()
{
    // capture while the tab is visible, unless a fixed batch is already full
    const bool full = mode_ == Batch && int(residents_.size()) >= nCascades_;
    channel_->setCapturing(viewActive_ && !full);
}

void Track3DViewport::readSceneFromConfig()
{
    if (!driver_)
        return;

    const mcconfig &opt = driver_->options();
    const auto &t = opt.Target;

    boxMin_ = QVector3D(t.origin.x(), t.origin.y(), t.origin.z());
    boxMax_ = boxMin_ + QVector3D(t.size.x(), t.size.y(), t.size.z());

    std::unordered_map<std::string, size_t> mmap; // material_id -> index
    for (size_t i = 0; i < t.materials.size(); ++i)
        mmap[t.materials[i].id] = i;

    regions_.clear();
    for (const auto &r : t.regions) {
        auto it = mmap.find(r.material_id);
        QColor c = (it == mmap.end())
                ? QColor(Qt::gray)
                : QColor(QString::fromStdString(t.materials[it->second].color));
        if (!c.isValid())
            c = QColor(Qt::gray);
        QVector3D x0(r.origin.x(), r.origin.y(), r.origin.z());
        QVector3D x1 = x0 + QVector3D(r.size.x(), r.size.y(), r.size.z());
        regions_.push_back({ x0, x1, c });
    }

    center_ = 0.5f * (boxMin_ + boxMax_);
    radius_ = 0.5f * (boxMax_ - boxMin_).length();
    if (radius_ < 1e-3f)
        radius_ = 1.0f;
    sceneDirty_ = true;
}

void Track3DViewport::buildSceneBuffers()
{
    std::vector<float> box, reg;

    appendBoxEdges(box, boxMin_, boxMax_, QColor::fromRgbF(0.75, 0.75, 0.78, 1.0));
    for (const RegionBox &rb : regions_) {
        QColor c = rb.color;
        c.setAlphaF(0.18); // weak transparent tint
        appendBoxFaces(reg, rb.x0, rb.x1, c);
    }

    auto upload = [this](QOpenGLVertexArrayObject &vao, QOpenGLBuffer &vbo,
                         const std::vector<float> &data) {
        vao.bind();
        vbo.bind();
        vbo.allocate(data.data(), int(data.size() * sizeof(float)));
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, kStride, nullptr);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, kStride,
                              reinterpret_cast<void *>(3 * sizeof(float)));
        vbo.release();
        vao.release();
    };

    upload(boxVao_, boxVbo_, box);
    upload(regVao_, regVbo_, reg);
    boxVertices_ = int(box.size()) / kSceneFloats;
    regVertices_ = int(reg.size()) / kSceneFloats;
    sceneDirty_ = false;
}

QVector3D Track3DViewport::eye() const
{
    const float y = qDegreesToRadians(yaw_);
    const float p = qDegreesToRadians(pitch_);
    QVector3D dir(std::cos(p) * std::sin(y), std::sin(p), std::cos(p) * std::cos(y));
    return center_ + dist_ * dir;
}

QMatrix4x4 Track3DViewport::mvp() const
{
    const float aspect = height() > 0 ? float(width()) / float(height()) : 1.0f;
    QMatrix4x4 proj;
    proj.perspective(45.0f, aspect, qMax(0.05f, radius_ * 0.05f), dist_ + radius_ * 4.0f);
    QMatrix4x4 view;
    view.lookAt(eye(), center_, QVector3D(0, 1, 0));
    return proj * view;
}

void Track3DViewport::fitView()
{
    // fit the scene radius into the 45 deg fov, with margin
    dist_ = radius_ / std::tan(qDegreesToRadians(22.5f)) * 1.15f;
}

void Track3DViewport::homeView()
{
    yaw_ = 45.0f;
    pitch_ = 30.0f;
    center_ = 0.5f * (boxMin_ + boxMax_);
    fitView();
    update();
}

void Track3DViewport::setPresetView(int v)
{
    switch (v) {
    case Front: yaw_ = 0.0f; pitch_ = 0.0f; break;
    case Back: yaw_ = 180.0f; pitch_ = 0.0f; break;
    case Top: yaw_ = 0.0f; pitch_ = 89.0f; break;
    case Bottom: yaw_ = 0.0f; pitch_ = -89.0f; break;
    case Left: yaw_ = -90.0f; pitch_ = 0.0f; break;
    case Right: yaw_ = 90.0f; pitch_ = 0.0f; break;
    case Iso: default: yaw_ = 45.0f; pitch_ = 30.0f; break;
    }
    update();
}

void Track3DViewport::refreshScene()
{
    readSceneFromConfig();
    update();
}

void Track3DViewport::onCascadeReady()
{
    for (const auto &c : channel_->takeCascades())
        addCascade(c);
}

void Track3DViewport::addCascade(const std::shared_ptr<const Cascade> &c)
{
    if (!c || c->buff.empty())
        return;
    if (int(c->buff.size()) > kTrackVboVerts) {
        qWarning() << "Track3DViewport: cascade too large to draw:" << c->buff.size();
        return;
    }

    residents_.push_back(c);

    if (mode_ == Ring) {
        while (int(residents_.size()) > nCascades_)
            residents_.pop_front();
    } else {
        while (int(residents_.size()) > nCascades_)
            residents_.pop_back();
    }

    // stay within the VBO capacity
    int total = 0;
    for (const auto &r : residents_)
        total += int(r->buff.size());
    while (total > kTrackVboVerts && residents_.size() > 1) {
        total -= int(residents_.front()->buff.size());
        residents_.pop_front();
    }

    updateCapture();
    tracksDirty_ = true;
    update();
}

void Track3DViewport::rebuildTrackBuffer()
{
    first_.clear();
    count_.clear();
    tMax_ = 0.f;

    int total = 0;
    for (const auto &c : residents_)
        total += int(c->buff.size());

    std::vector<TrackVertex> all;
    all.reserve(total);
    int base = 0;
    for (const auto &c : residents_) {
        for (size_t j = 0; j < c->start_pos.size(); ++j) {
            first_.push_back(base + c->start_pos[j]);
            count_.push_back(c->length[j]);
        }
        for (const TrackVertex &v : c->buff)
            tMax_ = std::max(tMax_, v.t);
        all.insert(all.end(), c->buff.begin(), c->buff.end());
        base += int(c->buff.size());
    }

    if (!all.empty()) {
        trackVbo_.bind();
        trackVbo_.write(0, all.data(), int(all.size() * sizeof(TrackVertex)));
        trackVbo_.release();
    }
}

double Track3DViewport::currentPhase() const
{
    if (!playing_)
        return phase_;
    double p = phase_ + (clock_.elapsed() / 1000.0) / kPlaybackSeconds;
    return p - std::floor(p); // loop 0..1
}

float Track3DViewport::playbackTime() const
{
    if (!playing_ || tMax_ <= 0.f) // paused or no time data: show full tracks
        return std::numeric_limits<float>::max();
    return float(currentPhase() * tMax_);
}

void Track3DViewport::play(bool on)
{
    if (on == playing_)
        return;
    if (on) {
        phase_ = 0.0;
        clock_.restart();
        playing_ = true;
    } else {
        phase_ = currentPhase();
        playing_ = false;
    }
    update();
}

void Track3DViewport::clear()
{
    residents_.clear();
    first_.clear();
    count_.clear();
    tMax_ = 0.f;
    phase_ = 0.0;
    tracksDirty_ = false;
    clock_.restart();
    updateCapture();
    update();
}

void Track3DViewport::setNCascades(int n)
{
    n = qBound(1, n, 20);
    if (n == nCascades_)
        return;
    nCascades_ = n;
    while (int(residents_.size()) > nCascades_) {
        if (mode_ == Ring)
            residents_.pop_front();
        else
            residents_.pop_back();
    }
    updateCapture();
    tracksDirty_ = true;
    update();
}

void Track3DViewport::setRingMode(bool on)
{
    mode_ = on ? Ring : Batch;
    updateCapture();
}

void Track3DViewport::mousePressEvent(QMouseEvent *e)
{
    lastPos_ = e->pos();
}

void Track3DViewport::mouseMoveEvent(QMouseEvent *e)
{
    const int dx = e->x() - lastPos_.x();
    const int dy = e->y() - lastPos_.y();
    lastPos_ = e->pos();

    if (e->buttons() & Qt::LeftButton) {
        yaw_ -= dx * 0.3f;
        pitch_ += dy * 0.3f;
        pitch_ = qBound(-89.0f, pitch_, 89.0f);
        update();
    } else if (e->buttons() & (Qt::RightButton | Qt::MiddleButton)) {
        QMatrix4x4 view;
        view.lookAt(eye(), center_, QVector3D(0, 1, 0));
        QVector3D right(view(0, 0), view(0, 1), view(0, 2));
        QVector3D up(view(1, 0), view(1, 1), view(1, 2));
        const float s = dist_ * 0.0015f;
        center_ -= right * (dx * s);
        center_ += up * (dy * s);
        update();
    }
}

void Track3DViewport::wheelEvent(QWheelEvent *e)
{
    const float steps = e->angleDelta().y() / 120.0f;
    dist_ *= std::pow(0.9f, steps);
    dist_ = qBound(radius_ * 0.1f, dist_, radius_ * 30.0f);
    update();
}
