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

// 3D viewport (QOpenGLWidget): target box + material regions + orbit camera,
// and the ion cascade tracks drawn as line strips with time-evolution playback.
// Owns the TrackDataChannel and installs the core event handler.
class Track3DViewport : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
{
    Q_OBJECT
    Q_PROPERTY(int nCascades READ nCascades WRITE setNCascades)
public:
    enum View { Front, Back, Top, Bottom, Left, Right, Iso };
    enum Mode { Batch, Ring };

    explicit Track3DViewport(McDriverObj *driver, QWidget *parent = nullptr);
    ~Track3DViewport() override;

    int nCascades() const { return nCascades_; }

public slots:
    void homeView();
    void setPresetView(int v);
    void refreshScene();

    void setCapture(bool on);
    void clear();
    void setNCascades(int n);
    void setRingMode(bool on);
    void setSpeed(double f); // f [ns/s]

signals:
    void statusUpdate(const QString &s);
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
    void onCascadeReady();

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

    void admitCascade(const std::shared_ptr<const Cascade> &c);
    void evictOldest_();
    void drainPending_();
    void recomputeTiming_(); // no GL
    void rebuildTrackBuffer();
    void appendTrackRuns_(const Cascade &c, int base, size_t j);
    void updateCapture();
    bool ringBehind() const;
    void advancePlayback_();
    void resetTracks_();
    void resumeClock_();
    void pauseClock_();
    double worldTime() const; // [s]
    float playbackTime() const; // [ns]
    void statusUpdate_();

    McDriverObj *driver_; // not owned
    TrackDataChannel *channel_;

    QVector3D boxMin_, boxMax_;
    std::vector<RegionBox> regions_;
    float wrapThresh_[3]{ 1e30f, 1e30f, 1e30f }; // per-axis, periodic only
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
    std::deque<std::shared_ptr<const Cascade>> residents_;
    std::deque<float> resDur_; // [ns]
    std::deque<std::shared_ptr<const Cascade>> pending_; // ring staging
    std::vector<int> first_, count_; // glMultiDrawArrays args
    std::vector<double> ends_; // cumulative end per resident [ns]
    double tWindowStart_{ 0.0 }; // [ns]
    bool tracksDirty_{ false };

    Mode mode_{ Ring };
    int nCascades_{ 5 };
    bool viewActive_{ false };
    bool captureOn_{ false };
    bool runEnded_{ false };
    double speed_{ 1.0 }; // [ns/s]
    QElapsedTimer clock_;
    bool clockRunning_{ false };
    double twOffset_{ 0.0 }; // [s]
    float tMax_{ 0.f }; // [ns]

    QVector3D center_;
    float radius_{ 100.f };
    float dist_{ 300.f };
    float yaw_{ 45.f }, pitch_{ 30.f }; // degrees, +pitch = eye above
    QPoint lastPos_;
    bool viewInitialized_{ false };
};

#endif // TRACK3DVIEWPORT_H
