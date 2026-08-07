#include "trackviewwidget.h"

#include "track3dviewport.h"

#include <fstream>

#include <nlohmann/json.hpp>

#include <QAction>
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QDebug>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QIcon>
#include <QImage>
#include <QPainter>
#include <QPixmap>
#include <QPlainTextEdit>
#include <QPolygonF>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSizePolicy>
#include <QFontMetrics>
#include <QSpinBox>
#include <QSplitter>
#include <QStyle>
#include <QTabWidget>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>
#include <QSvgRenderer>
#include <qlabel.h>

static float jsonNumber(const nlohmann::json &j, const char *key, float fallback)
{
    auto it = j.find(key);
    return (it != j.end() && it->is_number()) ? it->get<float>() : fallback;
}

namespace {

// Oblique cube: front square + back square offset (+6,-6), viewBox 32x32
constexpr double FX0 = 5, FY0 = 11, FX1 = 21, FY1 = 27;
constexpr double DX = 6, DY = -6;

const QPointF ftl{ FX0, FY0 }, ftr{ FX1, FY0 };
const QPointF fbl{ FX0, FY1 }, fbr{ FX1, FY1 };
const QPointF btl{ FX0 + DX, FY0 + DY }, btr{ FX1 + DX, FY0 + DY };
const QPointF bbl{ FX0 + DX, FY1 + DY }, bbr{ FX1 + DX, FY1 + DY };

QString polygonFor(Track3DViewport::View f)
{
    const QPointF *q[4] = {};
    switch (f) {
    case Track3DViewport::Front:
        q[0] = &ftl;
        q[1] = &ftr;
        q[2] = &fbr;
        q[3] = &fbl;
        break;
    case Track3DViewport::Back:
        q[0] = &btl;
        q[1] = &btr;
        q[2] = &bbr;
        q[3] = &bbl;
        break;
    case Track3DViewport::Left:
        q[0] = &ftl;
        q[1] = &btl;
        q[2] = &bbl;
        q[3] = &fbl;
        break;
    case Track3DViewport::Right:
        q[0] = &ftr;
        q[1] = &btr;
        q[2] = &bbr;
        q[3] = &fbr;
        break;
    case Track3DViewport::Top:
        q[0] = &ftl;
        q[1] = &ftr;
        q[2] = &btr;
        q[3] = &btl;
        break;
    case Track3DViewport::Bottom:
        q[0] = &fbl;
        q[1] = &fbr;
        q[2] = &bbr;
        q[3] = &bbl;
        break;
    default:
        return {};
    }
    QString pts;
    for (int i = 0; i < 4; ++i)
        pts += QString("%1,%2 ").arg(q[i]->x()).arg(q[i]->y());
    return QString("<polygon points='%1' fill='%2'/>").arg(pts.trimmed(), QStringLiteral("COL"));
}

QString edgesPath()
{
    const QPointF *e[12][2] = {
        { &ftl, &ftr }, { &ftr, &fbr }, { &fbr, &fbl }, { &fbl, &ftl }, // front
        { &btl, &btr }, { &btr, &bbr }, { &bbr, &bbl }, { &bbl, &btl }, // back
        { &ftl, &btl }, { &ftr, &btr }, { &fbl, &bbl }, { &fbr, &bbr }
    }; // connectors
    QString d;
    for (auto &seg : e)
        d += QString("M%1,%2 L%3,%4 ")
                     .arg(seg[0]->x())
                     .arg(seg[0]->y())
                     .arg(seg[1]->x())
                     .arg(seg[1]->y());
    return QString("<path d='%1' fill='none' stroke='COL' stroke-width='1.5' "
                   "stroke-linecap='round' stroke-linejoin='round'/>")
            .arg(d.trimmed());
}

QString buildSvg(Track3DViewport::View face, const QColor &color)
{
    QString svg = QStringLiteral("<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 32 32'>");
    if (face != Track3DViewport::Iso)
        svg += polygonFor(face);
    svg += edgesPath();
    svg += QStringLiteral("</svg>");
    svg.replace(QLatin1String("COL"), color.name(QColor::HexRgb));
    return svg;
}

} // namespace

