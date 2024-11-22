#include "welcomeview.h"

#include "ionsui.h"
#include "mcdriver.h"

#include <QPushButton>
#include <QToolButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QButtonGroup>
#include <QFontMetrics>
#include <QListWidget>
#include <QListWidgetItem>
#include <QStackedWidget>
#include <QLabel>
#include <QDir>
#include <QDialogButtonBox>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QJsonDocument>
#include <QMenu>

#include "jsedit/jsedit.h"

WelcomeView::WelcomeView(IonsUI* iui, QWidget *parent)
    : QWidget{parent}, ionsui(iui)
{
    QFontMetrics fm = this->fontMetrics();
    QRect rect = fm.boundingRect("OOOOpen SimulationOOO");
    int h = rect.height()*2;
    int w = rect.width();
    btNew =     createButton("New Simulation",w,h,0);
    btOpen =    createButton("Open Simulation",w,h,0);

    btSave = new QToolButton;
    btSave->setText("Save");
    btSave->setPopupMode(QToolButton::InstantPopup);
    btSave->setMinimumSize(QSize(w,h));
    {
        QMenu* toolMenu = new QMenu;
        QAction* action;
        actSaveJson = toolMenu->addAction("Save Config to JSON");
        actSaveJson->setEnabled(false);
        connect(actSaveJson, &QAction::triggered,
                this, &WelcomeView::onSaveJson);
        actSaveH5 = toolMenu->addAction("Save Config+Data to HDF5");
        actSaveH5->setEnabled(false);
        connect(actSaveH5, &QAction::triggered,
                this, &WelcomeView::onSaveH5);
        btSave->setMenu(toolMenu);
    }

    btSaveAs = new QToolButton;
    btSaveAs->setText("Save As ...");
    btSaveAs->setPopupMode(QToolButton::InstantPopup);
    btSaveAs->setMinimumSize(QSize(w,h));
    {
        QMenu* toolMenu = new QMenu;
        QAction* action;
        actSaveJsonAs = toolMenu->addAction("Save Config to JSON As ...");
        connect(actSaveJsonAs, &QAction::triggered,
                this, &WelcomeView::onSaveJsonAs);
        actSaveH5As = toolMenu->addAction("Save Config+Data to HDF5 As ...");
        connect(actSaveH5As, &QAction::triggered,
                this, &WelcomeView::onSaveH5As);
        btSaveAs->setMenu(toolMenu);
    }

    btRecent =  createButton("Recent",w,h,1);
    btExamples = createButton("Examples",w,h,1);
    btGettingStarted = createButton("Getting Started",w,h,1);
    btAbout =   createButton("About",w,h,1);
    btRecent->setChecked(true);

    QVBoxLayout* vbox = new QVBoxLayout;
    vbox->addWidget(btNew);
    vbox->addWidget(btOpen);
    vbox->addWidget(btSave);
    vbox->addWidget(btSaveAs);
    vbox->addStretch();
    vbox->addWidget(btRecent);
    vbox->addWidget(btExamples);
    vbox->addWidget(btGettingStarted);
    vbox->addWidget(btAbout);
    buttonGrp = new QButtonGroup(this);
    buttonGrp->addButton(btRecent,0);
    buttonGrp->addButton(btExamples,1);
    buttonGrp->addButton(btGettingStarted,2);
    buttonGrp->addButton(btAbout,3);

    QHBoxLayout* viewLayout = new QHBoxLayout;
    viewLayout->addLayout(vbox,0);
    //viewLayout->addWidget(new QWidget,2);
    viewLayout->setContentsMargins(0,0,0,0);
    setLayout(viewLayout);

    /* create right stacked pane */
    stackedWidget = new QStackedWidget;
    viewLayout->addWidget(stackedWidget,2);

    pushCenterWidget("Recent simulations", new QWidget);

    /* create example view */
    QWidget* examplePage = new QWidget;
    {
        exampleList = new QListWidget;
        foreach( const QString &ex, QDir(":/examples").entryList() )
        {
            new QListWidgetItem(
                QIcon(":/icons/assets/ionicons/document-text-outline.svg"),
                ex,
                exampleList);
        }
        jsonView = new JSEdit;
        jsonView->setReadOnly(true);

        QDialogButtonBox* buttonBox = new QDialogButtonBox(Qt::Horizontal);
        btOpenExample = new QPushButton(QIcon(":/icons/assets/ionicons/checkmark-done-outline.svg"),
                                                  "Open example");
        buttonBox->addButton(btOpenExample, QDialogButtonBox::AcceptRole);
        connect(btOpenExample, &QPushButton::clicked,
                this, &WelcomeView::onOpenExample);
        btOpenExample->setEnabled(false);

        QVBoxLayout* vbox = new QVBoxLayout;
        vbox->addWidget(exampleList);
        vbox->addSpacing(20);
        vbox->addWidget(jsonView);
        vbox->addWidget(buttonBox);
        vbox->setContentsMargins(0,0,0,0);
        examplePage->setLayout(vbox);
    }
    pushCenterWidget("Example simulations",examplePage);

    pushCenterWidget("Getting Started", new QWidget);

    /* create about view */
    {
        QPlainTextEdit* about = new QPlainTextEdit;
        about->setPlainText(
            QString("IONS simulation\nVersion: %1\nBuild time: %2\nCompiler: %3 v%4\nSystem: %5")
                .arg(IRADINAPP_VERSION)
                .arg(BUILD_TIME)
                .arg(COMPILER_ID).arg(COMPILER_VERSION)
                .arg(SYSTEM_ID)
            );
        pushCenterWidget("About", about);
    }




    /* signal/slot connections */
    connect(buttonGrp, &QButtonGroup::idClicked,
            this, &WelcomeView::changeCenterWidget);
    connect(exampleList, &QListWidget::currentItemChanged,
            this, &WelcomeView::exampleSelected);
    connect(exampleList, &QListWidget::itemDoubleClicked,
            this, &WelcomeView::onOpenExample);

    connect(btOpen, &QPushButton::clicked,
            this, &WelcomeView::onOpenJson);
    connect(btNew, &QPushButton::clicked,
            this, &WelcomeView::onNew);

    connect(ionsui->driverObj(),&McDriverObj::modificationChanged,
            actSaveJson,&QAction::setEnabled);
    connect(ionsui->driverObj(),&McDriverObj::modificationChanged,
            this,&WelcomeView::onDriverStatusChanged);
    connect(ionsui->driverObj(),&McDriverObj::statusChanged,
            this,&WelcomeView::onDriverStatusChanged);

    btAbout->setChecked(true);
    changeCenterWidget(3);
}

