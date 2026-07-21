#ifndef TRACK3DVIEWPORT_H
#define TRACK3DVIEWPORT_H

#include <vector>

#include <QColor>
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

// 3D viewport (QOpenGLWidget): target box + material regions + orbit camera.
// Owns the TrackDataChannel and installs the core event handler.
class Track3DViewport : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
{
    Q_OBJECT
public:
    // preset camera angles
    enum View { Front, Back, Top, Bottom, Left, Right, Iso };

    explicit Track3DViewport(McDriverObj *driver, QWidget *parent = nullptr);
    ~Track3DViewport() override;

public slots:
    void homeView(); // default angle, fit the box
    void setPresetView(int v);
    void refreshScene(); // rebuild from the config

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    void showEvent(QShowEvent *e) override;

    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void wheelEvent(QWheelEvent *e) override;

private slots:
    void onCascadeReady(); // GUI thread; drains finished cascades

private:
    struct RegionBox
    {
        QVector3D x0, x1;
        QColor color;
    };

    void readSceneFromConfig(); // fill boxMin_/boxMax_/regions_
    void buildSceneBuffers(); // fill the box + region VBOs
    QVector3D eye() const;
    QMatrix4x4 mvp() const;
    void fitView();

    McDriverObj *driver_; // not owned
    TrackDataChannel *channel_; // owned (QObject child)

    // scene
    QVector3D boxMin_, boxMax_;
    std::vector<RegionBox> regions_;
    bool sceneDirty_{ true };

    // GL
    QOpenGLShaderProgram *prog_{ nullptr };
    QOpenGLVertexArrayObject boxVao_, regVao_;
    QOpenGLBuffer boxVbo_{ QOpenGLBuffer::VertexBuffer };
    QOpenGLBuffer regVbo_{ QOpenGLBuffer::VertexBuffer };
    int boxVertices_{ 0 };
    int regVertices_{ 0 };

    // orbit camera
    QVector3D center_; // look-at point
    float radius_{ 100.f }; // scene radius
    float dist_{ 300.f }; // eye distance
    float yaw_{ 45.f }, pitch_{ 30.f }; // degrees, +pitch = eye above
    QPoint lastPos_;
    bool viewInitialized_{ false };
};

#endif // TRACK3DVIEWPORT_H