QIcon makeViewIcon(Track3DViewport::View face, const QColor &color)
{
    QSvgRenderer renderer(buildSvg(face, color).toUtf8());

    QIcon icon;
    for (int size : { 12, 16, 22, 24, 32, 48, 64 }) { // covers 1x/2x at all toolbar sizes
        QPixmap pm(size, size);
        pm.fill(Qt::transparent);
        QPainter p(&pm);
        p.setRenderHint(QPainter::Antialiasing);
        renderer.render(&p); // scales to pixmap rect
        p.end();
        icon.addPixmap(pm);
    }
    return icon;
}

TrackViewWidget::TrackViewWidget(McDriverObj *driver, QWidget *parent) : QWidget(parent)
{
    view_ = new Track3DViewport(driver, this);
    TrackColorBar *colorBar = new TrackColorBar(view_);

    QFrame *frm = new QFrame;
    frm->setFrameShape(QFrame::StyledPanel);
    frm->setFrameShadow(QFrame::Sunken);
    QVBoxLayout *frmLay = new QVBoxLayout(frm);
    frmLay->setContentsMargins(0, 0, 0, 0);
    frmLay->setSpacing(0);
    frmLay->addWidget(buildToolBar_());
    frmLay->addWidget(view_);

    QWidget *viewArea = new QWidget;
    QHBoxLayout *viewLay = new QHBoxLayout(viewArea);
    viewLay->setContentsMargins(0, 0, 0, 0);
    viewLay->addWidget(frm, 1);
    viewLay->addWidget(colorBar);

    QWidget *left = new QWidget;
    QVBoxLayout *leftLay = new QVBoxLayout(left);
    // left->setStyleSheet("border: 1px solid black;");
    leftLay->setContentsMargins(0, 0, 0, 0);
    leftLay->setSpacing(0);
    leftLay->addWidget(viewArea, 1);
    // leftLay->addWidget(buildToolBar_());

    QTabWidget *tabs = new QTabWidget;
    tabs->addTab(buildCaptureTab_(), tr("Capture"));
    tabs->addTab(buildColorTab_(), tr("Color"));
    tabs->addTab(buildCameraTab_(), tr("Camera"));

    QPlainTextEdit *info = new QPlainTextEdit;
    info->setReadOnly(true);
    connect(view_->cascadeRecorder(), &CascadeRecorder::statusUpdate, info,
            &QPlainTextEdit::setPlainText);

    QSplitter *rightSplit = new QSplitter(Qt::Vertical);
    rightSplit->addWidget(tabs);
    rightSplit->addWidget(info);
    rightSplit->setStretchFactor(0, 0);
    rightSplit->setStretchFactor(1, 1);
    rightSplit->setSizes({ 200, 300 });

    QWidget *right = new QWidget;
    QVBoxLayout *rightLay = new QVBoxLayout(right);
    rightLay->setContentsMargins(0, 0, 0, 0);
    rightLay->addWidget(rightSplit, 1);

    QSplitter *split = new QSplitter(Qt::Horizontal);
    split->addWidget(left);
    split->addWidget(right);
    split->setStretchFactor(0, 1);
    split->setSizes({ 720, 280 });

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    frm->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    viewArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    left->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    right->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    split->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->addWidget(split);
}

