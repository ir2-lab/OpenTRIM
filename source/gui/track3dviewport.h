#ifndef TRACK3DVIEWPORT_H
#define TRACK3DVIEWPORT_H

#include <cstdint>
#include <deque>
#include <memory>
#include <vector>

#include <QColor>
#include <QElapsedTimer>
#include <QMatrix4x4>
#include <QOpenGLBuffer>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLWidget>
#include <QPoint>
#include <QSize>
#include <QVector3D>
#include <QWidget>

class McDriverObj;
class TrackDataChannel;
class QOpenGLShaderProgram;
class QImage;
struct Cascade;

// specialized clock for world- & simulation(playback)- time
// both times are monotonous, always advancing
class CascadeRecorderClock
{
public:
    bool isRunning() const { return clockRunning_; }
    double speed() const { return speed_; }
    void setSpeed(double s)
    {
        if (clockRunning_) {
            pause();
            speed_ = s;
            resume();
        } else
            speed_ = s;
    }
    double worldTime() const
    {
        return twOffset_ + (clockRunning_ ? clock_.elapsed() / 1000.0 : 0.0);
    }
    double playbackTime() const
    {
        return tsOffset_ + (clockRunning_ ? clock_.elapsed() / 1000.0 * speed_ : 0.0);
    }
    void setPlaybackTime(double t0)
    {
        twOffset_ = 0;
        tsOffset_ = t0;
        if (clockRunning_)
            clock_.restart();
    }
    void start()
    {
        twOffset_ = tsOffset_ = 0;
        clock_.start();
        clockRunning_ = true;
    }
    void reset()
    {
        twOffset_ = tsOffset_ = 0;
        if (clockRunning_)
            clock_.restart();
    }

    void pause()
    {
        if (!clockRunning_)
            return;
        double t = clock_.elapsed() / 1000.0;
        twOffset_ += t;
        tsOffset_ += t * speed_;
        clockRunning_ = false;
    }
    void resume()
    {
        if (clockRunning_)
            return;
        clock_.restart();
        clockRunning_ = true;
    }

private:
    QElapsedTimer clock_;
    bool clockRunning_{ false };
    double twOffset_{ 0.0 }; // [s] world time offset
    double tsOffset_{ 0.0 }; // [ps]] simulation/playback time offset
    double speed_{ 1.0 }; // [ps/s] playback speed
};

// Recorder object with internal state machine functionality
// Owns the TrackDataChannel and installs the core event handler.
// Gets cascades from the TrackDataChannel
// Stores and updates the buffer of displayed cascades
class CascadeRecorder : public QObject
{
    Q_OBJECT
    // # of cascades to buffer
    Q_PROPERTY(int nCascades READ nCascades WRITE setNCascades)
    // buffer memory capacity in bytes
    Q_PROPERTY(int memCap READ memCap WRITE setMemCap)
    // current data memory size in bytes
    Q_PROPERTY(int memSize READ memSize)
    Q_PROPERTY(double playbackTime READ playbackTime WRITE setPlaybackTime)
    Q_PROPERTY(double playbackSpeed READ playbackSpeed WRITE setPlaybackSpeed)
    // energy threshold for track capture
    // ion tracks below this energy are dropped
    Q_PROPERTY(float energyThreshold READ energyThreshold WRITE setEnergyThreshold NOTIFY
                       energyThresholdChanged)
    // capture enable flag - initially false
    Q_PROPERTY(bool enableCap READ enableCap WRITE setEnableCap)

public:
    /* State Machine
     * Idle
     * Capturing
     * Paused: paused while capturing
     * Finishing: playing back the captured cascades until the end
     * Pausing: going from Capturing to Paused mode
     */
    enum State { Idle, Capturing, Playing, Paused, Finishing };

    /* Buffer Mode
     * - Batch: get N cascades and stop
     * - Ring: populate a ring buffer of N cascades
     */
    enum Mode { Batch, Ring };

