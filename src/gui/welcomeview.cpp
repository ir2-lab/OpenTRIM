#include "welcomeview.h"

#include "ionsui.h"
#include "mcdriver.h"

#include "mcdriverobj.h"

#include <QPushButton>
#include <QToolButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QButtonGroup>
#include <QFontMetrics>
#include <QListWidget>
#include <QListWidgetItem>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QStackedWidget>
#include <QLabel>
#include <QDir>
#include <QDialogButtonBox>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QJsonDocument>
#include <QMenu>
#include <QSettings>
#include <QStyle>
#include <QMimeDatabase>
#include <QMimeType>
#include <QFileIconProvider>

#include "jsedit/jsedit.h"

WelcomeView::WelcomeView(IonsUI* iui, QWidget *parent)
    : QWidget{parent}, ionsui(iui)
{
    QFontMetrics fm = this->fontMetrics();
    QRect rect = fm.boundingRect("OOOOpen SimulationOOO");
    int h = rect.height()*2;
    int w = rect.width();
    btNew =     createButton("New Simulation",w,h,0);

    btOpen = new QToolButton;
    btOpen->setText("Open");
    btOpen->setPopupMode(QToolButton::InstantPopup);
    btOpen->setMinimumSize(QSize(w,h));
    {
        QMenu* toolMenu = new QMenu;
        QAction* action;
        action = toolMenu->addAction("Open JSON config");
        connect(action, &QAction::triggered,
                this, &WelcomeView::onOpenJson);
        action = toolMenu->addAction("Open HDF5 file");
        connect(action, &QAction::triggered,
                this, &WelcomeView::onOpenH5);
        btOpen->setMenu(toolMenu);
    }

    btSave = new QToolButton;
    btSave->setText("Save");
    btSave->setPopupMode(QToolButton::InstantPopup);
    btSave->setMinimumSize(QSize(w,h));
    {
        QMenu* toolMenu = new QMenu;
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

    /* create recent files tree */
    QWidget* recentFilesPage = new QWidget;
    {
        recentFilesTree = new QTreeWidget;
        recentFilesTree->setColumnCount(3);
        QTreeWidgetItem* hdr = recentFilesTree->headerItem();
        hdr->setText(0,"File");
        hdr->setText(1,"Title");
        hdr->setText(2,"Location");

        updateRecentFiles();

        QDialogButtonBox* buttonBox = new QDialogButtonBox(Qt::Horizontal);
        btOpenRecent = new QPushButton(QIcon(":/icons/assets/ionicons/checkmark-done-outline.svg"),
                                        "Open recent simulation");
        buttonBox->addButton(btOpenRecent, QDialogButtonBox::AcceptRole);
        connect(btOpenRecent, &QPushButton::clicked,
                this, &WelcomeView::onOpenRecent);
        btOpenRecent->setEnabled(false);

        QVBoxLayout* vbox = new QVBoxLayout;
        vbox->addWidget(recentFilesTree);
        vbox->addSpacing(20);
        vbox->addWidget(buttonBox);
        vbox->setContentsMargins(0,0,0,0);
        recentFilesPage->setLayout(vbox);
    }
    pushCenterWidget("Recent simulations", recentFilesPage);

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
            QString("%1 - %2\n\nVersion: %3\nBuild time: %4\nCompiler: %5 v%6\nSystem: %7")
                .arg(PROJECT_NAME)
                .arg(PROJECT_DESCRIPTION)
                .arg(PROJECT_VERSION)
                .arg(BUILD_TIME)
                .arg(COMPILER_ID)
                .arg(COMPILER_VERSION)
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
    connect(recentFilesTree, &QTreeWidget::currentItemChanged,
            this, &WelcomeView::onRecentFileSelected);
    connect(recentFilesTree, &QTreeWidget::itemDoubleClicked,
            this, &WelcomeView::onOpenRecent);


    connect(btNew, &QPushButton::clicked,
            this, &WelcomeView::onNew);

//    connect(ionsui->driverObj(),&McDriverObj::modificationChanged,
//            actSaveJson,&QAction::setEnabled);
    connect(ionsui->driverObj(),&McDriverObj::modificationChanged,
            this,&WelcomeView::onDriverStatusChanged);
    connect(ionsui->driverObj(),&McDriverObj::statusChanged,
            this,&WelcomeView::onDriverStatusChanged);
    connect(ionsui->driverObj(), &McDriverObj::fileNameChanged,
            this, &WelcomeView::onFileNameChanged);

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
        ionsui->driverObj()->loadJsonTemplate(f.fileName());
        ionsui->setCurrentPage(IonsUI::idConfigPage);
    }
}