QToolBar *TrackViewWidget::buildToolBar_()
{
    CascadeRecorder *R = view_->cascadeRecorder();

    QToolBar *tb = new QToolBar;
    tb->setMovable(false);
    tb->setIconSize(QSize(22, 22));
    tb->setStyleSheet("background: white;");
    // tb->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    QAction *cap = tb->addAction(tr("Capture on"));
    cap->setCheckable(true);

    // The button text changes with recorder state ("Capture on/off/Paused"),
    // which would otherwise reflow the whole toolbar on every state change.
    // Reserve width for the widest label up front so the button size is fixed.
    if (auto *capBtn = qobject_cast<QToolButton *>(tb->widgetForAction(cap))) {
        QFontMetrics fm(capBtn->font());
        int w = 0;
        for (const QString &s : { tr("Capture on"), tr("Capture off"), tr("Paused") })
            w = qMax(w, fm.horizontalAdvance(s));
        capBtn->setMinimumWidth(w + 4 * capBtn->style()->pixelMetric(QStyle::PM_ButtonMargin));
        capBtn->setAutoRaise(false); // keep the button background visible for the text
    }

    connect(cap, &QAction::toggled, R, &CascadeRecorder::capture);
    // connect(view_, &Track3DViewport::captureChanged, cap, [cap](bool on) {
    //     QSignalBlocker block(cap); // don't let setChecked re-emit toggled()
    //     cap->setChecked(on);
    //     cap->setText(on ? tr("Capture off") : tr("Capture on"));
    // });
    connect(R, &CascadeRecorder::stateChange, cap,
            [cap](CascadeRecorder::State from, CascadeRecorder::State to) {
                QSignalBlocker block(cap); // don't let setChecked re-emit toggled()
                switch (to) {
                case CascadeRecorder::Idle:
                    cap->setChecked(false);
                    cap->setText(tr("Capture on"));
                    break;
                case CascadeRecorder::Capturing:
                case CascadeRecorder::Finishing:
                    cap->setChecked(true);
                    cap->setText(tr("Capture off"));
                    break;
                case CascadeRecorder::Paused:
                    cap->setChecked(true);
                    cap->setText(tr("Paused"));
                    break;
                }
            });

    QAction *clr = tb->addAction(tr("Clear"));
    connect(clr, &QAction::triggered, R, &CascadeRecorder::clear);

    tb->addSeparator();

    QAction *shot = tb->addAction(QIcon(":/assets/ionicons/camera-outline.svg"), QString());
    shot->setToolTip(tr("Screenshot"));
    connect(shot, &QAction::triggered, this, &TrackViewWidget::saveScreenshot_);

    tb->addSeparator();

    struct
    {
        const char *tip;
        Track3DViewport::View view;
    } views[] = { { "Home/Iso", Track3DViewport::Iso },  { "Top", Track3DViewport::Top },
                  { "Bottom", Track3DViewport::Bottom }, { "Front", Track3DViewport::Front },
                  { "Back", Track3DViewport::Back },     { "Left", Track3DViewport::Left },
                  { "Right", Track3DViewport::Right } };
    QColor iconColor = palette().color(QPalette::ButtonText);
    for (const auto &v : views) {
        // QAction *a = tb->addAction(viewIcon(v.view), QString());
        QAction *a = tb->addAction(makeViewIcon(v.view, iconColor), QString());
        a->setToolTip(tr(v.tip));
        const Track3DViewport::View vw = v.view;
        connect(a, &QAction::triggered, view_, [this, vw]() {
            if (vw == Track3DViewport::Iso)
                view_->homeView();
            else
                view_->setPresetView(vw);
        });
    }

    return tb;
}

