#include "trackviewwidget.h"

#include "track3dviewport.h"

#include <fstream>

#include <nlohmann/json.hpp>

#include <QAction>
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
#include <QSpinBox>
#include <QSplitter>
#include <QTabWidget>
#include <QToolBar>
#include <QVBoxLayout>

static float jsonNumber(const nlohmann::json &j, const char *key, float fallback)
{
    auto it = j.find(key);
    return (it != j.end() && it->is_number()) ? it->get<float>() : fallback;
}

static const float kCube[8][3] = { { 0, 0, 0 }, { 1, 0, 0 }, { 1, 1, 0 }, { 0, 1, 0 },
                                   { 0, 0, 1 }, { 1, 0, 1 }, { 1, 1, 1 }, { 0, 1, 1 } };
static const int kEdge[12][2] = { { 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 0 }, { 4, 5 }, { 5, 6 },
                                  { 6, 7 }, { 7, 4 }, { 0, 4 }, { 1, 5 }, { 2, 6 }, { 3, 7 } };
static const int kFace[6][4] = { { 0, 1, 2, 3 }, { 4, 5, 6, 7 }, { 3, 2, 6, 7 },
                                 { 0, 1, 5, 4 }, { 0, 3, 7, 4 }, { 1, 2, 6, 5 } };

static QPointF cubePt(int i)
{
    const float x = kCube[i][0], y = kCube[i][1], z = kCube[i][2];
    return QPointF(13.0 + (x - z) * 8.0, 13.0 - y * 8.0 + (x + z) * 4.0);
}

static QIcon viewIcon(int face)
{
    QPixmap pm(26, 26);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    if (face >= Track3DViewport::Front && face <= Track3DViewport::Right) {
        QPolygonF poly;
        for (int k = 0; k < 4; ++k)
            poly << cubePt(kFace[face][k]);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(30, 30, 30));
        p.drawPolygon(poly);
    }
    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(QColor(130, 130, 130), 1.0));
    for (auto &e : kEdge)
        p.drawLine(cubePt(e[0]), cubePt(e[1]));
    return QIcon(pm);
}

static QIcon cameraIcon()
{
    QPixmap pm(26, 26);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    p.setRenderHint(QPainter::Antialiasing);
    const QColor col(40, 40, 40);
    p.setPen(QPen(col, 1.6));
    p.setBrush(Qt::NoBrush);
    p.drawRect(9, 5, 7, 4);
    p.drawRoundedRect(4, 9, 18, 13, 2, 2);
    p.setBrush(col);
    p.drawEllipse(QPointF(13, 15.5), 4.0, 4.0);
    return QIcon(pm);
}

TrackViewWidget::TrackViewWidget(McDriverObj *driver, QWidget *parent) : QWidget(parent)
{
    view_ = new Track3DViewport(driver, this);
    TrackColorBar *colorBar = new TrackColorBar(view_);

    QFrame *frm = new QFrame;
    frm->setFrameShape(QFrame::StyledPanel);
    frm->setFrameShadow(QFrame::Sunken);
    QHBoxLayout *frmLay = new QHBoxLayout(frm);
    frmLay->setContentsMargins(0, 0, 0, 0);
    frmLay->addWidget(view_);

    QWidget *viewArea = new QWidget;
    QHBoxLayout *viewLay = new QHBoxLayout(viewArea);
    viewLay->setContentsMargins(0, 0, 0, 0);
    viewLay->addWidget(frm, 1);
    viewLay->addWidget(colorBar);

    QWidget *left = new QWidget;
    QVBoxLayout *leftLay = new QVBoxLayout(left);
    leftLay->setContentsMargins(0, 0, 0, 0);
    leftLay->addWidget(viewArea, 1);
    leftLay->addWidget(buildToolBar_());

    QTabWidget *tabs = new QTabWidget;
    tabs->addTab(buildPlaybackTab_(), tr("Playback"));
    tabs->addTab(buildColorTab_(), tr("Color"));
    tabs->addTab(buildLimitsTab_(), tr("Limits"));

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

    QPushButton *saveCamBt = new QPushButton(tr("Save camera..."));
    connect(saveCamBt, &QPushButton::clicked, this, &TrackViewWidget::saveCamera_);
    QPushButton *loadCamBt = new QPushButton(tr("Load camera..."));
    connect(loadCamBt, &QPushButton::clicked, this, &TrackViewWidget::loadCamera_);
    QHBoxLayout *camRow = new QHBoxLayout;
    camRow->setContentsMargins(0, 0, 0, 0);
    camRow->addWidget(saveCamBt);
    camRow->addWidget(loadCamBt);

    QWidget *right = new QWidget;
    QVBoxLayout *rightLay = new QVBoxLayout(right);
    rightLay->setContentsMargins(0, 0, 0, 0);
    rightLay->addWidget(rightSplit, 1);
    rightLay->addLayout(camRow);

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
    tb->setIconSize(QSize(24, 24));
    tb->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    QAction *cap = tb->addAction(tr("Capture on"));
    cap->setCheckable(true);
    connect(cap, &QAction::toggled, R, &CascadeRecorder::capture);
    connect(view_, &Track3DViewport::captureChanged, cap, [cap](bool on) {
        QSignalBlocker block(cap); // don't let setChecked re-emit toggled()
        cap->setChecked(on);
        cap->setText(on ? tr("Capture off") : tr("Capture on"));
    });

    QAction *clr = tb->addAction(tr("Clear"));
    connect(clr, &QAction::triggered, R, &CascadeRecorder::clear);

    tb->addSeparator();

    QAction *shot = tb->addAction(cameraIcon(), QString());
    shot->setToolTip(tr("Screenshot"));
    connect(shot, &QAction::triggered, this, &TrackViewWidget::saveScreenshot_);

    tb->addSeparator();

    struct
    {
        const char *tip;
        int view;
    } views[] = { { "Home", -1 },
                  { "Top", Track3DViewport::Top },
                  { "Bottom", Track3DViewport::Bottom },
                  { "Front", Track3DViewport::Front },
                  { "Back", Track3DViewport::Back },
                  { "Left", Track3DViewport::Left },
                  { "Right", Track3DViewport::Right } };
    for (const auto &v : views) {
        QAction *a = tb->addAction(viewIcon(v.view), QString());
        a->setToolTip(tr(v.tip));
        const int vw = v.view;
        connect(a, &QAction::triggered, view_, [this, vw]() {
            if (vw < 0)
                view_->homeView();
            else
                view_->setPresetView(vw);
        });
    }

    return tb;
}

