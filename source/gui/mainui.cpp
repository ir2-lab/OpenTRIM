#include "mainui.h"

#include "optionsmodel.h"
#include "simulationoptionsview.h"
#include "welcomeview.h"
#include "mcdriverobj.h"
#include "simcontrolwidget.h"
#include "resultsview.h"
#include "tabularview.h"
#include "track3dviewport.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLineEdit>

#include <QJsonDocument>
#include <QStatusBar>
#include <QToolButton>
#include <QStackedWidget>
#include <QTextBrowser>
#include <QLabel>
#include <QProgressBar>
#include <QMessageBox>
#include <QCloseEvent>
#include <QGuiApplication>
#include <QScreen>
#include <QFile>
#include <QButtonGroup>

#define SIDEBAR_W 70
#define SIDEBAR_H 70

MainUI::MainUI(QWidget *parent) : QWidget(parent), quickStartWidget(nullptr)
{
    /* create runner thread */
    driverObj_ = new McDriverObj;
    driverObj_->moveToThread(&runnerThread);
    connect(&runnerThread, &QThread::finished, driverObj_, &QObject::deleteLater);
    runnerThread.start();

    optionsModel = new OptionsModel(this);

    // Load our style sheet style
    QFile styleFile(":/styles/default.qss");
    styleFile.open(QFile::ReadOnly);
    QString style(styleFile.readAll());

    /* Create the sidebar */
    QWidget *sidebar = new QWidget(this);
    QVBoxLayout *sidebarLayout = new QVBoxLayout();

    pageButtonGrp = new QButtonGroup(this);

    QString iconFolder = ":/assets/ionicons/";
    QStringList icons{ "grid-outline.png", "settings-outline.png", "cube-outline.svg",
                       "list-outline.png", "bar-chart-outline.png" };

    QStringList titles{ "Welcome", "Config", "3D Vis", "Summary", "Data" };
    for (int i = 0; i < titles.count(); ++i) {
        pageButtonGrp->addButton(createSidebarButton(iconFolder + icons.at(i), titles.at(i)), i);
        sidebarLayout->addWidget(pageButtonGrp->button(i));
    }
    sidebarLayout->addSpacerItem(
            new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding));
    sidebarLayout->setSpacing(0);
    sidebarLayout->setMargin(0);
    /* Add the sidebar layout to the sidebar widget container */
    sidebar->setLayout(sidebarLayout);
    sidebar->setObjectName("sidebar");
    sidebar->setMinimumHeight(sidebarLayout->count() * SIDEBAR_H);
    sidebar->setStyleSheet(style);

    /* Create the stacked widget + statusbar*/
    _stackedWidget = new QStackedWidget;

    ctrlWidget = new SimControlWidget(this);

    /* Create the layout */
    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addWidget(_stackedWidget);
    vbox->addWidget(ctrlWidget);

    QHBoxLayout *layout = new QHBoxLayout;
    layout->addWidget(sidebar);
    layout->addLayout(vbox);
    setLayout(layout);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    /* Create pages */
    welcomeView = new WelcomeView(this);
    push(tr("Welcome"), welcomeView);

    optionsView = new SimulationOptionsView(this);
    push(tr("Configuration"), optionsView);

    push(tr("3D Visualization"), createTrackViewPage());

    // runView = new RunView(this);
    // push(tr("Run"), runView);
    tblView = new TabularView(this);
    push(tr("Summary Tables"), tblView);

    resultsView = new ResultsView(this);
    push(tr("Simulation Data"), resultsView);

    optionsView->revert();

    pageButtonGrp->button(0)->setChecked(true);
    _stackedWidget->setCurrentIndex(0);

    connect(pageButtonGrp, &QButtonGroup::idClicked, this, &MainUI::changePage);
    connect(driverObj_, &McDriverObj::fileNameChanged, this, &MainUI::updateWindowTitle);
    connect(driverObj_, &McDriverObj::modificationChanged, this, &MainUI::updateWindowTitle);

    driverObj_->loadJsonTemplate();

    setWindowTitle(mcdriver::version_info().project_name);
    QPoint x0 = geometry().center();
    QScreen *scr = QGuiApplication::screenAt(x0);
    // resize(1024, 768);
    resize(600, 600);

    show();

    move(scr->geometry().center() - geometry().center());
}

MainUI::~MainUI()
{
    if (driverObj_->status() == McDriverObj::mcRunning)
        driverObj_->start(false);
    runnerThread.quit();
    runnerThread.wait();

    if (quickStartWidget)
        delete quickStartWidget;
}

void MainUI::changePage(int idx)
{
    _stackedWidget->setCurrentIndex(idx);
}

void MainUI::updateWindowTitle()
{
    QString title(driverObj_->fileName());
    if (driverObj_->isModified())
        title += '*';
    title += " - ";
    title += mcdriver::version_info().project_name;
    setWindowTitle(title);
}

