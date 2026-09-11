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
#include <QImage>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QOpenGLFramebufferObject>
#include <QOpenGLShaderProgram>
#include <QPainter>
#include <QPalette>
#include <QShowEvent>
#include <QSurfaceFormat>
#include <QWheelEvent>
#include <QtMath>

// set this to 1 to debug CascadeRecorder
#define CASCADE_RECORDER_DEBUG 0

const char *StateName[] = { "Idle", "Capturing", "Playing", "Paused", "Finishing" };

const char *EventName[] = { "Start", "Stop", "Pause", "Resume", "Update", "Clear", "Play" };

// scene vertex: pos (loc 0) + rgba color (loc 1), interleaved
static const int kSceneFloats = 7;
static const int kStride = kSceneFloats * sizeof(float);

static const int kTrackVboBytes = 120 * 1024 * 1024;
static const int kTrackVboVerts = kTrackVboBytes / int(sizeof(TrackVertex));
static const int kMaxCapMB = 2000; // keep cap bytes < INT_MAX (QOpenGLBuffer::allocate)

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

CascadeRecorder::CascadeRecorder(McDriverObj *driver, QObject *parent)
    : QObject(parent), driver_(driver), capacity_(kTrackVboVerts)
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
                }
                pause(!running);
            },
            Qt::QueuedConnection);
    connect(driver, &McDriverObj::simulationCreated, this, &CascadeRecorder::clear);
    connect(driver, &McDriverObj::configChanged, this, &CascadeRecorder::applyGeometry_);
    connect(driver, &McDriverObj::simulationCreated, this, &CascadeRecorder::applyGeometry_);
    connect(driver, &McDriverObj::simulationCreated, this, [this]() { capture(true); });

    applyGeometry_();
}

void CascadeRecorder::applyGeometry_()
{
    const auto &t = driver_->options().Target;
    const float hx = 0.5f * static_cast<float>(t.size.x());
    const float hy = 0.5f * static_cast<float>(t.size.y());
    const float hz = 0.5f * static_cast<float>(t.size.z());
    channel_->setWrapThresholds(t.periodic_bc.x() ? hx : 1e30f, t.periodic_bc.y() ? hy : 1e30f,
                                t.periodic_bc.z() ? hz : 1e30f);
}

void CascadeRecorder::setPlaybackSpeed(double f)
{
    // f = qBound(0.1, f, 10.0);
    if (f == clock_.speed())
        return;
    clock_.setSpeed(f);
    // if (clock_.playbackTime() > tMax_)
    //     clock_.setPlaybackTime(tMax_);
    emit needsUpdate();
}

void CascadeRecorder::setNCascades(int n)
{
    // n = qBound(1, n, 100);
    if (n == nCascades_)
        return;
    nCascades_ = n;
    stateMachine(Clear);
    emit needsUpdate();
}

int CascadeRecorder::memCap() const
{
    return capacity_ * sizeof(TrackVertex);
}

int CascadeRecorder::memSize() const
{
    return size_ * sizeof(TrackVertex);
}

const char *CascadeRecorder::stateName() const
{
    return StateName[state_];
}

void CascadeRecorder::setMemCap(int bytes)
{
    bytes = qBound(0, bytes, kMaxCapMB * 1024 * 1024);
    int cap = bytes / sizeof(TrackVertex);
    if (cap == capacity_)
        return;
    capacity_ = cap;
    stateMachine(Clear);
    emit needsUpdate();
}

void CascadeRecorder::setEnergyThreshold(double eV)
{
    float th = std::max(0.0, eV);
    if (th == energyThreshold_)
        return;
    energyThreshold_ = th;
    channel_->setEnergyThreshold(th);
    bumpFilterEpoch_();
    stateMachine(Clear);
    emit energyThresholdChanged(th);
}

int CascadeRecorder::capVerts() const
{
    return capacity_;
}

void CascadeRecorder::setGenCutoff(int g)
{
    channel_->setGenCutoff(g);
    bumpFilterEpoch_();
    stateMachine(Clear);
}

void CascadeRecorder::bumpFilterEpoch_()
{
    channel_->setFilterEpoch(++filterEpoch_);
}

