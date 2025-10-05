#include "simboxview.h"

#include <cmath>

#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>

#include <QtGui/QRgb>
#include <QtGui/QScreen>

#include <QtDataVisualization/QCustom3DItem>
#include <QtDataVisualization/q3dcamera.h>
#include <QtDataVisualization/q3dinputhandler.h>
#include <QtDataVisualization/q3dscatter.h>
#include <QtDataVisualization/q3dscene.h>
#include <QtDataVisualization/q3dtheme.h>
#include <QtDataVisualization/qcustom3dlabel.h>
#include <QtDataVisualization/qcustom3dvolume.h>
#include <QtDataVisualization/qvalue3daxis.h>

#include <QValue3DAxisFormatter>

using namespace QtDataVisualization;

class NullAxisFormatter : public QValue3DAxisFormatter
{
public:
    explicit NullAxisFormatter(QObject *parent = nullptr)
        : QValue3DAxisFormatter(parent)
    {}
    virtual ~NullAxisFormatter() {}

protected:
    virtual void recalculate() {}
    virtual QString stringForValue(qreal value, const QString &format) const { return QString(); }
    virtual float positionAt(float value) const { return 0.0f; }
    virtual float valueAt(float position) const { return 0.0f; }

    QVector<float> &gridPositions() const { return const_cast<QVector<float> &>(v_); }
    QVector<float> &subGridPositions() const { return const_cast<QVector<float> &>(v_); }
    QVector<float> &labelPositions() const { return const_cast<QVector<float> &>(v_); }
    QStringList &labelStrings() const { return const_cast<QStringList &>(s_); }

private:
    Q_DISABLE_COPY(NullAxisFormatter)

    QStringList s_;
    QVector<float> v_;
};

SimBoxView::SimBoxView(QWidget *parent) : QFrame(parent), simbox(12)
{
    setFrameStyle(QFrame::StyledPanel | QFrame::Raised);

    graph = new Q3DScatter();
    container = QWidget::createWindowContainer(graph);

    QSize screenSize = graph->screen()->size();
    container->setMinimumSize(QSize(150, 150));
    container->setMaximumSize(screenSize);
    container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    container->setFocusPolicy(Qt::StrongFocus);

    createCtrls();
    create3DScene();

    connect(sliceEnable, &QToolButton::toggled, this, &SimBoxView::on_sliceEnable_toggled);
    connect(sliceSelector, &QButtonGroup::idClicked, this, &SimBoxView::on_sliceSelector_idClicked);
    connect(sliceSlider, &QSlider::valueChanged, this, &SimBoxView::on_sliceSlider_sliderMoved);

    resize(330, 330);
}

SimBoxView::~SimBoxView()
{
    delete graph;
}

bool SimBoxView::isSupported()
{
    // QCustom3DVolume is not supported on OpenGL ES 2.0
#if defined(QT_OPENGL_ES_2)
    return false;
#elif (QT_VERSION < QT_VERSION_CHECK(5, 3, 0))
    return true;
#else
    Q3DScatter *graph = new Q3DScatter();
    bool ret = false;

    if (graph->hasContext()) {
        ret = !QOpenGLContext::currentContext()->isOpenGLES();
    }

    delete graph;
    return ret;
#endif
}

QVector<QRgb> SimBoxView::colorTable() const
{
    if (volume)
        return volume->colorTable();
    QVector<QRgb> d;
    return d;
}

