#include "track3dviewport.h"

#include "cascadeassembler.h"
#include "mcdriverobj.h"
#include "trackdatachannel.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
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

static float cascadeDuration(const Cascade &c)
{
    float t = 0.f;
    for (const TrackVertex &v : c.buff)
        t = std::max(t, v.t);
    return t;
}

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

CascadeRecorder::CascadeRecorder(McDriverObj *driver, QObject *parent) : QObject(parent)
{
    // capture follows the 3D tab; flush publishes the tail cascade after a run
    channel_ = new TrackDataChannel(this);
    driver->setEventHandler(TrackDataChannel::onEvent, TrackDataChannel::eventMask(), channel_);
    connect(channel_, &TrackDataChannel::cascadeReady, this, &CascadeRecorder::onCascadeReady,
            Qt::QueuedConnection);
    connect(
            driver, &McDriverObj::simulationStarted, this,
            [this](bool running) {
                if (!running) {
                    channel_->flush();
                    capture(false);
                }
            },
            Qt::QueuedConnection);
    connect(driver, &McDriverObj::simulationCreated, this, &CascadeRecorder::clear);
}

void CascadeRecorder::setPlaybackSpeed(double f)
{
    f = qBound(0.1, f, 10.0);
    if (f == clock_.speed())
        return;
    clock_.setSpeed(f);
    emit needsUpdate();
}

void CascadeRecorder::setNCascades(int n)
{
    n = qBound(1, n, 20);
    if (n == nCascades_)
        return;
    nCascades_ = n;
    stateMachine(Clear);
    emit needsUpdate();
}

void CascadeRecorder::setRingMode(bool on)
{
    mode_ = on ? Ring : Batch;
    stateMachine(Clear);
    emit needsUpdate();
}

void CascadeRecorder::stateMachine(Event e)
{
    State old_ = state_;
    switch (state_) {
    case Idle:
        switch (e) {
        case Start:
            clear_();
            clock_.start();
            channel_->setCapturing(true);
            state_ = Capturing;
            break;
        case Clear:
            clear_();
            clock_.reset();
            break;
        default:
            break;
        }
        break;
    case Capturing:
        switch (e) {
        case Stop:
            channel_->setCapturing(false);
            state_ = Finishing;
            break;
        case Pause:
            channel_->setCapturing(false);
            clock_.pause();
            state_ = Paused;
            break;
        case Update:
            if (mode_ == Batch) {
                if (int(cascade_buffer_.size()) == nCascades_) {
                    channel_->setCapturing(false);
                    state_ = Finishing;
                }
            } else {
                if (int(cascade_buffer_.size()) == nCascades_ + 1 && clock_.playbackTime() > tMax_)
                    evictOldest();
            }
            break;
        case Clear:
            clear_();
            clock_.reset();
            break;
        default:
            break;
        }
        break;
    case Paused:
        switch (e) {
        case Resume:
            clock_.resume();
            channel_->setCapturing(true);
            state_ = Capturing;
            break;
        case Stop:
            clock_.pause();
            channel_->setCapturing(false);
            state_ = Idle;
            break;
        case Clear:
            clear_();
            clock_.reset();
            break;
        default:
            break;
        }
        break;
    case Finishing:
        switch (e) {
        case Update:
            if (clock_.playbackTime() > tMax_) {
                clock_.pause();
                state_ = Idle;
            }
            break;
        case Clear:
            clear_();
            clock_.reset();
            break;
        default:
            break;
        }
        break;
    }
    if (old_ != state_)
        emit stateChange(old_, state_);
    if (e == Update)
        statusUpdate_();
    emit needsUpdate();
}

void CascadeRecorder::clear_()
{
    int nresidents_ = cascade_buffer_.size();
    cascade_buffer_.clear();
    tMin_ = 0.f;
    tMax_ = 0.f;
    tracksDirty_ = nresidents_ > 0;
}

void CascadeRecorder::onCascadeReady()
{
    for (const auto &c : channel_->takeCascades()) {
        if (!c || c->buff.empty())
            continue;
        if (int(c->buff.size()) > kTrackVboVerts) {
            qWarning() << "Track3DViewport: cascade too large to draw:" << c->buff.size();
            continue;
        }
        admitCascade(c);
    }
}

