#ifndef TRACK3DVIEWPORT_H
#define TRACK3DVIEWPORT_H

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
#include <QVector3D>

class McDriverObj;
class TrackDataChannel;
class QOpenGLShaderProgram;
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
    double tsOffset_{ 0.0 }; // [ns] simulation/playback time offset
    double speed_{ 1.0 }; // [ns/s] playback speed
};

// Recorder object with internal state machine functionality
// Owns the TrackDataChannel and installs the core event handler.
// Gets cascades from the TrackDataChannel
// Stores and updates the buffer of displayed cascades
class CascadeRecorder : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int nCascades READ nCascades WRITE setNCascades)
    Q_PROPERTY(double playbackSpeed READ playbackSpeed WRITE setPlaybackSpeed)

public:
    enum State { Idle, Capturing, Paused, Finishing };
    enum Mode { Batch, Ring };

    typedef std::deque<std::shared_ptr<const Cascade>> cascade_buffer_t;

    explicit CascadeRecorder(McDriverObj *driver, QObject *parent);

    int nCascades() const { return nCascades_; }
    double playbackSpeed() const { return clock_.speed(); }
    State state() const { return state_; }
    bool dirty() const { return tracksDirty_; }
    void clearDirtyFlag() { tracksDirty_ = false; }
    TrackDataChannel *channel() { return channel_; }
    const cascade_buffer_t &cascade_buffer() const { return cascade_buffer_; }
    double playbackTime() const { return clock_.playbackTime(); }
    bool isRunning() const { return clock_.isRunning(); }
    double tMin() const { return tMin_; }
    double tMax() const { return tMax_; }

public slots:
    void capture(bool on) { stateMachine(on ? Start : Stop); }
    void pause(bool on) { stateMachine(on ? Pause : Resume); }
    void clear() { stateMachine(Clear); }
    void setNCascades(int n);
    void setRingMode(bool on);
    void setPlaybackSpeed(double f); // f [ns/s]
    void update() { stateMachine(Update); }

signals:
    void stateChange(State from, State to);
    void statusUpdate(const QString &s);
    void needsUpdate();

private slots:
    void onCascadeReady();
    void applyGeometry_();

private:
    McDriverObj *driver_; // not owned
    TrackDataChannel *channel_;
    enum Event { Start, Stop, Pause, Resume, Update, Clear };
    State state_{ Idle };
    std::deque<std::shared_ptr<const Cascade>> cascade_buffer_;
    bool tracksDirty_{ false };
    Mode mode_{ Ring };
    int nCascades_{ 5 };
    CascadeRecorderClock clock_;
    double tMin_{ 0.f }; // [ns] start of 1st displayed cascade
    double tMax_{ 0.f }; // [ns] end of last displayed cascade

    void stateMachine(Event e);
    bool admitCascade(const std::shared_ptr<const Cascade> &c);
    void evictOldest();
    void statusUpdate_();
    void clear_();
};

// 3D viewport (QOpenGLWidget): target box + material regions + orbit camera,
// and the ion cascade tracks drawn as line strips with time-evolution playback.
// Owns a CascadeRecorder object
class Track3DViewport : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
{
    Q_OBJECT

public:
    enum View { Front, Back, Top, Bottom, Left, Right, Iso };

    explicit Track3DViewport(McDriverObj *driver, QWidget *parent = nullptr);
    ~Track3DViewport() override;

    CascadeRecorder *cascadeRecorder() { return recorder_; }

public slots:
    void homeView();
    void setPresetView(int v);
    void refreshScene();

signals:   
    void captureChanged(bool on);

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
    void buildSceneBuffers();
    QVector3D eye() const;
    QMatrix4x4 mvp() const;
    void fitView();

    void rebuildTrackBuffer();

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
    std::vector<int> first_, count_; // glMultiDrawArrays args

    QVector3D center_;
    float radius_{ 100.f };
    float dist_{ 300.f };
    float yaw_{ 45.f }, pitch_{ 30.f }; // degrees, +pitch = eye above
    QPoint lastPos_;
    bool viewInitialized_{ false };
};

#endif // TRACK3DVIEWPORT_H