void SimBoxView::setScale(const QVector3D &L0)
{
    scaleFactor = 1.f / qMax(L0.x(), qMax(L0.y(), L0.z()));
    QVector3D L(L0);
    L *= scaleFactor;
    if (volume) {
        volume->setScaling(L);
        volume->setPosition(QVector3D(0, 0, 0));
    }
    beamWidth = 0.01f * pow(L.x() * L.y() * L.z(), 1.0f / 3);

    // create box
    QVector3D l, p;
    float w = -beamWidth;
    int i = 0;

    // X
    l = QVector3D(L.x(), w, w);
    p = QVector3D(L.x() / 2, w / 2, w / 2);
    simbox[i]->setPosition(p);
    simbox[i++]->setScaling(l);
    p = QVector3D(L.x() / 2, w / 2, L.z() - w / 2);
    simbox[i]->setPosition(p);
    simbox[i++]->setScaling(l);
    p = QVector3D(L.x() / 2, L.y() - w / 2, w / 2);
    simbox[i]->setPosition(p);
    simbox[i++]->setScaling(l);
    p = QVector3D(L.x() / 2, L.y() - w / 2, L.z() - w / 2);
    simbox[i]->setPosition(p);
    simbox[i++]->setScaling(l);
    // Y
    l = QVector3D(w, L.y(), w);
    p = QVector3D(w / 2, L.y() / 2, w / 2);
    simbox[i]->setPosition(p);
    simbox[i++]->setScaling(l);
    p = QVector3D(w / 2, L.y() / 2, L.z() - w / 2);
    simbox[i]->setPosition(p);
    simbox[i++]->setScaling(l);
    p = QVector3D(L.x() - w / 2, L.y() / 2, w / 2);
    simbox[i]->setPosition(p);
    simbox[i++]->setScaling(l);
    p = QVector3D(L.x() - w / 2, L.y() / 2, L.z() - w / 2);
    simbox[i]->setPosition(p);
    simbox[i++]->setScaling(l);
    // Z
    l = QVector3D(w, w, L.z() + 2 * beamWidth);
    p = QVector3D(w / 2, w / 2, L.z() / 2);
    simbox[i]->setPosition(p);
    simbox[i++]->setScaling(l);
    p = QVector3D(w / 2, L.y() - w / 2, L.z() / 2);
    simbox[i]->setPosition(p);
    simbox[i++]->setScaling(l);
    p = QVector3D(L.x() - w / 2, w / 2, L.z() / 2);
    simbox[i]->setPosition(p);
    simbox[i++]->setScaling(l);
    p = QVector3D(L.x() - w / 2, L.y() - w / 2, L.z() / 2);
    simbox[i]->setPosition(p);
    simbox[i++]->setScaling(l);

    for (int i = 0; i < 12; i++) {
        p = simbox[i]->position();
        simbox[i]->setPosition(p - 0.5 * L);
    }

    graph->requestUpdate();
}

void SimBoxView::setBackgroundColor(QColor clr)
{
    if (graph) {
        graph->activeTheme()->setWindowColor(clr);
        graph->activeTheme()->setBackgroundColor(clr);
    }
}

void SimBoxView::fill(const QVector3D &lowerLeft, const QVector3D &upperRight, const QColor &clr,
                      bool drawBorder)
{
    // check if color exists
    QVector<QRgb> colorTable = volume->colorTable();
    int c = colorTable.indexOf(clr.rgba());
    if (c < 0) {
        colorTable.push_back(clr.rgba());
        volume->setColorTable(colorTable);
        c = colorTable.size() - 1;
    }

    QVector3D x0(lowerLeft), x1(upperRight);
    x0 *= scaleFactor;
    x1 *= scaleFactor;
    QVector3D l = volume->scaling();
    int ix0[3], ix1[3];
    for (int i = 0; i < 3; ++i) {
        ix0[i] = floor(x0[i] / l[i] * textureWidth * 0.999);
        if (ix0[i] < 0)
            ix0[i] = 0;
        if (ix0[i] > textureWidth - 1)
            ix0[i] = textureWidth - 1;
        ix1[i] = floor(x1[i] / l[i] * textureWidth * 0.999);
        if (ix1[i] < 0)
            ix1[i] = 0;
        if (ix1[i] > textureWidth - 1)
            ix1[i] = textureWidth - 1;
    }

    uchar *d = volume->textureData()->data();
    for (int x = ix0[0]; x <= ix1[0]; ++x) {
        for (int y = ix0[1]; y < ix1[1]; ++y) {
            for (int z = ix0[2]; z < ix1[2]; ++z) {
                d[idx(x, y, z)] = c;
            }
        }
    }

    if (drawBorder) {
        int x, y, z;

        // x lines
        y = ix0[1], z = ix0[2];
        for (x = ix0[0]; x <= ix1[0]; ++x)
            d[idx(x, y, z)] = 1;
        y = ix0[1], z = ix1[2];
        for (x = ix0[0]; x <= ix1[0]; ++x)
            d[idx(x, y, z)] = 1;
        y = ix1[1], z = ix0[2];
        for (x = ix0[0]; x <= ix1[0]; ++x)
            d[idx(x, y, z)] = 1;
        y = ix1[1], z = ix1[2];
        for (x = ix0[0]; x <= ix1[0]; ++x)
            d[idx(x, y, z)] = 1;

        // y lines
        x = ix0[0], z = ix0[2];
        for (y = ix0[1]; y <= ix1[1]; ++y)
            d[idx(x, y, z)] = 1;
        x = ix0[0], z = ix1[2];
        for (y = ix0[1]; y <= ix1[1]; ++y)
            d[idx(x, y, z)] = 1;
        x = ix1[0], z = ix0[2];
        for (y = ix0[1]; y <= ix1[1]; ++y)
            d[idx(x, y, z)] = 1;
        x = ix1[0], z = ix1[2];
        for (y = ix0[1]; y <= ix1[1]; ++y)
            d[idx(x, y, z)] = 1;

        // z lines
        x = ix0[0], y = ix0[1];
        for (z = ix0[2]; z <= ix1[2]; ++z)
            d[idx(x, y, z)] = 1;
        x = ix0[0], y = ix1[1];
        for (z = ix0[2]; z <= ix1[2]; ++z)
            d[idx(x, y, z)] = 1;
        x = ix1[0], y = ix0[1];
        for (z = ix0[2]; z <= ix1[2]; ++z)
            d[idx(x, y, z)] = 1;
        x = ix1[0], y = ix1[1];
        for (z = ix0[2]; z <= ix1[2]; ++z)
            d[idx(x, y, z)] = 1;
    }

    volume->setTextureData(volume->textureData());
    graph->requestUpdate();
}