bool CascadeRecorder::admitCascade(const std::shared_ptr<const Cascade> &c)
{
    if (!Capturing)
        return false;

    if (mode_ == Batch) {
        // check if we already have N cascades
        if (int(cascade_buffer_.size()) == nCascades_)
            return false;

        // check if it fits in the free space
        int free = kTrackVboVerts;
        for (const auto &r : cascade_buffer_)
            free -= int(r->buff.size());
        if (free < int(c->buff.size()))
            return false;
        // add cascade
        cascade_buffer_.push_back(c);
        // the 1st cascade initializes tMin, tMax
        if (cascade_buffer_.size() == 1) {
            tMin_ = tMax_ = clock_.playbackTime();
        }
        tMax_ += c->duration;
        tracksDirty_ = true;
        update();
        return true;
    } else {
        // check if we already have N+1 cascades
        // the last cascade is the one pending to be shown
        if (int(cascade_buffer_.size()) == nCascades_ + 1)
            return false;

        // check if it fits in the free space
        int free = kTrackVboVerts;
        for (const auto &r : cascade_buffer_)
            free -= int(r->buff.size());
        if (free < int(c->buff.size()))
            return false;

        // add cascade
        cascade_buffer_.push_back(c);
        // the 1st cascade initializes tMin, tMax
        if (cascade_buffer_.size() == 1) {
            tMin_ = tMax_ = clock_.playbackTime();
        }
        if (int(cascade_buffer_.size()) <= nCascades_) {
            tMax_ += c->duration;
            tracksDirty_ = true;
        }
        update();
        return true;
    }
}

void CascadeRecorder::evictOldest()
{
    tMin_ += cascade_buffer_.front()->duration;
    cascade_buffer_.pop_front();
    tMax_ += cascade_buffer_.back()->duration;
    tracksDirty_ = true;
}

void CascadeRecorder::statusUpdate_()
{

    int n = cascade_buffer_.size();
    float ts = clock_.playbackTime();
    double tw = clock_.worldTime();
    QString s = QString("N: %1, tw: %2, tmin: %3, ts: %4, tmax: %5")
                        .arg(n)
                        .arg(tw, 0, 'f', 2)
                        .arg(tMin_, 0, 'f', 2)
                        .arg(ts, 0, 'f', 2)
                        .arg(tMax_, 0, 'f', 2);
    emit statusUpdate(s);
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

    recorder_ = new CascadeRecorder(driver_, this);
    connect(recorder_, &CascadeRecorder::stateChange, this,
            &Track3DViewport::onRecorderStateChange);
    connect(recorder_, &CascadeRecorder::needsUpdate, this, [this]() { update(); });

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
        || !prog_->addShaderFromSourceCode(QOpenGLShader::Fragment, kFragSrc) || !prog_->link())
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
    if (recorder_->dirty()) {
        rebuildTrackBuffer();
        recorder_->clearDirtyFlag();
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
        trackProg_->setUniformValue("uTime", static_cast<GLfloat>(recorder_->playbackTime()));
        trackVao_.bind();
        glMultiDrawArrays(GL_LINE_STRIP, first_.data(), count_.data(), int(first_.size()));
        trackVao_.release();
        trackProg_->release();
    }

    recorder_->update();

    if (recorder_->isRunning())
        update();
}

void Track3DViewport::showEvent(QShowEvent *e)
{
    QOpenGLWidget::showEvent(e);
    recorder_->pause(false);
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
    recorder_->pause(true); // capture pauses when the view is hidden
}

