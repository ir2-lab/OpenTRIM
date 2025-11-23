#include "simboxview.h"

#include <cmath>

#include <QFontMetrics>
#include <QVector3D>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>

#include <qpainterpath.h>
#include <qwt_legend.h>
#include <qwt_plot.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_magnifier.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_picker.h>
#include <qwt_plot_shapeitem.h>
#include <qwt_point_data.h>
#include <qwt_scale_engine.h>
#include <qwt_scale_widget.h>

class ToolTipper : public QwtPlotPicker
{
public:
    ToolTipper(SimBoxView *s,
               int xAxis,
               int yAxis,
               RubberBand rubberBand,
               DisplayMode trackerMode,
               QWidget *pc)
        : QwtPlotPicker(xAxis, yAxis, rubberBand, trackerMode, pc)
        , S(s)
    {}

protected:
    SimBoxView *S;
    QwtText trackerTextF(const QPointF &pos) const override
    {
        //Since the "paintAttributes", [text+background colour] act on QwtTexts
        //break up the creation of trackerTextF: one function to create the text
        //(as a QString), and another to set attributes and return the object.
        QString s = S->labelAt(pos); //createLabelText(pos);
        QwtText trackerText;
        trackerText.setBackgroundBrush(Qt::lightGray);
        trackerText.setText(s);
        trackerText.setColor(Qt::black);
        return trackerText;
    }
    QString createLabelText(const QPointF &pos) const
    {
        QwtText lx = plot()->axisScaleDraw(QwtPlot::xBottom)->label(pos.x());
        QwtText ly = plot()->axisScaleDraw(QwtPlot::yLeft)->label(pos.y());
        QString S(lx.text());
        S += QChar(',');
        S += ly.text();
        return S;
    }
};

QRectF SimBoxView::Region3D::slice(slice_plane_t p, double v)
{
    QRectF r;
    switch (p) {
    case xy:
        if (v >= x0.z() && v <= x1.z()) {
            r.setRect(x0.x(), x0.y(), x1.x() - x0.x(), x1.y() - x0.y());
        }
        break;
    case yz:
        if (v >= x0.x() && v <= x1.x()) {
            r.setRect(x0.y(), x0.z(), x1.y() - x0.y(), x1.z() - x0.z());
        }
        break;
    case zx:
        if (v >= x0.y() && v <= x1.y()) {
            r.setRect(x0.z(), x0.x(), x1.z() - x0.z(), x1.x() - x0.x());
        }
        break;
    }
    return r;
}

class RectItem : public QwtPlotShapeItem
{
public:
    RectItem(const QRectF &r, const QPen &p, const QBrush &b)
        : QwtPlotShapeItem("Rect")
    {
        setRenderHint(QwtPlotItem::RenderAntialiased, true);
        QPainterPath path;
        path.addRect(r);
        setShape(path);
        setPen(p);
        setBrush(b);
    }
};

SimBoxView::SimBoxView(QWidget *parent)
    : QFrame{parent}
{
    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setSpacing(0);
    setLayout(vLayout);

    // create plot widget
    {
        plot = new QwtPlot;
        QwtPlotCanvas *cnv = new QwtPlotCanvas();
        //cnv->setFrameStyle( QFrame::Box | QFrame::Plain );
        //cnv->setLineWidth( 1 );
        cnv->setStyleSheet("border: 1px solid black; background: white");
        plot->setCanvas(cnv);

        for (int i = 0; i < QwtPlot::axisCnt; i++) {
            QwtScaleWidget *scaleWidget = plot->axisWidget(i);
            if (scaleWidget)
                scaleWidget->setMargin(0);

            QwtScaleDraw *scaleDraw = plot->axisScaleDraw(i);
            if (scaleDraw)
                scaleDraw->enableComponent(QwtAbstractScaleDraw::Backbone, false);
        }

        plot->plotLayout()->setAlignCanvasToScales(true);
        plot->setAutoReplot(true);

        QFont font;
        font = plot->axisFont(QwtPlot::xBottom);
        font.setPointSize(8);
        plot->setAxisFont(QwtPlot::xBottom, font);
        plot->setAxisFont(QwtPlot::yLeft, font);

        QwtText txt = plot->title();
        font = txt.font();
        font.setPointSize(10);
        txt.setFont(font);
        plot->setTitle(txt);

        txt = plot->axisTitle(QwtPlot::xBottom);
        font = txt.font();
        font.setPointSize(9);
        txt.setFont(font);
        plot->setAxisTitle(QwtPlot::xBottom, txt);
        plot->setAxisTitle(QwtPlot::yLeft, txt);

        QWidget *container = new QWidget;
        QHBoxLayout *hbox = new QHBoxLayout;
        QFontMetrics fm(this->font());
        int lineSpc = fm.lineSpacing();
        QMargins m = hbox->contentsMargins();
        m.setRight(m.right() + lineSpc + 2);
        m += 10;
        hbox->setContentsMargins(m);
        container->setLayout(hbox);
        hbox->addWidget(plot);

        vLayout->addWidget(container, 1);
    }

    // create slice ctrls
    {
        QWidget *slicerGroup = new QWidget;

        QHBoxLayout *hbox1 = new QHBoxLayout;
        slicerGroup->setLayout(hbox1);

        sliceSelector = new QButtonGroup(this);
        sliceSelector->setExclusive(true);
        QToolButton *bt[3];
        QHBoxLayout *hbox = new QHBoxLayout;
        hbox->setContentsMargins(0, 0, 0, 0);
        hbox->setSpacing(0);
        const char *btLbl[] = {"XY", "YZ", "ZX"};
        for (int i = 0; i < 3; i++)
        {
            bt[i] = new QToolButton;
            bt[i]->setCheckable(true);
            bt[i]->setText(btLbl[i]);
            sliceSelector->addButton(bt[i], i);
            hbox->addWidget(bt[i]);
        }
        bt[0]->setChecked(true);
        hbox1->addLayout(hbox);

        QFrame *h2 = new QFrame;
        h2->setFrameStyle(QFrame::Raised | QFrame::VLine);
        hbox1->addWidget(h2);

        sliderLbl = new QLabel("Z");
        hbox1->addWidget(sliderLbl);

        sliceSlider = new QSlider(Qt::Horizontal, this);
        sliceSlider->setMinimum(0);
        sliceSlider->setMaximum(100);
        sliceSlider->setPageStep(10);
        sliceSlider->setMinimumWidth(80);
        hbox1->addWidget(sliceSlider);

        vLayout->addWidget(slicerGroup);
    }

    connect(sliceSlider, &QSlider::valueChanged, this, &SimBoxView::setSlicePos);
    connect(sliceSelector, &QButtonGroup::idClicked, this, &SimBoxView::setSlicePlane);

    ToolTipper *tt = new ToolTipper(this,
                                    QwtPlot::xBottom,
                                    QwtPlot::yLeft,
                                    //        QwtPicker::PointSelection | QwtPicker::DragSelection,
                                    QwtPlotPicker::CrossRubberBand,
                                    QwtPicker::AlwaysOn,
                                    plot->canvas());
    tt->setRubberBand(QwtPicker::CrossRubberBand);

    QwtPlotMagnifier *magnifier = new QwtPlotMagnifier(plot->canvas());
    magnifier->setAxisEnabled(QwtPlot::yRight, false);

    QwtPlotPanner *panner = new QwtPlotPanner(plot->canvas());
    panner->setAxisEnabled(QwtPlot::yRight, false);

    clear();
}