void WelcomeView::changeCenterWidget(int id)
{
    stackedWidget->setCurrentIndex(id);
}

void WelcomeView::onOpenExample()
{
    if (!userDiscardCurrentSim("Open Example")) return;

    QListWidgetItem* i = exampleList->currentItem();
    if (!i) return;
    QFile f(QString(":/examples/%1").arg(i->text()));
    if (f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        f.close();
        ionsui->driverObj()->loadJson(f.fileName());
        ionsui->setCurrentPage(IonsUI::idConfigPage);
    }
}

void WelcomeView::onDriverStatusChanged()
{
    McDriverObj* D = ionsui->driverObj();
    auto s = D->status();
    actSaveH5->setEnabled(D->isModified() && s==McDriverObj::mcIdle);
    actSaveH5As->setEnabled(s==McDriverObj::mcIdle);
}

void WelcomeView::onOpenJson()
{
    if (!userDiscardCurrentSim("Open JSON")) return;

    QString fileName = QFileDialog::getOpenFileName(this,
                                            tr("Open JSON configuration"), QString(),
                                            tr("Json Files [*.json](*.json);;All Files [*.*](*.*)"));

    if (fileName.isNull()) return; // cancelled by user

    // Check the selected file
    QFile f(fileName);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Open JSON",
                             QString("Could not open file:\n%1").arg(fileName),
                             QMessageBox::Ok);
        return;
    }

    QByteArray json = f.readAll();

    // Is it a valid json config ??
    mcdriver::options opt;
    std::istringstream is(json.constData());
    std::ostringstream os;
    if (opt.parseJSON(is,false,&os)!=0) {
        QMessageBox::warning(this, "Open JSON",
                             QString("Error parsing JSON file:\n%1\n%2")
                                 .arg(fileName)
                                 .arg(os.str().c_str()),
                             QMessageBox::Ok);
        return;
    }

    ionsui->driverObj()->loadJson(fileName);
    ionsui->setCurrentPage(IonsUI::idConfigPage);
    // set path to loaded file
    QFileInfo finfo(fileName);
    QString dirPath = finfo.dir().absolutePath();
    bool ret = QDir::setCurrent(dirPath);

}