void MainUI::closeEvent(QCloseEvent *event)
{
    bool driver_ok;

    McDriverObj::DriverStatus st = driverObj_->status();
    if (st == McDriverObj::mcReset) {
        driver_ok = true;
    } else {
        QString msg = st == McDriverObj::mcRunning
                ? "Stop the running simulation, discard data & quit program?"
                : "Discard simulation data & quit program?";
        int ret = QMessageBox::warning(
                this, QString("Close %1").arg(mcdriver::version_info().project_name), msg,
                QMessageBox::Ok | QMessageBox::Cancel);
        driver_ok = (ret == QMessageBox::Ok);
    }

    if (driver_ok) {

        if (quickStartWidget)
            quickStartWidget->close();

        event->accept();

    } else {

        event->ignore();
    }
}

void MainUI::push(const QString &title, QWidget *page)
{
    QWidget *w = new QWidget;
    QVBoxLayout *vbox = new QVBoxLayout;
    QLabel *lbl = new QLabel(title);
    lbl->setStyleSheet("font-size : 20pt; font-weight : bold;");
    vbox->addWidget(lbl);
    vbox->addSpacing(V_SPACING);
    vbox->addWidget(page);
    w->setLayout(vbox);
    _stackedWidget->addWidget(w);
}

void MainUI::pop()
{
    QWidget *currentWidget = _stackedWidget->currentWidget();
    _stackedWidget->removeWidget(currentWidget);

    // delete currentWidget; currentWidget = nullptr;
}

QWidget *MainUI::createTrackViewPage()
{
    trackView = new Track3DViewport(driverObj_, this);

    // toolbar under the viewport
    QWidget *bar = new QWidget;
    QHBoxLayout *hbox = new QHBoxLayout(bar);
    hbox->setContentsMargins(0, 0, 0, 0);

    CascadeRecorder *R = trackView->cascadeRecorder();

    QPushButton *capBt = new QPushButton(tr("Capture on"));
    capBt->setCheckable(true);
    connect(capBt, &QPushButton::toggled, R, &CascadeRecorder::capture);
    connect(trackView, &Track3DViewport::captureChanged, capBt, [capBt](bool on) {
        QSignalBlocker block(capBt); // don't let setChecked re-emit toggled()
        capBt->setChecked(on); // reflect auto-stop
        capBt->setText(on ? tr("Capture off") : tr("Capture on"));
    });
    hbox->addWidget(capBt);

    QPushButton *clearBt = new QPushButton(tr("Clear"));
    connect(clearBt, &QPushButton::clicked, R, &CascadeRecorder::clear);
    hbox->addWidget(clearBt);

    QCheckBox *ringBox = new QCheckBox(tr("Ring"));
    ringBox->setChecked(true);
    connect(ringBox, &QCheckBox::toggled, R, &CascadeRecorder::setRingMode);
    hbox->addWidget(ringBox);

    hbox->addWidget(new QLabel(tr("Cascades")));
    QSpinBox *nBox = new QSpinBox;
    nBox->setRange(1, 100);
    nBox->setValue(R->nCascades());
    connect(nBox, QOverload<int>::of(&QSpinBox::valueChanged), R, &CascadeRecorder::setNCascades);
    hbox->addWidget(nBox);

    hbox->addWidget(new QLabel(tr("Speed [ns/s]")));
    QDoubleSpinBox *spdBox = new QDoubleSpinBox;
    spdBox->setRange(0.1, 10.0);
    spdBox->setSingleStep(0.1);
    spdBox->setValue(1.0);
    connect(spdBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), R,
            &CascadeRecorder::setPlaybackSpeed);
    hbox->addWidget(spdBox);

    QLineEdit *edtStatus = new QLineEdit;
    edtStatus->setReadOnly(true);
    connect(R, &CascadeRecorder::statusUpdate, edtStatus, &QLineEdit::setText);
    hbox->addWidget(edtStatus);

    hbox->addStretch();

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
        connect(bt, &QPushButton::clicked, trackView, [this, v, home]() {
            if (home)
                trackView->homeView();
            else
                trackView->setPresetView(v);
        });
        hbox->addWidget(bt);
    }

    // color & limits toolbar
    QWidget *bar2 = new QWidget;
    QHBoxLayout *hbox2 = new QHBoxLayout(bar2);
    hbox2->setContentsMargins(0, 0, 0, 0);

    hbox2->addWidget(new QLabel(tr("Color")));
    QComboBox *colorBox = new QComboBox;
    colorBox->addItems({ tr("Generation"), tr("Energy"), tr("Species") });
    connect(colorBox, QOverload<int>::of(&QComboBox::currentIndexChanged), trackView,
            &Track3DViewport::setColorMode);
    hbox2->addWidget(colorBox);

    QCheckBox *logBox = new QCheckBox(tr("Log E"));
    logBox->setChecked(trackView->energyLog());
    connect(logBox, &QCheckBox::toggled, trackView, &Track3DViewport::setEnergyLog);
    hbox2->addWidget(logBox);

    QCheckBox *autoBox = new QCheckBox(tr("Auto E"));
    autoBox->setChecked(trackView->energyAuto());
    hbox2->addWidget(autoBox);

    hbox2->addWidget(new QLabel(tr("E scale [eV]")));
    QDoubleSpinBox *escaleMin = new QDoubleSpinBox;
    escaleMin->setRange(0.001, 1e9);
    escaleMin->setDecimals(3);
    escaleMin->setValue(trackView->energyMin());
    escaleMin->setEnabled(!trackView->energyAuto());
    connect(escaleMin, QOverload<double>::of(&QDoubleSpinBox::valueChanged), trackView,
            &Track3DViewport::setEnergyUserMin);
    hbox2->addWidget(escaleMin);

    QDoubleSpinBox *escaleMax = new QDoubleSpinBox;
    escaleMax->setRange(0.001, 1e9);
    escaleMax->setDecimals(3);
    escaleMax->setValue(trackView->energyMax());
    escaleMax->setEnabled(!trackView->energyAuto());
    connect(escaleMax, QOverload<double>::of(&QDoubleSpinBox::valueChanged), trackView,
            &Track3DViewport::setEnergyUserMax);
    hbox2->addWidget(escaleMax);

    connect(autoBox, &QCheckBox::toggled, trackView, &Track3DViewport::setEnergyAuto);
    connect(autoBox, &QCheckBox::toggled, escaleMin, &QWidget::setDisabled);
    connect(autoBox, &QCheckBox::toggled, escaleMax, &QWidget::setDisabled);
    connect(autoBox, &QCheckBox::toggled, trackView, [this, escaleMin, escaleMax](bool on) {
        if (!on) {
            QSignalBlocker b1(escaleMin), b2(escaleMax);
            escaleMin->setValue(trackView->energyMin());
            escaleMax->setValue(trackView->energyMax());
        }
    });

    hbox2->addWidget(new QLabel(tr("E min [eV]")));
    QDoubleSpinBox *eThrBox = new QDoubleSpinBox;
    eThrBox->setRange(0.001, 1e9);
    eThrBox->setDecimals(3);
    eThrBox->setValue(trackView->energyMin());
    connect(eThrBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), trackView,
            &Track3DViewport::setEnergyThreshold);
    hbox2->addWidget(eThrBox);

    hbox2->addWidget(new QLabel(tr("Max gen")));
    QSpinBox *genBox = new QSpinBox;
    genBox->setRange(-1, 20);
    genBox->setValue(-1);
    genBox->setSpecialValueText(tr("all"));
    connect(genBox, QOverload<int>::of(&QSpinBox::valueChanged), R,
            &CascadeRecorder::setGenCutoff);
    hbox2->addWidget(genBox);

    hbox2->addWidget(new QLabel(tr("Mem [MB]")));
    QSpinBox *memBox = new QSpinBox;
    memBox->setRange(0, 2000);
    memBox->setSpecialValueText(tr("off"));
    connect(memBox, QOverload<int>::of(&QSpinBox::valueChanged), R,
            &CascadeRecorder::setMemoryCapMB);
    hbox2->addWidget(memBox);

    hbox2->addStretch();

    TrackColorBar *colorBar = new TrackColorBar(trackView);

    QWidget *page = new QWidget;
    QVBoxLayout *vbox = new QVBoxLayout(page);
    vbox->setContentsMargins(0, 0, 0, 0);
    QHBoxLayout *center = new QHBoxLayout;
    center->setContentsMargins(0, 0, 0, 0);
    center->addWidget(trackView, 1);
    center->addWidget(colorBar);
    vbox->addLayout(center, 1);
    vbox->addWidget(bar);
    vbox->addWidget(bar2);
    return page;
}