void CascadeRecorder::setRingMode(bool on)
{
    mode_ = on ? Ring : Batch;
    stateMachine(Clear);
    emit needsUpdate();
}

void CascadeRecorder::setPlaybackTime(double t)
{
    clock_.setPlaybackTime(t);
    emit needsUpdate();
}

void CascadeRecorder::stateMachine(Event e)
{
    State old_ = state_;
    switch (state_) {
    case Idle:
        switch (e) {
        case Start:
            if (driver_->status() == McDriverObj::mcRunning && enableCap_) {
                channel_->setCapturing(true);
                clock_.resume();
                state_ = Capturing;
            } else {
                channel_->setCapturing(false);
                state_ = Paused;
            }
            break;
        case Play:
            if (!cascade_buffer_.empty()) {
                clock_.resume();
                state_ = Playing;
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
                if (bufferFull_) {
                    channel_->setCapturing(false);
                    state_ = Finishing;
                }
            } else {
                if (bufferFull_ && clock_.playbackTime() > tMax_)
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
    case Playing:
        switch (e) {
        case Stop:
            clock_.pause();
            state_ = Idle;
            break;
        case Update:
            if (playbackTime() > tMax_) {
                setPlaybackTime(tMin_);
            }
            break;
        case Clear:
            clear_();
            clock_.reset();
            clock_.pause();
            state_ = Idle;
            break;
        default:
            break;
        }
        break;
    case Paused:
        switch (e) {
        case Resume:
            if (driver_->status() == McDriverObj::mcRunning && enableCap_) {
                clock_.resume();
                channel_->setCapturing(true);
                state_ = Capturing;
            }
            break;
        case Stop:
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
        case Start:
            if (driver_->status() == McDriverObj::mcRunning && enableCap_) {
                channel_->setCapturing(true);
                state_ = Capturing;
            } else {
                channel_->setCapturing(false);
                state_ = Paused;
            }
            break;
        case Update:
            if (clock_.playbackTime() > tMax_) {
                clock_.pause();
                state_ = Idle;
            }
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
    }
    if (old_ != state_) {
#if (CASCADE_RECORDER_DEBUG)
        qDebug() << "State change: " << StateName[old_] << " -> " << StateName[state_]
                 << " event=" << EventName[e];
#endif
        emit stateChange(old_, state_);
    }
    // if (e == Update)
    statusUpdate_();
    emit dataChanged();

    if (tracksDirty_)
        emit needsUpdate();
}

void CascadeRecorder::clear_()
{
    int nresidents_ = cascade_buffer_.size();
    cascade_buffer_.clear();
    size_ = 0;
    bufferFull_ = false;
    tMin_ = 0.f;
    tMax_ = 0.f;
    tracksDirty_ = nresidents_ > 0;
}

void CascadeRecorder::onCascadeReady()
{
    for (const auto &c : channel_->takeCascades()) {
        if (!c || c->buff.empty())
            continue;
        if (int(c->buff.size()) > capVerts()) {
            if (int(c->buff.size()) > kTrackVboVerts) // genuinely oversized, not just a small cap
                qWarning() << "Track3DViewport: cascade too large to draw:" << c->buff.size();
            continue;
        }
        admitCascade(c);
    }
}

bool CascadeRecorder::admitCascade(const std::shared_ptr<Cascade> &c)
{
    if (state_ != Capturing)
        return false;

    if (c->epoch != filterEpoch_)
        return false;

    // check if cascade fits in the free space
    int free = capacity_ - size_;
    if (free < int(c->buff.size())) {
        if (size_)
            bufferFull_ = true;
        return false;
    }

    if (mode_ == Batch) {

        // check if we already have N cascades
        if (int(cascade_buffer_.size()) == nCascades_) {
            bufferFull_ = true;
            return false;
        }

        // the 1st cascade initializes tMin, tMax
        if (cascade_buffer_.empty()) {
            tMin_ = tMax_ = 0; // clock_.playbackTime();
            clock_.start(); // start the clock on the 1st cascade
        }

        // update cascade timing
        for (auto &v : c->buff)
            v.t += tMax_;

        // add cascade
        cascade_buffer_.push_back(c);
        size_ += int(c->buff.size());

        // update state
        tMax_ += c->duration;
        tracksDirty_ = true;
        update();
        return true;
    } else {

        // check if we already have N+1 cascades
        // the last cascade is the one pending to be shown
        if (int(cascade_buffer_.size()) == nCascades_ + 1) {
            bufferFull_ = true;
            return false;
        }

        // the 1st cascade initializes tMin, tMax
        if (cascade_buffer_.empty()) {
            tMin_ = tMax_ = 0; // clock_.playbackTime();
            clock_.start(); // start the clock on the 1st cascade
        }

        // update cascade timing
        for (auto &v : c->buff)
            v.t += tMax_;

        // add cascade
        cascade_buffer_.push_back(c);
        size_ += int(c->buff.size());

        // update state
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
    size_ -= int(cascade_buffer_.front()->buff.size());
    bufferFull_ = false;
    cascade_buffer_.pop_front();
    tMax_ += cascade_buffer_.back()->duration;
    tracksDirty_ = true;
}

void CascadeRecorder::statusUpdate_()
{
    const double mb = double(size_) * sizeof(TrackVertex) / (1024.0 * 1024.0);
    QString s = QString("Cascades: %1\nMemory: %2 MB\nworld t: %3 s\nplay t: %4 ps\n"
                        "span: %5 - %6 ps")
                        .arg(int(cascade_buffer_.size()))
                        .arg(mb, 0, 'f', 2)
                        .arg(clock_.worldTime(), 0, 'f', 2)
                        .arg(clock_.playbackTime(), 0, 'f', 2)
                        .arg(tMin_, 0, 'f', 2)
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
    connect(recorder_, &CascadeRecorder::energyThresholdChanged, this,
            &Track3DViewport::onEnergyThresholdChanged);

    connect(driver_, &McDriverObj::configChanged, this, &Track3DViewport::refreshScene);
    connect(driver_, &McDriverObj::simulationCreated, this, [this]() {
        this->refreshScene();
        this->homeView();
    });
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
    // glClearColor(0.10f, 0.10f, 0.12f, 1.0f);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
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
    trackVboBytes_ = kTrackVboBytes;
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
    glEnableVertexAttribArray(4);
    glVertexAttribIPointer(4, 1, GL_SHORT, stride,
                           reinterpret_cast<void *>(offsetof(TrackVertex, aid)));
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
    if (sceneDirty_)
        buildSceneBuffers();
    if (recorder_->dirty()) {
        rebuildTrackBuffer();
        recorder_->clearDirtyFlag();
    }

    // QPainter (HUD) can leave the viewport changed; restore it for the widget.
    const qreal dpr = devicePixelRatioF();
    glViewport(0, 0, GLsizei(width() * dpr), GLsizei(height() * dpr));

    drawScene_();

    // fps estimate: exponential moving average of the wall-clock frame interval
    if (frameClock_.isValid()) {
        double dt = frameClock_.nsecsElapsed() / 1.0e9;
        if (dt > 0.0) {
            double inst = 1.0 / dt;
            fps_ = fps_ > 0.0 ? 0.9 * fps_ + 0.1 * inst : inst;
        }
    }
    frameClock_.restart();

    if (hudVisible_)
        drawHud_();

    recorder_->update();

    if (recorder_->isRunning())
        update();
}

void Track3DViewport::drawHud_()
{
    const CascadeRecorder *R = recorder_;
    double t = R->playbackTime();
    double tmin = R->tMin();
    double tmax = R->tMax();
    t = std::min(t, tmax);
    double w = tmax - tmin;
    t -= tmin;
    int ncmax = R->nCascades();
    int nc = R->cascade_buffer().size();
    nc = std::min(nc, ncmax); // to avoid showing n+1/n in ring mode

    static const char *kColorMode[] = { "recoil gen.", "energy", "species" };

    // long long verts = 0;
    // for (int c : count_)
    //     verts += c;

    QStringList rows;
    rows << QStringLiteral("state          %1").arg(QLatin1String(R->stateName()))
         << QStringLiteral("buffer mode    %1")
                    .arg(R->mode() == CascadeRecorder::Batch ? "batch" : "ring")
         << QStringLiteral("cascades       %1 / %2").arg(nc).arg(ncmax)
         << QStringLiteral("memory         %1 / %2 MB")
                    .arg(R->memSize() / (1024.0 * 1024.0), 0, 'f', 1)
                    .arg(R->memCap() / (1024 * 1024))
         << QStringLiteral("total duration %1 ps").arg(w, 0, 'f', 3)
         << QStringLiteral("<time/cascade> %1 ps").arg(nc ? w / nc : 0.0, 0, 'f', 3)
         << QStringLiteral("current time   %1 ps").arg(t, 0, 'f', 3)
         << QStringLiteral("color          %1").arg(QLatin1String(kColorMode[colorMode_ % 3]));
    //<< QStringLiteral("tracks     %1").arg(first_.size())
    //<< QStringLiteral("vertices   %1").arg(verts)
    //<< QStringLiteral("camera     yaw %1°  pitch %2°  d %3")
    //           .arg(yaw_, 0, 'f', 0)
    //          .arg(pitch_, 0, 'f', 0)
    //           .arg(dist_, 0, 'f', 0)
    //<< QStringLiteral("fps         %1").arg(fps_, 0, 'f', 0);

    QPainter p(this);
    p.setRenderHint(QPainter::TextAntialiasing);

    QFont f(QStringLiteral("monospace"));
    f.setStyleHint(QFont::Monospace);
    f.setPointSizeF(font().pointSizeF() * 0.95);
    p.setFont(f);
    const QFontMetrics fm(f);

    int textW = 0;
    for (const QString &r : rows)
        textW = qMax(textW, fm.horizontalAdvance(r));

    const int pad = 8;
    const QRect box(10, 10, textW + 2 * pad, rows.size() * fm.lineSpacing() + 2 * pad);

    p.setPen(Qt::NoPen);
    p.setBrush(QColor(0, 0, 0, 130));
    p.drawRoundedRect(box, 4, 4);

    p.setPen(QColor(235, 235, 235));
    int y = box.top() + pad + fm.ascent();
    for (const QString &r : rows) {
        p.drawText(box.left() + pad, y, r);
        y += fm.lineSpacing();
    }
}

void Track3DViewport::setHudVisible(bool on)
{
    if (hudVisible_ == on)
        return;
    hudVisible_ = on;
    emit hudVisibleChanged(on);
    update();
}

void Track3DViewport::drawScene_()
{
    // The HUD overlay paints with QPainter, whose GL paint engine mutates the
    // context (multisample, depth test, blend func, scissor, colour mask, the
    // viewport, bound program/VAO ...) and restores almost none of it. That
    // state leaks into the *next* frame, so re-assert everything the scene
    // needs here every frame instead of trusting initializeGL(). The viewport
    // is caller-specific (grabScreenshot renders to a larger FBO), so that one
    // is reset in paintGL(), not here.
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_STENCIL_TEST);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
        glLineWidth(1.5f);
        trackProg_->bind();
        trackProg_->setUniformValue("uMvp", mvp());
        trackProg_->setUniformValue("uTime", static_cast<GLfloat>(recorder_->playbackTime()));
        trackProg_->setUniformValue("uColorMode", colorMode_);
        trackProg_->setUniformValue("uColorMap", colorMap_);
        trackProg_->setUniformValue("uEnergyMin", energyDataMin_);
        trackProg_->setUniformValue("uEnergyMax", energyDataMax_);
        trackProg_->setUniformValue("uEnergyLog", energyLog_ ? 1 : 0);
        trackVao_.bind();
        glMultiDrawArrays(GL_LINE_STRIP, first_.data(), count_.data(), int(first_.size()));
        trackVao_.release();
        trackProg_->release();
    }
}

QImage Track3DViewport::grabScreenshot(int scale)
{
    makeCurrent();

    if (sceneDirty_)
        buildSceneBuffers();
    if (recorder_->dirty()) {
        rebuildTrackBuffer();
        recorder_->clearDirtyFlag();
    }

    int s = qBound(1, scale, 4);
    const int kMaxSide = 4096;
    while (s > 1 && std::max(width(), height()) * s > kMaxSide)
        --s;

    QOpenGLFramebufferObjectFormat fmt;
    fmt.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    fmt.setSamples(4);
    QOpenGLFramebufferObject fbo(width() * s, height() * s, fmt);
    if (!fbo.isValid()) {
        doneCurrent();
        return grabFramebuffer();
    }

    fbo.bind();
    glViewport(0, 0, fbo.width(), fbo.height());
    drawScene_();
    fbo.release();

    QImage img = fbo.toImage();
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
    glViewport(0, 0, width(), height());
    doneCurrent();
    return img;
}

void Track3DViewport::showEvent(QShowEvent *e)
{
    QOpenGLWidget::showEvent(e);
#if (CASCADE_RECORDER_DEBUG)
    qDebug() << "showEvent";
#endif
    recorder_->setEnableCap(true);
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
#if (CASCADE_RECORDER_DEBUG)
    qDebug() << "hideEvent";
#endif
    recorder_->pause(true); // capture pauses when the view is hidden
    recorder_->setEnableCap(false);
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

    updateEnergyRange_();
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

void Track3DViewport::setCamera(const CameraState &c)
{
    if (std::isfinite(c.yaw))
        yaw_ = c.yaw;
    if (std::isfinite(c.pitch))
        pitch_ = qBound(-89.0f, c.pitch, 89.0f);
    if (std::isfinite(c.dist))
        dist_ = qBound(radius_ * 0.1f, c.dist, radius_ * 30.0f);
    if (std::isfinite(c.center.x()) && std::isfinite(c.center.y()) && std::isfinite(c.center.z()))
        center_ = c.center;
    update();
}

void Track3DViewport::refreshScene()
{
    readSceneFromConfig();
    update();
}

void Track3DViewport::rebuildTrackBuffer()
{
    first_.clear();
    count_.clear();

    const auto &cbuff = recorder_->cascade_buffer();
    int N = std::min(recorder_->nCascades(), int(cbuff.size()));

    trackVbo_.bind();

    const int want = recorder_->memCap() > 0 ? recorder_->memCap() : kTrackVboBytes;
    if (want != trackVboBytes_) {
        trackVbo_.allocate(want);
        trackVboBytes_ = want;
    }

    int vbo_offset = 0;
    int track_offset = 0;
    for (int r = 0; r < N; ++r) {
        const Cascade &c = *cbuff[r];

        for (size_t j = 0; j < c.start_pos.size(); ++j) {
            first_.push_back(track_offset + c.start_pos[j]);
            count_.push_back(c.length[j]);
        }
        track_offset += int(c.buff.size());

        int len = c.buff.size() * sizeof(TrackVertex);
        trackVbo_.write(vbo_offset, c.buff.data(), len);
        vbo_offset += len;
    }
    trackVbo_.release();
}

void Track3DViewport::setColorMode(int m)
{
    if (m == colorMode_)
        return;
    colorMode_ = m;
    emit colorConfigChanged();
    update();
}

void Track3DViewport::setColorMap(int m)
{
    if (m < 0 || m == colorMap_)
        return;
    colorMap_ = m;
    emit colorConfigChanged();
    update();
}

void Track3DViewport::setEnergyLog(bool on)
{
    if (on == energyLog_)
        return;
    energyLog_ = on;
    emit colorConfigChanged();
    update();
}

void Track3DViewport::onEnergyThresholdChanged(float eV)
{
    updateEnergyRange_();
}

void Track3DViewport::setEnergyAuto(bool on)
{
    if (on == energyAuto_)
        return;
    if (!on) {
        energyUserMin_ = energyDataMin_;
        energyUserMax_ = energyDataMax_;
    }
    energyAuto_ = on;
    updateEnergyRange_();
}

void Track3DViewport::setEnergyUserMin(double eV)
{
    energyUserMin_ = static_cast<float>(eV);
    if (!energyAuto_)
        updateEnergyRange_();
}

void Track3DViewport::setEnergyUserMax(double eV)
{
    energyUserMax_ = static_cast<float>(eV);
    if (!energyAuto_)
        updateEnergyRange_();
}

void Track3DViewport::updateEnergyRange_()
{
    if (energyAuto_) {
        const auto &opt = driver_->options();
        energyDataMin_ = std::max(static_cast<float>(opt.Transport.min_energy),
                                  recorder_->energyThreshold());
        energyDataMax_ = static_cast<float>(opt.IonBeam.energy_distribution.center);
    } else {
        energyDataMin_ = energyUserMin_;
        energyDataMax_ = energyUserMax_;
    }
    if (energyDataMax_ <= energyDataMin_)
        energyDataMax_ = energyDataMin_ * 10.f;
    emit colorConfigChanged();
    update();
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

// -------------------- TrackColorBar --------------------

// axis divisions: decades in log, 1-2-5x10^n in linear
static std::vector<double> axisTicks(double lo, double hi, bool log)
{
    std::vector<double> t;
    if (hi <= lo)
        return t;
    if (log) {
        for (int e = int(std::floor(std::log10(lo))); e <= int(std::ceil(std::log10(hi))); ++e)
            t.push_back(std::pow(10.0, e));
    } else {
        const double raw = (hi - lo) / 4.0;
        const double mag = std::pow(10.0, std::floor(std::log10(raw)));
        const double n = raw / mag;
        const double step = (n < 1.5 ? 1.0 : n < 3.0 ? 2.0 : n < 7.0 ? 5.0 : 10.0) * mag;
        for (double v = std::ceil(lo / step) * step; v <= hi + step * 1e-6; v += step)
            t.push_back(v);
    }
    return t;
}

// matplotlib 'rainbow' colormap — exact analytic definition.
// Source: matplotlib _cm.py; License: matplotlib (BSD-compatible)
static QColor mplRainbow(double x)
{
    x = qBound(0.0, x, 1.0);
    const double r = qBound(0.0, std::abs(2.0 * x - 0.5), 1.0);
    const double g = std::sin(M_PI * x);
    const double b = std::cos(M_PI * x / 2.0);
    return QColor::fromRgbF(r, g, b);
}

QColor TrackColorBar::continuousColor(int map, float t)
{
    if (map == 1) {
        // turbo, (c) Google LLC, Apache-2.0 (A. Mikhailov / R. Du)
        double x = std::min(std::max(double(t), 0.0), 1.0);
        const double r = 0.13572138 + x * (4.61539260 + x * (-42.66032258 + x * 132.13108234))
                + x * x * x * x * (-152.94239396 + x * 59.28637943);
        const double g = 0.09140261 + x * (2.19418839 + x * (4.84296658 + x * -14.18503333))
                + x * x * x * x * (4.27729857 + x * 2.82956604);
        const double b = 0.10667330 + x * (12.64194608 + x * (-60.58204836 + x * 110.36276771))
                + x * x * x * x * (-89.90310912 + x * 27.34824973);
        return QColor::fromRgbF(std::min(std::max(r, 0.0), 1.0), std::min(std::max(g, 0.0), 1.0),
                                std::min(std::max(b, 0.0), 1.0));
    }
    // else if (map == 0)
    return mplRainbow(t);
}

// Tableau 10 palette; values from matplotlib's BSD-licensed TABLEAU_COLORS ("tab10")
QColor TrackColorBar::tab10(int i)
{
    static const int rgb[10][3] = { { 31, 119, 180 },  { 255, 127, 14 },  { 44, 160, 44 },
                                    { 214, 39, 40 },   { 148, 103, 189 }, { 140, 86, 75 },
                                    { 227, 119, 194 }, { 127, 127, 127 }, { 188, 189, 34 },
                                    { 23, 190, 207 } };
    const int *c = rgb[((i % 10) + 10) % 10];
    return QColor(c[0], c[1], c[2]);
}

// Set1 palette; values from matplotlib's "Set1" ListedColormap (ColorBrewer,
// Apache-Style/BSD-compatible)
QColor TrackColorBar::set1(int i)
{
    static const int rgb[9][3] = { { 228, 26, 28 },  { 55, 126, 184 },  { 77, 175, 74 },
                                   { 152, 78, 163 }, { 255, 127, 0 },   { 255, 255, 51 },
                                   { 166, 86, 40 },  { 247, 129, 191 }, { 153, 153, 153 } };
    const int *c = rgb[((i % 9) + 9) % 9];
    return QColor(c[0], c[1], c[2]);
}

TrackColorBar::TrackColorBar(Track3DViewport *view, QWidget *parent) : QWidget(parent), view_(view)
{
    connect(view_, &Track3DViewport::colorConfigChanged, this, QOverload<>::of(&QWidget::update));
}

void TrackColorBar::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setPen(palette().color(QPalette::WindowText));
    const int m = 6;

    if (view_->colorMode() == Track3DViewport::Energy) {
        const QRect bar(m, m + 16, 20, height() - 2 * m - 16);
        const int cmap = view_->colorMap();
        QLinearGradient g(bar.topLeft(), bar.bottomLeft());
        for (int k = 0; k <= 16; ++k)
            g.setColorAt(k / 16.0, continuousColor(cmap, 1.f - k / 16.f)); // high E on top
        p.fillRect(bar, g);
        p.drawRect(bar);
        p.drawText(m, m + 12, QStringLiteral("E [eV]"));

        const double lo = std::max(1e-6, double(view_->energyMin()));
        const double hi = std::max(lo * 1.0001, double(view_->energyMax()));
        const bool logm = view_->energyLog();
        const double llo = std::log10(lo), lhi = std::log10(hi);
        const int tx = bar.right() + 4;
        int lastY = bar.bottom() + 100;
        for (double v : axisTicks(lo, hi, logm)) {
            const double f = logm ? (std::log10(v) - llo) / (lhi - llo) : (v - lo) / (hi - lo);
            if (f < -1e-3 || f > 1.0 + 1e-3)
                continue;
            const int y = bar.bottom() - int(f * bar.height());
            p.drawLine(bar.right(), y, bar.right() + 3, y);
            if (lastY - y < 12)
                continue;
            p.drawText(tx, y + 4, QString::number(v, 'g', 3));
            lastY = y;
        }
        return;
    }

    const bool tab = view_->colorMap() == 0;

    if (view_->colorMode() == Track3DViewport::Generation) {
        p.drawText(m, m + 12, QStringLiteral("Recoil Gen."));
        const int n = 5;
        for (int i = 0; i < n; ++i) {
            const int y = m + 20 + i * 22;
            const QRect sw(m, y, 16, 16);
            p.fillRect(sw, tab ? tab10(i) : set1(i));
            p.drawRect(sw);
            QString lbl = i == 0 ? QStringLiteral("source")
                                 : (i == 4 ? QStringLiteral("4+") : QString::number(i));
            p.drawText(sw.right() + 6, y + 13, lbl);
        }
        return;
    }

    if (view_->colorMode() == Track3DViewport::Species) {
        p.drawText(m, m + 12, QStringLiteral("Atom"));
        if (view_->driver()->status() != McDriverObj::mcReset) {
            auto atom_labels = view_->driver()->get_mcdriver()->getSim()->getTarget().atom_labels();
            int n = atom_labels.size();
            for (int i = 0; i < n; ++i) {
                const int y = m + 20 + i * 22;
                const QRect sw(m, y, 16, 16);
                p.fillRect(sw, tab ? tab10(i) : set1(i));
                p.drawRect(sw);
                p.drawText(sw.right() + 6, y + 13, atom_labels[i].c_str());
            }
        } else {
            int n = 5;
            for (int i = 0; i < n; ++i) {
                const int y = m + 20 + i * 22;
                const QRect sw(m, y, 16, 16);
                p.fillRect(sw, tab ? tab10(i) : set1(i));
                p.drawRect(sw);
                QString lbl = i == 0 ? QStringLiteral("source")
                                     : (i == 4 ? QStringLiteral("4+") : QString::number(i));
                p.drawText(sw.right() + 6, y + 13, lbl);
            }
        }
        return;
    }
}
