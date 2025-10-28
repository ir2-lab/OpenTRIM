#include "mainui.h"

#include "optionsmodel.h"
#include "simulationoptionsview.h"
#include "welcomeview.h"
#include "mcdriverobj.h"
#include "simcontrolwidget.h"
#include "resultsview.h"
#include "tabularview.h"

#include <QVBoxLayout>

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
    QStringList icons{ "grid-outline.png", "settings-outline.png", "list-outline.png",
                       "bar-chart-outline.png" };

    QStringList titles{ "Welcome", "Config", "Summary", "Data" };
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

    setWindowTitle(tr(PROJECT_NAME));
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
    title += PROJECT_NAME;
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
        int ret = QMessageBox::warning(this, QString("Close %1").arg(PROJECT_NAME), msg,
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