QWidget *TrackViewWidget::buildCaptureTab_()
{
    CascadeRecorder *R = view_->cascadeRecorder();

    QWidget *w = new QWidget;
    QFormLayout *form = new QFormLayout(w);

    QPushButton *batchBt = new QPushButton(tr("Batch"));
    batchBt->setCheckable(true);
    batchBt->setToolTip(tr("Capture N cascades and stop"));

    QPushButton *ringBt = new QPushButton(tr("Ring"));
    ringBt->setCheckable(true);
    ringBt->setToolTip(tr("Continuously capture cascades in a N-size ring buffer"));
    ringBt->setChecked(true); // matches CascadeRecorder's default Mode::Ring

    QButtonGroup *modeGroup = new QButtonGroup(w);
    modeGroup->setExclusive(true);
    modeGroup->addButton(batchBt);
    modeGroup->addButton(ringBt);
    connect(ringBt, &QPushButton::toggled, R, &CascadeRecorder::setRingMode);

    QHBoxLayout *modeLay = new QHBoxLayout;
    modeLay->setContentsMargins(0, 0, 0, 0);
    modeLay->setSpacing(0);
    modeLay->addWidget(batchBt);
    modeLay->addWidget(ringBt);
    form->addRow(tr("Buffer Mode"), modeLay);

    {
        QFrame *frm = new QFrame;
        frm->setFrameStyle(QFrame::HLine);
        form->addRow(frm);
    }

    {
        QDoubleSpinBox *spdBox = new QDoubleSpinBox;
        spdBox->setRange(0.1, 10.0);
        spdBox->setSingleStep(0.1);
        spdBox->setValue(1.0);
        connect(spdBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), R,
                &CascadeRecorder::setPlaybackSpeed);
        form->addRow(tr("Speed [ns/s]"), spdBox);

        QSlider *sldr = new QSlider(Qt::Horizontal);
        const int N = 20;
        sldr->setRange(-N, N);
        sldr->setValue(0);
        connect(sldr, &QSlider::valueChanged, R,
                [R](int v) { R->setPlaybackSpeed(std::pow(10.0, v * 1.0 / N)); });
        form->addRow(sldr);

        // bind slider + spin box
        connect(spdBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), sldr,
                [sldr](double v) {
                    QSignalBlocker block(sldr);
                    sldr->setValue(std::log10(v) * N);
                });
        connect(sldr, &QSlider::valueChanged, spdBox, [spdBox](int v) {
            QSignalBlocker block(spdBox);
            spdBox->setValue(std::pow(10.0, v * 1.0 / N));
        });

        const char *eTip = "Simulation playback speed. "
                           "The simulation time will advance at this rate relative to real time.";
        spdBox->setToolTip(tr(eTip));
        sldr->setToolTip(tr(eTip));
        form->labelForField(spdBox)->setToolTip(tr(eTip));
    }

    {
        QFrame *frm = new QFrame;
        frm->setFrameStyle(QFrame::HLine | QFrame::Sunken);
        form->addRow(frm);
    }

    form->addRow(new QLabel("Buffer Size"));

    QSpinBox *nBox = new QSpinBox;
    nBox->setRange(1, 100);
    nBox->setValue(R->nCascades());
    connect(nBox, QOverload<int>::of(&QSpinBox::valueChanged), R, &CascadeRecorder::setNCascades);
    form->addRow(tr("Cascades"), nBox);
    {
        const char *eTip =
                "Number of cascades to capture. "
                "In ring buffer mode, older cascades will be dropped when the buffer is full.";
        nBox->setToolTip(tr(eTip));
        form->labelForField(nBox)->setToolTip(tr(eTip));
    }

    QSpinBox *memBox = new QSpinBox;
    memBox->setRange(0, 2000);
    // memBox->setSpecialValueText(tr("off"));
    memBox->setValue(R->memoryCapMB());
    connect(memBox, QOverload<int>::of(&QSpinBox::valueChanged), R,
            &CascadeRecorder::setMemoryCapMB);
    form->addRow(tr("Mem [MB]"), memBox);

    {
        QFrame *frm = new QFrame;
        frm->setFrameStyle(QFrame::HLine | QFrame::Sunken);
        form->addRow(frm);
    }

    form->addRow(new QLabel("Ion track thresholds"));

    QDoubleSpinBox *eThrBox = new QDoubleSpinBox;
    eThrBox->setRange(0.001, 1e9);
    eThrBox->setDecimals(3);
    eThrBox->setValue(view_->energyMin());
    connect(eThrBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), view_,
            &Track3DViewport::setEnergyThreshold);
    form->addRow(tr("E min [eV]"), eThrBox);
    {
        const char *eTip = "Ion energy threshold for track capture. "
                           "Tracks with energy below this value will be dropped.";
        eThrBox->setToolTip(tr(eTip));
        form->labelForField(eThrBox)->setToolTip(tr(eTip));
    }

    QSpinBox *genBox = new QSpinBox;
    genBox->setRange(-1, 20);
    genBox->setValue(-1);
    genBox->setSpecialValueText(tr("all"));
    connect(genBox, QOverload<int>::of(&QSpinBox::valueChanged), R, &CascadeRecorder::setGenCutoff);
    form->addRow(tr("Max Recoil gen."), genBox);
    {
        const char *eTip = "Maximum recoil generation for track capture. "
                           "Recoil generations above this value will be dropped.";
        genBox->setToolTip(tr(eTip));
        form->labelForField(genBox)->setToolTip(tr(eTip));
    }

    return w;
}