void SimBoxView::setBox(const QVector3D &X0, const QVector3D &X1)
{
    box = {X0, X1};
    v = box.x0.z() + (box.x1.z() - box.x0.z()) * 0.5;
    sliceSlider->setValue(sliceSlider->maximum() / 2);
    setSlicePlane(0);
}

void SimBoxView::addRegion(const QVector3D &X0,
                           const QVector3D &X1,
                           const QRgb &c,
                           const QString &name)
{
    regions.push_back({X0, X1, c, name});
    updatePlot();
}

void SimBoxView::clear()
{
    regions.clear();
    setBox({0., 0., 0.}, {100., 100., 100.});
}

SimBoxView::slice_plane_t SimBoxView::slice_plane() const
{
    return static_cast<slice_plane_t>(sliceSelector->checkedId());
}

void SimBoxView::resizeEvent(QResizeEvent *event)
{
    updatePlot();
    QWidget::resizeEvent(event);
}

QString SimBoxView::labelAt(const QPointF &x)
{
    QString s;
    for (int i = 0; i < viewRects.size(); ++i) {
        if (viewRects[i].contains(x))
            s = regions[i].name;
    }
    return s;
}

void SimBoxView::updatePlot()
{
    plot->detachItems();
    plot->setAxisAutoScale(QwtPlot::yLeft);
    plot->setAxisAutoScale(QwtPlot::xBottom);

    slice_plane_t sp = slice_plane();

    const char *axisLabels[] = {"X [nm]", "Y [nm]", "Z [nm]"};
    int ix = (int) sp;
    int iy = (ix + 1) % 3;
    plot->setAxisTitle(QwtPlot::xBottom, axisLabels[ix]);
    plot->setAxisTitle(QwtPlot::yLeft, axisLabels[iy]);

    RectItem *I;
    QRectF boxrect = box.slice(sp, v);

    viewRects.clear();
    for (int i = 0; i < regions.size(); ++i) {
        QRectF r = regions[i].slice(sp, v);
        r = r.intersected(boxrect);
        viewRects.push_back(r);
        const QRgb &c = regions[i].color;
        I = new RectItem(r, QPen(Qt::NoPen), QBrush(c));
        I->attach(plot);
    }

    I = new RectItem(boxrect, QPen(Qt::black), QBrush());
    I->attach(plot);

    setAxisEqual();
    plot->setTitle(QString("%1 = %2 [nm]").arg(sliderLbl->text()).arg(v));
}

void SimBoxView::setAxisEqual()
{
    QSize sz = plot->canvas()->size();
    double x0 = plot->axisScaleDiv(QwtPlot::xBottom).lowerBound();
    double x1 = plot->axisScaleDiv(QwtPlot::xBottom).upperBound();
    double y0 = plot->axisScaleDiv(QwtPlot::yLeft).lowerBound();
    double y1 = plot->axisScaleDiv(QwtPlot::yLeft).upperBound();
    double ax = (x1 - x0) / sz.width();
    double ay = (y1 - y0) / sz.height();
    if (ay < ax) {
        double ym = 0.5 * (y0 + y1);
        double dy = ax * sz.height();
        plot->setAxisScale(QwtPlot::yLeft, ym - 0.5 * dy, ym + 0.5 * dy);
    } else if (ax < ay) {
        double xm = 0.5 * (x0 + x1);
        double dx = ay * sz.width();
        plot->setAxisScale(QwtPlot::xBottom, xm - 0.5 * dx, xm + 0.5 * dx);
    }
}

void SimBoxView::setSlicePlane(int i)
{
    if (i < 0 || i > 2)
        return;
    const char *lbl[] = {"Z", "X", "Y"};
    sliderLbl->setText(lbl[i]);
    setSlicePos(sliceSlider->value());
}

void SimBoxView::setSlicePos(int p)
{
    int i = sliceSelector->checkedId();
    i = (i + 2) % 3;
    v = box.x0[i] + (box.x1[i] - box.x0[i]) * p / sliceSlider->maximum();
    updatePlot();
}
