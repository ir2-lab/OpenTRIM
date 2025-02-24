#include "mainui.h"

#include "optionsmodel.h"
#include "simulationoptionsview.h"
#include "welcomeview.h"
#include "runview.h"
#include "mcdriverobj.h"
#include "simcontrolwidget.h"
#include "resultsview.h"

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

#include <sstream>

MainUI::MainUI(QWidget *parent) : QWidget(parent)
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

    QStringList titles{ "Welcome", "Config", "Results", "Graphs" };
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
    sidebar->setMinimumHeight(sidebarLayout->count() * 76);
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

    setWindowTitle(tr(PROJECT_NAME));
    QPoint x0 = geometry().center();
    QScreen *scr = QGuiApplication::screenAt(x0);
    resize(1024, 768);
    move(scr->geometry().center() - geometry().center());
    // setGeometry(0,0, 1024, 768);

    /* Create pages */
    welcomeView = new WelcomeView(this);
    push(tr("Welcome"), welcomeView);

    optionsView = new SimulationOptionsView(this);
    push(tr("Configuration"), optionsView);

    runView = new RunView(this);
    push(tr("Run"), runView);

    resultsView = new ResultsView(this);
    push(tr("Results"), resultsView);

    optionsView->revert();

    pageButtonGrp->button(0)->setChecked(true);
    _stackedWidget->setCurrentIndex(0);

    connect(pageButtonGrp, &QButtonGroup::idClicked, this, &MainUI::changePage);
    connect(driverObj_, &McDriverObj::fileNameChanged, this, &MainUI::updateWindowTitle);
    connect(driverObj_, &McDriverObj::modificationChanged, this, &MainUI::updateWindowTitle);

    driverObj_->loadJsonTemplate();
}

MainUI::~MainUI()
{
    if (driverObj_->status() == McDriverObj::mcRunning)
        driverObj_->start(false);
    runnerThread.quit();
    runnerThread.wait();
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
    McDriverObj::DriverStatus st = driverObj_->status();
    if (st == McDriverObj::mcReset) {
        event->accept();
        return;
    }
    QString msg = st == McDriverObj::mcRunning
            ? "Stop the running simulation, discard data & quit program?"
            : "Discard simulation data & quit program?";
    int ret = QMessageBox::warning(this, QString("Close %1").arg(PROJECT_NAME), msg,
                                   QMessageBox::Ok | QMessageBox::Cancel);
    if (ret == QMessageBox::Ok) {
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
    btn->setIconSize(QSize(42, 42));
    btn->setText(title);
    btn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btn->setFixedSize(76, 76);
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
