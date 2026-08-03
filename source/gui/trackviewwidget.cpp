#include "trackviewwidget.h"

#include "track3dviewport.h"

#include <fstream>

#include <nlohmann/json.hpp>

#include <QCheckBox>
#include <QComboBox>
#include <QDebug>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QImage>
#include <QLineEdit>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QTabWidget>
#include <QVBoxLayout>

static float jsonNumber(const nlohmann::json &j, const char *key, float fallback)
{
    auto it = j.find(key);
    return (it != j.end() && it->is_number()) ? it->get<float>() : fallback;
}

TrackViewWidget::TrackViewWidget(McDriverObj *driver, QWidget *parent) : QWidget(parent)
{
    view_ = new Track3DViewport(driver, this);
    TrackColorBar *colorBar = new TrackColorBar(view_);

    QTabWidget *tabs = new QTabWidget;
    tabs->addTab(buildViewTab_(), tr("View"));
    tabs->addTab(buildPlaybackTab_(), tr("Playback"));
    tabs->addTab(buildColorTab_(), tr("Color"));
    tabs->addTab(buildLimitsTab_(), tr("Limits"));
    tabs->setMaximumWidth(260);

    QHBoxLayout *center = new QHBoxLayout;
    center->setContentsMargins(0, 0, 0, 0);
    center->addWidget(view_, 1);
    center->addWidget(colorBar);
    center->addWidget(tabs);

    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->addLayout(center, 1);
    root->addWidget(buildPrimaryBar_());
}

QWidget *TrackViewWidget::buildPrimaryBar_()
{
    CascadeRecorder *R = view_->cascadeRecorder();

    QWidget *bar = new QWidget;
    QHBoxLayout *row = new QHBoxLayout(bar);
    row->setContentsMargins(0, 0, 0, 0);

    QPushButton *capBt = new QPushButton(tr("Capture on"));
    capBt->setCheckable(true);
    connect(capBt, &QPushButton::toggled, R, &CascadeRecorder::capture);
    connect(view_, &Track3DViewport::captureChanged, capBt, [capBt](bool on) {
        QSignalBlocker block(capBt); // don't let setChecked re-emit toggled()
        capBt->setChecked(on); // reflect auto-stop
        capBt->setText(on ? tr("Capture off") : tr("Capture on"));
    });
    row->addWidget(capBt);

    QPushButton *clearBt = new QPushButton(tr("Clear"));
    connect(clearBt, &QPushButton::clicked, R, &CascadeRecorder::clear);
    row->addWidget(clearBt);

    QPushButton *shotBt = new QPushButton(tr("Screenshot"));
    connect(shotBt, &QPushButton::clicked, this, &TrackViewWidget::saveScreenshot_);
    row->addWidget(shotBt);

    QLineEdit *status = new QLineEdit;
    status->setReadOnly(true);
    connect(R, &CascadeRecorder::statusUpdate, status, &QLineEdit::setText);
    row->addWidget(status, 1);

    return bar;
}

QWidget *TrackViewWidget::buildViewTab_()
{
    QWidget *w = new QWidget;
    QVBoxLayout *col = new QVBoxLayout(w);

    struct
    {
        const char *label;
        int view;
        bool home; // Home also fits the box
    } btns[] = { { "Home", Track3DViewport::Iso, true },
                 { "Front", Track3DViewport::Front, false },
                 { "Top", Track3DViewport::Top, false },
                 { "Left", Track3DViewport::Left, false },
                 { "Iso", Track3DViewport::Iso, false } };

    for (const auto &b : btns) {
        QPushButton *bt = new QPushButton(tr(b.label));
        const int v = b.view;
        const bool home = b.home;
        connect(bt, &QPushButton::clicked, view_, [this, v, home]() {
            if (home)
                view_->homeView();
            else
                view_->setPresetView(v);
        });
        col->addWidget(bt);
    }

    QPushButton *saveBt = new QPushButton(tr("Save camera..."));
    connect(saveBt, &QPushButton::clicked, this, &TrackViewWidget::saveCamera_);
    col->addWidget(saveBt);

    QPushButton *loadBt = new QPushButton(tr("Load camera..."));
    connect(loadBt, &QPushButton::clicked, this, &TrackViewWidget::loadCamera_);
    col->addWidget(loadBt);

    col->addStretch();
    return w;
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
        c.center = QVector3D((*ctr)[0].get<float>(), (*ctr)[1].get<float>(),
                             (*ctr)[2].get<float>());
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