    typedef std::deque<std::shared_ptr<const Cascade>> cascade_buffer_t;

    explicit CascadeRecorder(McDriverObj *driver, QObject *parent);

    int nCascades() const { return nCascades_; }
    int memCap() const;
    int memSize() const;
    Mode mode() const { return mode_; }
    double playbackTime() const { return clock_.playbackTime(); }
    double playbackSpeed() const { return clock_.speed(); }
    float energyThreshold() const { return energyThreshold_; }
    bool enableCap() const { return enableCap_; }
    State state() const { return state_; }
    const char *stateName() const;
    // tracks need redraw
    bool dirty() const { return tracksDirty_; }
    void clearDirtyFlag() { tracksDirty_ = false; }
    // channel to get ion track data
    TrackDataChannel *channel() { return channel_; }
    const cascade_buffer_t &cascade_buffer() const { return cascade_buffer_; }
    bool isRunning() const { return clock_.isRunning(); }
    double tMin() const { return tMin_; }
    double tMax() const { return tMax_; }

public slots:
    void capture(bool on) { stateMachine(on ? Start : Stop); }
    void pause(bool on) { stateMachine(on ? Pause : Resume); }
    void play(bool on) { stateMachine(on ? Play : Stop); }
    void clear() { stateMachine(Clear); }
    void setNCascades(int n);
    void setRingMode(bool on);
    void setPlaybackTime(double t);
    void setPlaybackSpeed(double f); // f [ps/s]
    void setMemCap(int bytes);
    void setEnergyThreshold(double eV);
    void setEnableCap(bool on) { enableCap_ = on; }
    void setGenCutoff(int g);
    void update() { stateMachine(Update); }

signals:
    void stateChange(State from, State to);
    void statusUpdate(const QString &s);
    void dataChanged();
    void needsUpdate();
    void energyThresholdChanged(float v);

private slots:
    void onCascadeReady();
    void applyGeometry_();

private:
    McDriverObj *driver_; // not owned
    TrackDataChannel *channel_;
    enum Event { Start, Stop, Pause, Resume, Update, Clear, Play };
    State state_{ Idle };
    std::deque<std::shared_ptr<const Cascade>> cascade_buffer_;
    bool tracksDirty_{ false };
    Mode mode_{ Ring };
    int nCascades_{ 10 };
    float energyThreshold_{ 0.f }; // [eV]
    bool enableCap_{ false };
    CascadeRecorderClock clock_;
    double tMin_{ 0.f }; // [ps] start of 1st displayed cascade
    double tMax_{ 0.f }; // [ps] end of last displayed cascade
    int capacity_; // in # of vertices
    int size_{ 0 }; // current total # of vertices
    bool bufferFull_{ false };
    uint32_t filterEpoch_{ 0 };

    void stateMachine(Event e);
    bool admitCascade(const std::shared_ptr<Cascade> &c);
    int capVerts() const;
    void evictOldest();
    void statusUpdate_();
    void clear_();
    void bumpFilterEpoch_();
};

// 3D viewport (QOpenGLWidget): target box + material regions + orbit camera,
// and the ion cascade tracks drawn as line strips with time-evolution playback.
// Owns a CascadeRecorder object
class Track3DViewport : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
{
    Q_OBJECT

public:
    enum View { Front, Back, Top, Bottom, Left, Right, Iso };
    enum ColorMode { Generation, Energy, Species };

    struct CameraState
    {
        float yaw{ 45.f }, pitch{ 30.f };
        float dist{ 300.f };
        QVector3D center;
    };

    explicit Track3DViewport(McDriverObj *driver, QWidget *parent = nullptr);
    ~Track3DViewport() override;

    CascadeRecorder *cascadeRecorder() { return recorder_; }
    McDriverObj *driver() { return driver_; }

    CameraState camera() const { return { yaw_, pitch_, dist_, center_ }; }
    void setCamera(const CameraState &c);

    QImage grabScreenshot(int scale = 2);

