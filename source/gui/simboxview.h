#ifndef SIMBOXVIEW_H
#define SIMBOXVIEW_H

#include <QFrame>
#include <QVector3D>

class QSlider;
class QLabel;
class QButtonGroup;
class QGroupBox;
class QToolButton;
class QwtPlot;

class SimBoxView : public QFrame
{
    Q_OBJECT

protected:
    enum slice_plane_t { xy, yz, zx };

    struct Region3D
    {
        QVector3D x0, x1;
        QRgb color;
        QString name;
        QRectF slice(slice_plane_t p, double v);
    };

public:
    explicit SimBoxView(QWidget *parent = nullptr);

    void setBox(const QVector3D &X0, const QVector3D &X1);
    void addRegion(const QVector3D &X0, const QVector3D &X1, const QRgb &c, const QString &name);
    void clear();

protected:
    QwtPlot *plot;
    QButtonGroup *sliceSelector;
    QSlider *sliceSlider;
    QLabel *sliderLbl;

    Region3D box;
    QVector<Region3D> regions;
    double v;
    QVector<QRectF> viewRects;

    friend class ToolTipper;

    slice_plane_t slice_plane() const;
    void resizeEvent(QResizeEvent *event) override;
    QString labelAt(const QPointF &x);

protected slots:

    void updatePlot();
    void setAxisEqual();
    void setSlicePlane(int);
    void setSlicePos(int);
};

#endif // SimBoxView2_H