QToolButton *MainUI::createSidebarButton(const QString &iconPath, const QString &title)
{
    QIcon icon(iconPath);
    QToolButton *btn = new QToolButton;
    btn->setIcon(icon);
    btn->setIconSize(QSize(32, 32));
    btn->setText(title);
    btn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btn->setFixedSize(SIDEBAR_W, SIDEBAR_H);
    btn->setObjectName(title);
    btn->setCheckable(true);
    return btn;
}

MainUI::PageId MainUI::currentPage() const
{
    return PageId(pageButtonGrp->checkedId());
}

void MainUI::setCurrentPage(PageId id)
{
    pageButtonGrp->button((int)id)->click();
}

void MainUI::showQuickStartWidget()
{
    if (quickStartWidget) {
        quickStartWidget->show();
        quickStartWidget->raise();
    } else {
        quickStartWidget = new QWidget;
        QVBoxLayout *vbox = new QVBoxLayout;
        quickStartWidget->setLayout(vbox);
        QLabel *label = new QLabel("OpenTRIM Quick Start Guide");
        label->setStyleSheet("font-size : 14pt; font-weight : bold;");
        vbox->addWidget(label);
        QTextBrowser *quickstart = new QTextBrowser;
        quickstart->setSource(QUrl("qrc:./md/quick_start.md"));
        quickstart->setOpenExternalLinks(true);
        vbox->addWidget(quickstart);
        quickStartWidget->resize(800, 800);
        quickStartWidget->show();
    }
}