QWidget *TrackViewWidget::buildColorTab_()
{
    QWidget *w = new QWidget;
    QFormLayout *form = new QFormLayout(w);

    QComboBox *colorBox = new QComboBox;
    colorBox->addItems({ tr("Recoil Generation"), tr("Energy"), tr("Atomic Species") });
    connect(colorBox, QOverload<int>::of(&QComboBox::currentIndexChanged), view_,
            &Track3DViewport::setColorMode);
    form->addRow(tr("Mode"), colorBox);

    QComboBox *colorMap = new QComboBox;
    connect(colorMap, QOverload<int>::of(&QComboBox::currentIndexChanged), view_,
            &Track3DViewport::setColorMap);
    form->addRow(tr("Color Map"), colorMap);

    // continuous maps for the energy scale, discrete ones for gen/species
    auto fillMaps = [colorMap](int mode) {
        QSignalBlocker block(colorMap);
        colorMap->clear();
        if (mode == Track3DViewport::Energy)
            colorMap->addItems({ tr("Ramp"), tr("Viridis"), tr("Turbo") });
        else
            colorMap->addItems({ tr("Default"), tr("Tab10") });
    };
    fillMaps(view_->colorMode());
    connect(colorBox, QOverload<int>::of(&QComboBox::currentIndexChanged), colorMap,
            [this, fillMaps](int mode) {
                fillMaps(mode);
                view_->setColorMap(0);
            });

    {
        QFrame *frm = new QFrame;
        frm->setFrameStyle(QFrame::HLine | QFrame::Sunken);
        form->addRow(frm);
    }

    form->addRow(new QLabel("Energy scale"));

    QCheckBox *logBox = new QCheckBox(tr("Log E"));
    logBox->setChecked(view_->energyLog());
    connect(logBox, &QCheckBox::toggled, view_, &Track3DViewport::setEnergyLog);
    form->addRow(logBox);

    QCheckBox *autoBox = new QCheckBox(tr("Auto Scale"));
    autoBox->setChecked(view_->energyAuto());
    form->addRow(autoBox);

    QDoubleSpinBox *escaleMin = new QDoubleSpinBox;
    escaleMin->setRange(0.001, 1e9);
    escaleMin->setDecimals(3);
    escaleMin->setValue(view_->energyMin());
    escaleMin->setEnabled(!view_->energyAuto());
    connect(escaleMin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), view_,
            &Track3DViewport::setEnergyUserMin);
    form->addRow(tr("min [eV]"), escaleMin);

    QDoubleSpinBox *escaleMax = new QDoubleSpinBox;
    escaleMax->setRange(0.001, 1e9);
    escaleMax->setDecimals(3);
    escaleMax->setValue(view_->energyMax());
    escaleMax->setEnabled(!view_->energyAuto());
    connect(escaleMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged), view_,
            &Track3DViewport::setEnergyUserMax);
    form->addRow(tr("max [eV]"), escaleMax);

    connect(autoBox, &QCheckBox::toggled, view_, &Track3DViewport::setEnergyAuto);
    connect(autoBox, &QCheckBox::toggled, escaleMin, &QWidget::setDisabled);
    connect(autoBox, &QCheckBox::toggled, escaleMax, &QWidget::setDisabled);
    connect(autoBox, &QCheckBox::toggled, this, [this, escaleMin, escaleMax](bool on) {
        if (!on) {
            QSignalBlocker b1(escaleMin), b2(escaleMax);
            escaleMin->setValue(view_->energyMin());
            escaleMax->setValue(view_->energyMax());
        }
    });

    return w;
}

