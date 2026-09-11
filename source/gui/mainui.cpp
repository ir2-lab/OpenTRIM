#include "mainui.h"

#include "optionsmodel.h"
#include "optionsview.h"
#include "optionwidgetmapper.h"
#include "welcomeview.h"
#include "mcdriverobj.h"
#include "simcontrolwidget.h"
#include "resultsview.h"
#include "summaryview.h"
#include "track3dviewport.h"
#include "trackviewwidget.h"
#include "dialogs.h"

#include <QVBoxLayout>
#include <QStatusBar>
#include <QToolButton>
#include <QStackedWidget>
#include <QTextBrowser>
#include <QLabel>
#include <QFont>
#include <QProgressBar>
#include <QCloseEvent>
#include <QGuiApplication>
#include <QScreen>
#include <QFile>
#include <QButtonGroup>
#include <QSplitter>
#include <qpainter.h>
#include <qsvgrenderer.h>

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
    /* Add the sidebar layout to the sidebar widget container */
    sidebar->setLayout(sidebarLayout);
    sidebar->setObjectName("sidebar");
    sidebar->setMinimumHeight(sidebarLayout->count() * SIDEBAR_H);
    sidebar->setStyleSheet(style);
    sidebarLayout->setSpacing(0);
    sidebarLayout->setMargin(0);
    sidebar->ensurePolished();

    pageButtonGrp = new QButtonGroup(this);

    QString iconFolder = ":/assets/ionicons/";
    QStringList icons{ "grid-outline.svg", "settings-outline.svg", "cube-outline.svg",
                       "list-outline.svg", "bar-chart-outline.svg" };

    QStringList titles{ "Welcome", "Config", "3D Vis", "Summary", "Results" };
    for (int i = 0; i < titles.count(); ++i) {
        pageButtonGrp->addButton(createSidebarButton(iconFolder + icons.at(i), titles.at(i)), i);
        sidebarLayout->addWidget(pageButtonGrp->button(i));
    }
    sidebarLayout->addSpacerItem(
            new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding));

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
    welcomeView = new WelcomeView(this, tr("Welcome"));
    addPage(welcomeView);

    optionsView = new OptionsView(this, tr("Configuration"));
    addPage(optionsView);

    trackView = new TrackViewWidget(this, tr("3D Visualization"));
    addPage(trackView);

    tblView = new SummaryView(this, tr("Results Summary"));
    addPage(tblView);

    resultsView = new ResultsView(this, tr("Simulation Results"));
    addPage(resultsView);

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
    // resize(1200, 900);
    // resize(1024, 768);
    //  resize(600, 600);
    resize(minimumSizeHint());

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
        driver_ok = Dialogs::confirm(this, tr("Quit Program"), msg);
    }

    if (driver_ok) {

        if (quickStartWidget)
            quickStartWidget->close();

        event->accept();

    } else {

        event->ignore();
    }
}

void MainUI::setHeadingFont(QWidget *w, int headingLevel, bool bold)
{
    static const qreal factors[] = { 1.9, 1.6, 1.3 };
    static const int nFactors = sizeof(factors) / sizeof(factors[0]);
    int idx = headingLevel - 1;
    qreal factor = (idx >= 0 && idx < nFactors) ? factors[idx] : 1.0;

    QFont f = w->font();
    f.setPointSizeF(f.pointSizeF() * factor);
    f.setBold(bold);
    w->setFont(f);
}

void MainUI::push(const QString &title, QWidget *page)
{
    QWidget *w = new QWidget;
    QVBoxLayout *vbox = new QVBoxLayout;
    QLabel *lbl = new QLabel(title);
    MainUI::setHeadingFont(lbl, 1);
    vbox->addWidget(lbl);
    vbox->addSpacing(V_SPACING);
    vbox->addWidget(page);
    QSizePolicy szPolicy = page->sizePolicy();
    w->setLayout(vbox);
    _stackedWidget->addWidget(w);

    qDebug() << title << " - min size hint: " << w->minimumSizeHint() << " - " << minimumSizeHint();
}

void MainUI::addPage(Page *page)
{
    _stackedWidget->addWidget(page);
}

