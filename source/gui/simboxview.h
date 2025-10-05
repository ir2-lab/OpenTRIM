#ifndef SIMBOXVIEW_H
#define SIMBOXVIEW_H

#include <QVector3D>
#include <QFrame>

namespace QtDataVisualization {
class Q3DScatter;
class QCustom3DVolume;
class QCustom3DItem;
} // namespace QtDataVisualization

class QSlider;
class QLabel;
class QButtonGroup;
class QGroupBox;
class QToolButton;

class SimBoxView : public QFrame
{
    Q_OBJECT

    // constexpr static const int txtrExp_ = 7;
    constexpr static const int txtrExp_ = 8;

public:
    SimBoxView(QWidget *parent = nullptr);
    ~SimBoxView();

    constexpr static const int textureWidth = 1 << txtrExp_;
    constexpr static const int textureSize = 1 << (txtrExp_ * 3);

    static bool isSupported();
    QVector<QRgb> colorTable() const;
    void setScale(float Lx, float Ly, float Lz) { setScale(QVector3D(Lx, Ly, Lz)); }
    void setScale(const QVector3D &L);
    void setBackgroundColor(QColor clr);
    void fill(const QVector3D &lowerLeft, const QVector3D &upperRight, const QColor &clr,
              bool drawBorder = true);
    void clearVolume();

protected:
    QWidget *container;
    QtDataVisualization::Q3DScatter *graph;
    QtDataVisualization::QCustom3DVolume *volume;
    QVector<QtDataVisualization::QCustom3DItem *> simbox;
    float scaleFactor;
    float beamWidth;

    // slicing
    QToolButton *sliceEnable;
    QButtonGroup *sliceSelector;
    QSlider *sliceSlider;

    int create3DScene();
    int createCtrls();
    void updateSlice();

    int idx(int x, int y, int z)
    {
        // return x + textureWidth * (y + textureWidth * z);
        return x + textureWidth * ((textureWidth - 1 - z) + textureWidth * y);
    }

protected slots:
    void on_sliceEnable_toggled(bool);
    void on_sliceSelector_idClicked(int);
    void on_sliceSlider_sliderMoved(int);
};
#endif // SIMBOXVISUALIZER_H