QWidget *TrackViewWidget::buildCameraTab_()
{
    QWidget *w = new QWidget;
    QFormLayout *form = new QFormLayout(w);

    // preset views and Home live on the toolbar; this tab persists the view
    form->addRow(new QLabel(tr("Camera state")));

    QPushButton *saveBt = new QPushButton(tr("Save to file..."));
    saveBt->setToolTip(tr("Save the current view to a JSON file"));
    connect(saveBt, &QPushButton::clicked, this, &TrackViewWidget::saveCamera_);
    form->addRow(saveBt);

    QPushButton *loadBt = new QPushButton(tr("Load from file..."));
    loadBt->setToolTip(tr("Restore a saved view from a JSON file"));
    connect(loadBt, &QPushButton::clicked, this, &TrackViewWidget::loadCamera_);
    form->addRow(loadBt);

    return w;
}

void TrackViewWidget::saveCamera_()
{
    const QString filter = tr("Json files [*.json](*.json);; All files (*.*)");
    QString path = QFileDialog::getSaveFileName(this, tr("Save camera"), QString(), filter);
    if (path.isEmpty())
        return;
    if (QFileInfo(path).suffix().toLower() != "json")
        path += ".json";

    const Track3DViewport::CameraState c = view_->camera();
    nlohmann::json j = { { "yaw", c.yaw },
                         { "pitch", c.pitch },
                         { "dist", c.dist },
                         { "center", { c.center.x(), c.center.y(), c.center.z() } } };

    std::ofstream os(path.toLatin1().constData());
    if (!os) {
        qWarning() << "TrackViewWidget: cannot write camera file:" << path;
        return;
    }
    os << j.dump(2) << std::endl;
}

void TrackViewWidget::loadCamera_()
{
    const QString filter = tr("Json files [*.json](*.json);; All files (*.*)");
    QString path = QFileDialog::getOpenFileName(this, tr("Load camera"), QString(), filter);
    if (path.isEmpty())
        return;

    std::ifstream is(path.toLatin1().constData());
    if (!is) {
        qWarning() << "TrackViewWidget: cannot read camera file:" << path;
        return;
    }
    nlohmann::json j = nlohmann::json::parse(is, nullptr, false);
    if (j.is_discarded() || !j.is_object()) {
        qWarning() << "TrackViewWidget: invalid camera file:" << path;
        return;
    }

    Track3DViewport::CameraState c = view_->camera();
    c.yaw = jsonNumber(j, "yaw", c.yaw);
    c.pitch = jsonNumber(j, "pitch", c.pitch);
    c.dist = jsonNumber(j, "dist", c.dist);
    auto ctr = j.find("center");
    if (ctr != j.end() && ctr->is_array() && ctr->size() == 3 && (*ctr)[0].is_number()
        && (*ctr)[1].is_number() && (*ctr)[2].is_number())
        c.center =
                QVector3D((*ctr)[0].get<float>(), (*ctr)[1].get<float>(), (*ctr)[2].get<float>());
    view_->setCamera(c);
}

void TrackViewWidget::saveScreenshot_()
{
    QString path = QFileDialog::getSaveFileName(this, tr("Save screenshot"), QString(),
                                                tr("PNG image [*.png](*.png);; All files (*.*)"));
    if (path.isEmpty())
        return;
    if (QFileInfo(path).suffix().isEmpty())
        path += ".png";

    const QImage img = view_->grabScreenshot();
    if (!img.save(path))
        qWarning() << "TrackViewWidget: failed to save screenshot:" << path;
}