void WelcomeView::onDriverStatusChanged()
{
    McDriverObj* D = ionsui->driverObj();
    auto s = D->status();
    actSaveJson->setEnabled(
        (D->isModified() || D->isTemplate()));
    actSaveH5->setEnabled(
        (D->isModified() || D->isTemplate()) &&
        s==McDriverObj::mcIdle);
    actSaveH5As->setEnabled(s==McDriverObj::mcIdle);
}

void WelcomeView::onOpenJson()
{
    if (!userDiscardCurrentSim("Open JSON")) return;

    QString fileName = QFileDialog::getOpenFileName(this,
                                            tr("Open JSON configuration"), QString(),
                                            tr("Json Files [*.json](*.json);;All Files [*.*](*.*)"));

    if (fileName.isNull()) return; // cancelled by user

    openJson(fileName);
}

void WelcomeView::onOpenH5()
{
    if (!userDiscardCurrentSim("Open HDF5")) return;

    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open HDF5 file"), QString(),
                                                    tr("HDF5 Files [*.h5](*.h5);;All Files [*.*](*.*)"));

    if (fileName.isNull()) return; // cancelled by user

    openH5(fileName);
}

void WelcomeView::onOpenRecent()
{
    QTreeWidgetItem* i = recentFilesTree->currentItem();
    QString fname = i->data(0,Qt::DisplayRole).toString();
    QString loc = i->data(2,Qt::DisplayRole).toString();
    QFileInfo info(QDir(loc),fname);
    if (!info.exists()) {
        QMessageBox::critical(ionsui,
                              "Open recent file ...",
                              QString("The file does not exist:\n%1")
                                  .arg(fname)
                              );
        return;
    }

    if (!userDiscardCurrentSim("Open recent file ...")) return;

    QMimeDatabase db;
    QMimeType mime = db.mimeTypeForFile(info);
    if (mime.inherits("text/plain")) {
        // The file is plain text, try to load as json
        openJson(info.absoluteFilePath());

    } else openH5(info.absoluteFilePath());

}

void WelcomeView::onRecentFileSelected()
{
    btOpenRecent->setEnabled(true);
}

void WelcomeView::onSaveJson()
{
    if (ionsui->driverObj()->isTemplate()) {
        onSaveJsonAs();
        return;
    }
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
                                                    finfo.absoluteFilePath(),
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
    if (ionsui->driverObj()->isTemplate()) {
        onSaveH5As();
        return;
    }

    // Let driver save the file
    McDriverObj* D = ionsui->driverObj();
    QString fname = D->fileName();
    fname += ".h5";
    if (!D->saveH5(fname)) {
        QMessageBox::warning(this, "Save to HDF5",
                             QString("Error creating file:\n%1\n%2")
                                 .arg(fname)
                                 .arg(D->ioErrorMsg()),
                             QMessageBox::Ok);
        return;
    }
}

void WelcomeView::onSaveH5As()
{
    QString fname = ionsui->driverObj()->fileName();
    fname += ".h5";
    QFileInfo finfo(fname);
    QString selectedFileName = QFileDialog::getSaveFileName(this, tr("Save HDF5 (Config+Data) As ..."),
                                                            finfo.absoluteFilePath(),
                                                            tr("HDF5 files [*.h5](*.h5);; All files (*.*)"));
    if (selectedFileName.isNull()) return;
    QFileInfo finfo2(selectedFileName);
    if (finfo2.suffix().toLower() != "h5") selectedFileName += ".h5";

    QString dirPath = finfo2.dir().absolutePath();
    QDir::setCurrent(dirPath);

    // Let driver save the file
    McDriverObj* D = ionsui->driverObj();
    if (!D->saveH5(selectedFileName)) {
        QMessageBox::warning(this, "Save to HDF5",
                             QString("Error creating file:\n%1\n%2")
                                 .arg(selectedFileName)
                                 .arg(D->ioErrorMsg()),
                             QMessageBox::Ok);
        return;
    }
}