QWidget *TrackViewWidget::buildPlaybackTab_()
{
    CascadeRecorder *R = view_->cascadeRecorder();

    QWidget *w = new QWidget;
    QFormLayout *form = new QFormLayout(w);

    QCheckBox *ringBox = new QCheckBox(tr("Ring buffer"));
    ringBox->setChecked(true);
    connect(ringBox, &QCheckBox::toggled, R, &CascadeRecorder::setRingMode);
    form->addRow(ringBox);

    QSpinBox *nBox = new QSpinBox;
    nBox->setRange(1, 100);
    nBox->setValue(R->nCascades());
    connect(nBox, QOverload<int>::of(&QSpinBox::valueChanged), R, &CascadeRecorder::setNCascades);
    form->addRow(tr("Cascades"), nBox);

    QDoubleSpinBox *spdBox = new QDoubleSpinBox;
    spdBox->setRange(0.1, 10.0);
    spdBox->setSingleStep(0.1);
    spdBox->setValue(1.0);
    connect(spdBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), R,
            &CascadeRecorder::setPlaybackSpeed);
    form->addRow(tr("Speed [ns/s]"), spdBox);

    return w;
}

QWidget *TrackViewWidget::buildColorTab_()
{
    QWidget *w = new QWidget;
    QFormLayout *form = new QFormLayout(w);

    QComboBox *colorBox = new QComboBox;
    colorBox->addItems({ tr("Generation"), tr("Energy"), tr("Species") });
    connect(colorBox, QOverload<int>::of(&QComboBox::currentIndexChanged), view_,
            &Track3DViewport::setColorMode);
    form->addRow(tr("Mode"), colorBox);

    QCheckBox *logBox = new QCheckBox(tr("Log E"));
    logBox->setChecked(view_->energyLog());
    connect(logBox, &QCheckBox::toggled, view_, &Track3DViewport::setEnergyLog);
    form->addRow(logBox);

    QCheckBox *autoBox = new QCheckBox(tr("Auto E"));
    autoBox->setChecked(view_->energyAuto());
    form->addRow(autoBox);

    QDoubleSpinBox *escaleMin = new QDoubleSpinBox;
    escaleMin->setRange(0.001, 1e9);
    escaleMin->setDecimals(3);
    escaleMin->setValue(view_->energyMin());
    escaleMin->setEnabled(!view_->energyAuto());
    connect(escaleMin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), view_,
            &Track3DViewport::setEnergyUserMin);
    form->addRow(tr("E scale min [eV]"), escaleMin);

    QDoubleSpinBox *escaleMax = new QDoubleSpinBox;
    escaleMax->setRange(0.001, 1e9);
    escaleMax->setDecimals(3);
    escaleMax->setValue(view_->energyMax());
    escaleMax->setEnabled(!view_->energyAuto());
    connect(escaleMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged), view_,
            &Track3DViewport::setEnergyUserMax);
    form->addRow(tr("E scale max [eV]"), escaleMax);

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

QWidget *TrackViewWidget::buildLimitsTab_()
{
    CascadeRecorder *R = view_->cascadeRecorder();

    QWidget *w = new QWidget;
    QFormLayout *form = new QFormLayout(w);

    QDoubleSpinBox *eThrBox = new QDoubleSpinBox;
    eThrBox->setRange(0.001, 1e9);
    eThrBox->setDecimals(3);
    eThrBox->setValue(view_->energyMin());
    connect(eThrBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), view_,
            &Track3DViewport::setEnergyThreshold);
    form->addRow(tr("E threshold [eV]"), eThrBox);

    QSpinBox *genBox = new QSpinBox;
    genBox->setRange(-1, 20);
    genBox->setValue(-1);
    genBox->setSpecialValueText(tr("all"));
    connect(genBox, QOverload<int>::of(&QSpinBox::valueChanged), R, &CascadeRecorder::setGenCutoff);
    form->addRow(tr("Max gen"), genBox);

    QSpinBox *memBox = new QSpinBox;
    memBox->setRange(0, 2000);
    memBox->setSpecialValueText(tr("off"));
    connect(memBox, QOverload<int>::of(&QSpinBox::valueChanged), R,
            &CascadeRecorder::setMemoryCapMB);
    form->addRow(tr("Mem [MB]"), memBox);

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