void MainUI::pop()
{
    QWidget *currentWidget = _stackedWidget->currentWidget();
    _stackedWidget->removeWidget(currentWidget);

    // delete currentWidget; currentWidget = nullptr;
}

QToolButton *MainUI::createSidebarButton(const QString &iconPath, const QString &title)
{
    const int iconSize = 32;

    QFile f(iconPath);
    f.open(QIODevice::ReadOnly);
    QString data = QString::fromUtf8(f.readAll());
    // The color here is hardcoded
    // The value comes from :/styles/default.qss, QToolButton:color
    data.replace(QLatin1String("currentColor"), QLatin1String("#ededed"));

    QSvgRenderer renderer(data.toUtf8());
    const qreal dpr = qApp->devicePixelRatio();
    QPixmap pm(QSize(iconSize, iconSize) * dpr);
    pm.setDevicePixelRatio(dpr);
    pm.fill(Qt::transparent);
    QPainter p(&pm);
    renderer.render(&p);
    p.end();

    QIcon icon(pm);

    // QIcon icon(iconPath);
    QToolButton *btn = new QToolButton;
    btn->setIcon(icon);
    btn->setIconSize(QSize(iconSize, iconSize));
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
        MainUI::setHeadingFont(label, 3);
        vbox->addWidget(label);
        QTextBrowser *quickstart = new QTextBrowser;
        quickstart->setSource(QUrl("qrc:./md/quick_start.md"));
        quickstart->setOpenExternalLinks(true);
        vbox->addWidget(quickstart);
        quickStartWidget->resize(800, 800);
        quickStartWidget->show();
    }
}

Page::Page(MainUI *ui, const QString &title, bool hasSimTitle, QWidget *parent) : QWidget(parent)
{
    // create widgets
    lblTitle = new QLabel(title);
    MainUI::setHeadingFont(lblTitle, 1);
    if (hasSimTitle) {
        lblSimTitle = new QLabel("Simulation Title");
        // MainUI::setHeadingFont(lblSimTitle, 3, true);
        lblSimTitle->setStyleSheet("color: gray;");
        edtSimTitle = new QLineEdit;
        // MainUI::setHeadingFont(edtSimTitle, 3, false);
        mapper = new OptionWidgetMapper(ui->optionsModel, this);
        QModelIndex idxOut = ui->optionsModel->index("Output", 0);
        QModelIndex idxTitle = ui->optionsModel->index("title", 0, idxOut);
        OptionsItem *item = ui->optionsModel->getItem(idxTitle);
        item->prepareWidget(edtSimTitle);
        lblSimTitle->setToolTip(edtSimTitle->toolTip());
        lblSimTitle->setWhatsThis(edtSimTitle->whatsThis());
        mapper->addMapping(edtSimTitle, idxTitle, true, item->editorSignal());
        mapper->addMapping(lblSimTitle, idxTitle, false);

        edtSimTitle->setReadOnly(true);

        connect(ui->driverObj(), &McDriverObj::configChanged, this, &Page::revert);
    }
    content = new QWidget;

    // layout
    QVBoxLayout *vbox = new QVBoxLayout(this);
    if (hasSimTitle) {
        QGridLayout *grid = new QGridLayout;
        grid->setContentsMargins(0, 0, 0, 0);
        grid->addWidget(lblTitle, 0, 0);
        QHBoxLayout *hbox = new QHBoxLayout;
        hbox->addWidget(lblSimTitle);
        hbox->addWidget(edtSimTitle);
        grid->addLayout(hbox, 0, 1);
        grid->setColumnStretch(0, 1);
        grid->setColumnStretch(1, 2);
        vbox->addLayout(grid);
    } else {
        vbox->addWidget(lblTitle);
    }
    vbox->addWidget(content);
}

void Page::setContent(QWidget *w)
{
    QVBoxLayout *vbox = (QVBoxLayout *)layout();
    QLayoutItem *i = vbox->replaceWidget(content, w);
    assert(i);
    QWidget *w1 = content;
    content = w;
    w1->deleteLater();
}

void Page::revert()
{
    mapper->revert();
}