void SimBoxView::clearVolume()
{
    if (volume) {
        volume->textureData()->fill(0);
        volume->setTextureData(volume->textureData());
        QVector<QRgb> colorTable(2);
        colorTable[0] = qRgba(0, 0, 0, 0);
        colorTable[1] = qRgba(0, 0, 0, 255);
        volume->setColorTable(colorTable);
    }
    graph->requestUpdate();
}

int SimBoxView::create3DScene()
{
    Q3DTheme *th = graph->activeTheme();
    th->setType(Q3DTheme::ThemeQt);
    QColor bckClr = palette().color(QPalette::Window);
    th->setWindowColor(bckClr);
    th->setBackgroundColor(bckClr);
    th->setBackgroundEnabled(false);

    th->setGridEnabled(false);
    th->setLabelBackgroundEnabled(false);
    th->setLabelBorderEnabled(false);
    //th->setLabelTextColor(bckClr);

    graph->setShadowQuality(QAbstract3DGraph::ShadowQualityNone);
    graph->scene()->activeCamera()->setCameraPreset(Q3DCamera::CameraPresetIsometricLeft);
    graph->setOrthoProjection(true);
    //graph->activeTheme()->setBackgroundEnabled(false);

    // Only allow zooming at the center and limit the zoom to 200% to avoid clipping issues
    static_cast<Q3DInputHandler *>(graph->activeInputHandler())->setZoomAtTargetEnabled(false);
    // graph->scene()->activeCamera()->setMaxZoomLevel(500.0f);
    graph->scene()->activeCamera()->setZoomLevel(100.f);

    graph->setAspectRatio(1);
    graph->setHorizontalAspectRatio(1);

    const char *axLbl[] = {"X", "Y", "Z"};
    int k = 0;
    const float beamW = 0.005f;
    for (auto ax : graph->axes()) {
        ax->setRange(-0.5 - 2 * beamW, 0.5 + 2 * beamW);
        ax->setTitle(axLbl[k++]);
        ax->setTitleVisible(false);
        ax->setFormatter(new NullAxisFormatter);
    }

    volume = new QCustom3DVolume;
    volume->setScalingAbsolute(false);
    volume->setTextureWidth(textureWidth);
    volume->setTextureHeight(textureWidth);
    volume->setTextureDepth(textureWidth);
    volume->setTextureFormat(QImage::Format_Indexed8);
    volume->setTextureData(new QVector<uchar>(textureSize, 0));
    volume->setScaling(QVector3D(1, 1, 1));
    volume->setPosition(QVector3D(0.5f, 0.5f, 0.5f));
    scaleFactor = 1;
    beamWidth = 0.005f;

    QVector<QRgb> colorTable(2);
    colorTable[0] = qRgba(0, 0, 0, 0);
    colorTable[1] = qRgba(0, 0, 0, 255);
    volume->setColorTable(colorTable);

    graph->addCustomItem(volume);

    QImage color = QImage(2, 2, QImage::Format_RGB32);
    color.fill(Qt::black);
    QVector3D p(0, 0, 0), l(1, 1, 1);
    for (int i = 0; i < 12; i++) {
        simbox[i] = new QCustom3DItem(":/assets/3d/cubeFlat.obj", p, l, QQuaternion(), color);
        simbox[i]->setScalingAbsolute(false);
        graph->addCustomItem(simbox[i]);
    }

    setScale(1, 1, 1);

    return 0;
}