    int colorMode() const { return colorMode_; }
    int colorMap() const { return colorMap_; }
    bool hudVisible() const { return hudVisible_; }
    bool energyLog() const { return energyLog_; }
    bool energyAuto() const { return energyAuto_; }
    float energyMin() const { return energyDataMin_; } // [eV]
    float energyMax() const { return energyDataMax_; }

public slots:
    void homeView();
    void setPresetView(int v);
    void refreshScene();
    void setColorMode(int m);
    void setColorMap(int m);
    void setHudVisible(bool on);
    void setEnergyLog(bool on);
    void onEnergyThresholdChanged(float eV);
    void setEnergyAuto(bool on);
    void setEnergyUserMin(double eV);
    void setEnergyUserMax(double eV);

signals:
    void captureChanged(bool on);
    void colorConfigChanged();
    void hudVisibleChanged(bool on);

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void showEvent(QShowEvent *e) override;
    void hideEvent(QHideEvent *e) override;

    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;

private slots:
    void onRecorderStateChange(CascadeRecorder::State from, CascadeRecorder::State to);

private:
    struct RegionBox
    {
        QVector3D x0, x1;
        QColor color;
        RegionBox intersection(const RegionBox &other);
    };

    void readSceneFromConfig();
    void updateEnergyRange_();
    void buildSceneBuffers();
    QVector3D eye() const;
    QMatrix4x4 mvp() const;
    void fitView();

    void rebuildTrackBuffer();
    void drawScene_();
    void drawHud_();

    McDriverObj *driver_; // not owned
    CascadeRecorder *recorder_;

    QVector3D boxMin_, boxMax_;
    std::vector<RegionBox> regions_;
    bool sceneDirty_{ true };

    QOpenGLShaderProgram *prog_{ nullptr };
    QOpenGLVertexArrayObject boxVao_, regVao_;
    QOpenGLBuffer boxVbo_{ QOpenGLBuffer::VertexBuffer };
    QOpenGLBuffer regVbo_{ QOpenGLBuffer::VertexBuffer };
    int boxVertices_{ 0 };
    int regVertices_{ 0 };

    QOpenGLShaderProgram *trackProg_{ nullptr };
    QOpenGLVertexArrayObject trackVao_;
    QOpenGLBuffer trackVbo_{ QOpenGLBuffer::VertexBuffer };
    int trackVboBytes_{ 0 };
    std::vector<int> first_, count_; // glMultiDrawArrays args

    int colorMode_{ Generation };
    int colorMap_{ 0 };
    bool energyLog_{ true };
    bool energyAuto_{ true };
    float energyDataMin_{ 1.f }, energyDataMax_{ 1.e6f }; // [eV]
    float energyUserMin_{ 1.f }, energyUserMax_{ 1.e6f }; // [eV]

    QVector3D center_;
    float radius_{ 100.f };
    float dist_{ 300.f };
    float yaw_{ 45.f }, pitch_{ 30.f }; // degrees, +pitch = eye above
    QPoint lastPos_;
    bool viewInitialized_{ false };

    bool hudVisible_{ true };
    QElapsedTimer frameClock_; // wall-clock between paintGL calls, for the HUD fps
    double fps_{ 0.0 };
};

// Legend next to the viewport: an energy gradient in Energy mode, else discrete
// generation/species swatches. Repaints on the viewport's colorConfigChanged.
class TrackColorBar : public QWidget
{
    Q_OBJECT

public:
    explicit TrackColorBar(Track3DViewport *view, QWidget *parent = nullptr);
    QSize sizeHint() const override { return QSize(84, 200); }

    static QColor continuousColor(int map, float t); // mirrors track.frag continuousColor()
    static QColor tab10(int i); // mirrors track.frag tab10()
    static QColor set1(int i); // mirrors track.frag set1()

protected:
    void paintEvent(QPaintEvent *e) override;

private:
    Track3DViewport *view_; // not owned
};

#endif // TRACK3DVIEWPORT_H