void WelcomeView::onSaveJson()
{
    QString fname = ionsui->driverObj()->fileName();
    fname += ".json";
    ionsui->driverObj()->saveJson(fname);
}

void WelcomeView::onSaveJsonAs()
{
    QString fname = ionsui->driverObj()->fileName();
    fname += ".json";
    QFileInfo finfo(fname);
    QString selectedFileName = QFileDialog::getSaveFileName(this, tr("Save JSON configuration As ..."),
                                                    finfo.absolutePath(),
                                                    tr("Json files [*.json](*.json);; All files (*.*)"));
    if (selectedFileName.isNull()) return;
    QFileInfo finfo2(selectedFileName);
    if (finfo2.suffix().toLower() != "json") selectedFileName += ".json";

    QString dirPath = finfo2.dir().absolutePath();
    bool ret = QDir::setCurrent(dirPath);

    ionsui->driverObj()->saveJson(selectedFileName);
}

void WelcomeView::onSaveH5()
{
    QString fname = ionsui->driverObj()->fileName();
    fname += ".h5";
    ionsui->driverObj()->saveH5(fname);
}

void WelcomeView::onSaveH5As()
{
    QString fname = ionsui->driverObj()->fileName();
    fname += ".h5";
    QFileInfo finfo(fname);
    QString selectedFileName = QFileDialog::getSaveFileName(this, tr("Save HDF5 (Config+Data) As ..."),
                                                            finfo.absolutePath(),
                                                            tr("HDF5 files [*.h5](*.h5);; All files (*.*)"));
    if (selectedFileName.isNull()) return;
    QFileInfo finfo2(selectedFileName);
    if (finfo2.suffix().toLower() != "h5") selectedFileName += ".h5";

    QString dirPath = finfo2.dir().absolutePath();
    bool ret = QDir::setCurrent(dirPath);

    ionsui->driverObj()->saveH5(selectedFileName);
}

void WelcomeView::onNew()
{    
    if (!userDiscardCurrentSim("New simulation")) return;

    ionsui->driverObj()->loadJson();
    ionsui->setCurrentPage(IonsUI::idConfigPage);
}

QPushButton* WelcomeView::createButton(const QString& txt, int w, int h, int ch)
{
    QPushButton* bt = new QPushButton(txt);
    bt->setMinimumHeight(h);
    bt->setMinimumWidth(w);
    if (ch) {
        bt->setCheckable(true);
        bt->setAutoExclusive(true);
    }
    return bt;
}

void WelcomeView::pushCenterWidget(const QString &title, QWidget *page)
{
    QWidget* w = new QWidget;
    QVBoxLayout* vbox = new QVBoxLayout;
    QLabel* lbl = new QLabel(title);
    lbl->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);
    lbl->setStyleSheet("font-size : 14pt; font-weight : bold;");
    vbox->addWidget(lbl);
    vbox->addWidget(page);
    w->setLayout(vbox);
    stackedWidget->addWidget(w);
}

void WelcomeView::exampleSelected()
{
    QListWidgetItem* i = exampleList->currentItem();
    QFile f(QString(":/examples/%1").arg(i->text()));
    if (f.open(QIODevice::ReadOnly | QIODevice::Text))
        jsonView->setPlainText(f.readAll());
    btOpenExample->setEnabled(true);
}

bool WelcomeView::userDiscardCurrentSim(const QString& title)
{
    McDriverObj::DriverStatus st = ionsui->driverObj()->status();
    QString msg = st == McDriverObj::mcRunning ?
                      "Stop the running simulation & discard data?" :
                      "Discard current simulation?";
    return QMessageBox::question(this,
                                 title,msg,
                                 QMessageBox::Ok | QMessageBox::Cancel)
           == QMessageBox::Ok;

}