void Track3DViewport::readSceneFromConfig()
{
    if (!driver_)
        return;

    const mcconfig &opt = driver_->options();
    const auto &t = opt.Target;

    boxMin_ = QVector3D(t.origin.x(), t.origin.y(), t.origin.z());
    boxMax_ = boxMin_ + QVector3D(t.size.x(), t.size.y(), t.size.z());
    RegionBox simBox({ boxMin_, boxMax_, Qt::black });

    // periodic axes wrap the ion; a jump > half the box is a wrap
    const QVector3D len = boxMax_ - boxMin_;
    wrapThresh_[0] = t.periodic_bc.x() ? 0.5f * len.x() : 1e30f;
    wrapThresh_[1] = t.periodic_bc.y() ? 0.5f * len.y() : 1e30f;
    wrapThresh_[2] = t.periodic_bc.z() ? 0.5f * len.z() : 1e30f;

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
        RegionBox reg{ x0, x1, c };
        // clip region to the simulation box
        reg = reg.intersection(simBox);
        regions_.push_back(reg);
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
    case Front:
        yaw_ = 0.0f;
        pitch_ = 0.0f;
        break;
    case Back:
        yaw_ = 180.0f;
        pitch_ = 0.0f;
        break;
    case Top:
        yaw_ = 0.0f;
        pitch_ = 89.0f;
        break;
    case Bottom:
        yaw_ = 0.0f;
        pitch_ = -89.0f;
        break;
    case Left:
        yaw_ = -90.0f;
        pitch_ = 0.0f;
        break;
    case Right:
        yaw_ = 90.0f;
        pitch_ = 0.0f;
        break;
    case Iso:
    default:
        yaw_ = 45.0f;
        pitch_ = 30.0f;
        break;
    }
    update();
}

void Track3DViewport::refreshScene()
{
    readSceneFromConfig();
    update();
}

void Track3DViewport::appendTrackRuns_(const Cascade &c, int base, size_t j)
{
    const int s = c.start_pos[j], len = c.length[j];
    int run = 0;
    for (int k = 1; k < len; ++k) {
        const TrackVertex &a = c.buff[s + k - 1], &b = c.buff[s + k];
        // split at recoil-id changes and periodic-boundary wraps
        const bool ridChange = a.rid != b.rid;
        assert(!ridChange);
        const bool wrap = std::abs(a.x - b.x) > wrapThresh_[0]
                || std::abs(a.y - b.y) > wrapThresh_[1] || std::abs(a.z - b.z) > wrapThresh_[2];
        if (ridChange || wrap) {
            if (k - run >= 2) {
                first_.push_back(base + s + run);
                count_.push_back(k - run);
            }
            run = k; // drop the joining segment, start a new strip
        }
    }
    if (len - run >= 2) {
        first_.push_back(base + s + run);
        count_.push_back(len - run);
    }
}

void Track3DViewport::rebuildTrackBuffer()
{
    first_.clear();
    count_.clear();

    int total = 0;
    const auto &cbuff = recorder_->cascade_buffer();
    int N = std::min(recorder_->nCascades(), int(cbuff.size()));
    for (int r = 0; r < N; ++r)
        total += int(cbuff[r]->buff.size());

    std::vector<TrackVertex> all;
    all.reserve(total);
    int base = 0;
    size_t idx = 0;
    float t0 = recorder_->tMin(); // window-relative start of the cascade
    for (int r = 0; r < N; ++r) {
        for (size_t j = 0; j < cbuff[r]->start_pos.size(); ++j)
            appendTrackRuns_(*cbuff[r], base, j);
        // lay this cascade end-to-end on the window-relative time axis
        size_t i = all.size(), n = cbuff[r]->buff.size() + i;
        all.insert(all.end(), cbuff[r]->buff.begin(), cbuff[r]->buff.end());
        for (; i < n; ++i)
            all[i].t += t0;
        t0 += cbuff[r]->duration;
        base += int(cbuff[r]->buff.size());
        ++idx;
    }

    if (!all.empty()) {
        trackVbo_.bind();
        trackVbo_.write(0, all.data(), int(all.size() * sizeof(TrackVertex)));
        trackVbo_.release();
    }
}

void Track3DViewport::onRecorderStateChange(CascadeRecorder::State from, CascadeRecorder::State to)
{
    if (from == CascadeRecorder::Capturing)
        emit captureChanged(false);
    if (to == CascadeRecorder::Capturing)
        emit captureChanged(true);
    update();
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

// return the intersection of two axis-aligned boxes, which may be empty
Track3DViewport::RegionBox Track3DViewport::RegionBox::intersection(const RegionBox &other)
{
    RegionBox b(*this);
    b.x0.setX(std::max(b.x0.x(), other.x0.x()));
    b.x0.setY(std::max(b.x0.y(), other.x0.y()));
    b.x0.setZ(std::max(b.x0.z(), other.x0.z()));
    b.x1.setX(std::min(b.x1.x(), other.x1.x()));
    b.x1.setY(std::min(b.x1.y(), other.x1.y()));
    b.x1.setZ(std::min(b.x1.z(), other.x1.z()));
    return b;
}