void WelcomeView::onNew()
{    
    if (!userDiscardCurrentSim("New simulation")) return;

    ionsui->driverObj()->loadJsonTemplate();
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

void WelcomeView::onFileNameChanged()
{
    QString filePath = ionsui->driverObj()->filePath();
    if (filePath.isEmpty()) return;

    QFileInfo info(filePath);
    QString name = info.fileName();
    QString location = info.absolutePath();
    QString title = ionsui->driverObj()->title();

    QSettings settings("ir2-lab", "ions-gui");
    QStringList fileNames = settings.value("recentFiles/names").toStringList();
    QStringList fileTitles = settings.value("recentFiles/titles").toStringList();
    QStringList fileLocations = settings.value("recentFiles/locations").toStringList();

    // check for duplicates
    for(int i=0; i<fileNames.size(); i++) {
        if (name == fileNames.at(i) &&
            location == fileLocations.at(i) &&
            title == fileTitles.at(i))
        {
            fileNames.removeAt(i);
            fileLocations.removeAt(i);
            fileTitles.removeAt(i);
        }
    }

    while (fileNames.size() >= MAX_RECENT_FILES)
    {
        fileNames.removeLast();
        fileTitles.removeLast();
        fileLocations.removeLast();
    }

    fileNames.prepend(name);
    fileTitles.prepend(title);
    fileLocations.prepend(location);

    settings.setValue("recentFiles/names", fileNames);
    settings.setValue("recentFiles/titles", fileTitles);
    settings.setValue("recentFiles/locations", fileLocations);

    updateRecentFiles();

}

void WelcomeView::updateRecentFiles()
{
    QSettings settings("ir2-lab", "ions-gui");
    QStringList fileNames = settings.value("recentFiles/names").toStringList();
    QStringList fileTitles = settings.value("recentFiles/titles").toStringList();
    QStringList fileLocations = settings.value("recentFiles/locations").toStringList();

    recentFilesTree->clear();
    QFileIconProvider fip;
    for(int i=0; i<fileNames.size(); i++) {
        QTreeWidgetItem* item = new QTreeWidgetItem(
            recentFilesTree,
            QStringList({fileNames.at(i),
                         fileTitles.at(i),
                         fileLocations.at(i)})
            );
        QFileInfo info(fileLocations.at(i),fileNames.at(i));
        item->setIcon(0,fip.icon(info));
    }

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

void WelcomeView::openJson(const QString &path)
{
    // Check the selected file
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Open JSON",
                             QString("Could not open file:\n%1").arg(path),
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
                                 .arg(path)
                                 .arg(os.str().c_str()),
                             QMessageBox::Ok);
        return;
    }

    // set path to loaded file
    QFileInfo finfo(path);
    QString dirPath = finfo.dir().absolutePath();
    QDir::setCurrent(dirPath);

    ionsui->driverObj()->loadJsonFile(path);
    ionsui->setCurrentPage(IonsUI::idConfigPage);
}

void WelcomeView::openH5(const QString &path)
{
    // Let driver load the file
    McDriverObj* D = ionsui->driverObj();
    if (!D->loadH5File(path)) {
        QMessageBox::warning(this, "Open HDF5",
                             QString("Error opening file:\n%1\n%2")
                                 .arg(path)
                                 .arg(D->ioErrorMsg()),
                             QMessageBox::Ok);
        return;
    }

    ionsui->setCurrentPage(IonsUI::idConfigPage);

    // set path to loaded file
    QFileInfo finfo(path);
    QString dirPath = finfo.dir().absolutePath();
    QDir::setCurrent(dirPath);
}