int SimBoxView::createCtrls()
{
    QWidget *slicerGroup = new QWidget;
    slicerGroup->setStyleSheet("background: white");
    {
        QHBoxLayout *hbox1 = new QHBoxLayout;
        // hbox1->setContentsMargins(2, 2, 2, 2);
        slicerGroup->setLayout(hbox1);

        sliceEnable = new QToolButton;
        sliceEnable->setText("Slice");
        sliceEnable->setCheckable(true);
        hbox1->addWidget(sliceEnable);

        QFrame *h1 = new QFrame;
        h1->setFrameStyle(QFrame::Raised | QFrame::VLine);
        hbox1->addWidget(h1);

        sliceSelector = new QButtonGroup(this);
        sliceSelector->setExclusive(true);
        QToolButton *bt[3];
        QHBoxLayout *hbox = new QHBoxLayout;
        hbox->setContentsMargins(0, 0, 0, 0);
        hbox->setSpacing(0);
        const char *btLbl[] = {"YZ", "XZ", "XY"};
        for (int i = 0; i < 3; i++) {
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

        sliceSlider = new QSlider(Qt::Horizontal, this);
        sliceSlider->setMinimum(0);
        sliceSlider->setMaximum(1024);
        sliceSlider->setValue(512);
        sliceSlider->setEnabled(true);
        sliceSlider->setMinimumWidth(80);
        hbox1->addWidget(sliceSlider);
    }

    // QLabel *title = new QLabel("Simulation Box");
    // title->setAlignment(Qt::AlignCenter);

    QVBoxLayout *vLayout = new QVBoxLayout(this);
    // vLayout->addWidget(title);
    vLayout->addWidget(container, 1);
    vLayout->addWidget(slicerGroup);
    vLayout->setContentsMargins(0, 0, 0, 0);
    vLayout->setSpacing(0);

    sliceSlider->setEnabled(false);
    for (auto button : sliceSelector->buttons())
        button->setEnabled(false);

    return 0;
}

void SimBoxView::updateSlice()
{
    sliceSlider->setEnabled(sliceEnable->isChecked());
    for (auto button : sliceSelector->buttons())
        button->setEnabled(sliceEnable->isChecked());

    if (!volume)
        return;

    if (sliceEnable->isChecked()) {
        int si[3] = {-1, -1, -1};
        int id = sliceSelector->checkedId();
        si[id] = (int)floor(0.99f * sliceSlider->sliderPosition() / 1024 * textureWidth);
        if (id == 2)
            si[id] = textureWidth - 1 - si[id];
        volume->setSliceIndices(si[0], si[2], si[1]);
        volume->setDrawSlices(true);
        volume->setDrawSliceFrames(true);
    } else {
        sliceSlider->setEnabled(false);

        volume->setSliceIndices(-1, -1, -1);
        volume->setDrawSlices(false);
        volume->setDrawSliceFrames(false);
    }
}

void SimBoxView::on_sliceEnable_toggled(bool)
{
    updateSlice();
}

void SimBoxView::on_sliceSelector_idClicked(int)
{
    updateSlice();
}

void SimBoxView::on_sliceSlider_sliderMoved(int)
{
    updateSlice();
}
