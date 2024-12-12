#include "ionsui.h"

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

IonsUI::IonsUI(QWidget *parent)
    : QWidget(parent)
{
    /* runner thread */
    driverObj_ = new McDriverObj;
    driverObj_->moveToThread(&runnerThread);
    connect(&runnerThread, &QThread::finished, driverObj_, &QObject::deleteLater);
    /* ToDo : connect slots for start, stop etc */

    runnerThread.start();

    optionsModel = new OptionsModel(this);

    // Load our style sheet style
    QFile styleFile(":/styles/default.qss");
    styleFile.open( QFile::ReadOnly );
    QString style( styleFile.readAll() );

    /* Create the sidebar */
    QWidget * sidebar = new QWidget(this);
    QVBoxLayout * sidebarLayout = new QVBoxLayout();

    pageButtonGrp = new QButtonGroup(this);
    QString iconFolder = ":/icons/assets/unknown/";
    QStringList icons{
        "small-circles-forming-a-circle.svg",
        "settings.svg",
        "play.svg",
        "presentation.svg"
    };
    QStringList titles{
        "Welcome",
        "Config",
        "Run",
        "Results"
    };
    for(int i=0; i<titles.count(); ++i) {
        pageButtonGrp->addButton(
            createSidebarButton(iconFolder + icons.at(i),
                                titles.at(i)),i);
        sidebarLayout->addWidget(pageButtonGrp->button(i));
    }
    sidebarLayout->addSpacerItem(new QSpacerItem(0,0,QSizePolicy::Minimum, QSizePolicy::MinimumExpanding));
    sidebarLayout->setSpacing(0);
    sidebarLayout->setMargin(0);
    /* Add the sidebar layout to the sidebar widget container */
    sidebar->setLayout(sidebarLayout);
    sidebar->setObjectName("sidebar");
    sidebar->setMinimumHeight(sidebarLayout->count() * 76);
    sidebar->setStyleSheet(style);

    /* Create the stacked widget + statusbar*/
    _stackedWidget = new QStackedWidget;

//    statusBar = new QStatusBar;
//    statusLabel = new QLabel;
//    QRect rect = fontMetrics().boundingRect("RunningOOO");
//    statusLabel->setMinimumWidth(rect.width());
//    statusLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
//    progressBar = new QProgressBar;
//    progressBar->setFormat("%p%");
//    progressBar->setMinimum(0);
//    progressBar->setMaximum(1000);
//    statusBar->addWidget(statusLabel,1);
//    statusBar->addWidget(progressBar,10);

    ctrlWidget = new SimControlWidget(driverObj_);

    /* Create the layout */
    QVBoxLayout * vbox = new QVBoxLayout;
    vbox->addWidget(_stackedWidget);
    vbox->addWidget(ctrlWidget);

    QHBoxLayout* layout = new QHBoxLayout;
    layout->addWidget(sidebar);
    layout->addLayout(vbox);
    setLayout(layout);
    layout->setSpacing(0);
    layout->setContentsMargins(0,0,0,0);

    setWindowTitle(tr("ions-gui"));
    QPoint x0 = geometry().center();
    QScreen* scr = QGuiApplication::screenAt(x0);
    resize(1024, 768);
    move(scr->geometry().center()-geometry().center());
    //setGeometry(0,0, 1024, 768);

    /* Create pages */
    welcomeView = new WelcomeView(this);
    push(tr("Welcome"),welcomeView);

    optionsView = new SimulationOptionsView(this);
    push(tr("Configuration"),optionsView);

    runView = new RunView(this);
    push(tr("Run"),runView);

    resultsView = new ResultsView(this);
    push(tr("Results"),resultsView);

    optionsView->revert();

    pageButtonGrp->button(0)->setChecked(true);
    _stackedWidget->setCurrentIndex(0);

    connect(pageButtonGrp, &QButtonGroup::idClicked,
            this, &IonsUI::changePage);
    connect(driverObj_, &McDriverObj::fileNameChanged,
            this, &IonsUI::updateWindowTitle);
    connect(driverObj_, &McDriverObj::modificationChanged,
            this, &IonsUI::updateWindowTitle);

    driverObj_->loadJsonTemplate();

}

IonsUI::~IonsUI()
{
    if (driverObj_->status() == McDriverObj::mcRunning)
        driverObj_->start(false);
    runnerThread.quit();
    runnerThread.wait();
}

void IonsUI::changePage(int idx)
{
    _stackedWidget->setCurrentIndex(idx);
}

void IonsUI::updateWindowTitle()
{
    QString title(driverObj_->fileName());
    if (driverObj_->isModified()) title += '*';
    title += " - ions-ui";
    setWindowTitle(title);
}

void IonsUI::closeEvent(QCloseEvent *event)
{
    McDriverObj::DriverStatus st = driverObj_->status();
    if (st == McDriverObj::mcReset) {
        event->accept();
        return;
    }
    QString msg = st == McDriverObj::mcRunning ?
        "Stop the running simulation, discard data & quit program?" :
                      "Discard simulation data & quit program?";
    int ret = QMessageBox::warning(this,
                                   QString("Close %1").arg(PROJECT_NAME),msg,
                                   QMessageBox::Ok | QMessageBox::Cancel);
    if (ret == QMessageBox::Ok) {
        event->accept();
    } else {
        event->ignore();
    }
}

void IonsUI::push(const QString &title, QWidget *page)
{
    QWidget* w = new QWidget;
    QVBoxLayout* vbox = new QVBoxLayout;
    QLabel* lbl = new QLabel(title);
    lbl->setStyleSheet("font-size : 20pt; font-weight : bold;");
    vbox->addWidget(lbl);
    vbox->addWidget(page);
    w->setLayout(vbox);
    _stackedWidget->addWidget(w);
}

void IonsUI::pop()
{
    QWidget * currentWidget = _stackedWidget->currentWidget();
    _stackedWidget->removeWidget(currentWidget);

    // delete currentWidget; currentWidget = nullptr;
}

QToolButton * IonsUI::createSidebarButton(const QString& iconPath, const QString& title)
{
    QIcon icon(iconPath);
    QToolButton * btn = new QToolButton;
    btn->setIcon(icon);
    btn->setIconSize(QSize(42, 42));
    btn->setText(title);
    btn->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    btn->setFixedSize(76, 76);
    btn->setObjectName(title);
    btn->setCheckable(true);
    return btn;
}

IonsUI::PageId IonsUI::currentPage() const
{
    return PageId(pageButtonGrp->checkedId());
}

void IonsUI::setCurrentPage(PageId id)
{
    pageButtonGrp->button((int)id)->click();
}






