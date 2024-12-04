#ifndef WELCOMEVIEW_H
#define WELCOMEVIEW_H

#include <QWidget>

class QPushButton;
class QToolButton;
class QStackedWidget;
class QListWidget;
class QTreeWidget;
class JSEdit;
class QButtonGroup;
class QAction;

class IonsUI;

class WelcomeView : public QWidget
{
    Q_OBJECT

public:

    WelcomeView(IonsUI* iui, QWidget *parent = nullptr);

signals:

private slots:
    void changeCenterWidget(int id);
    void onNew();
    void onOpenJson();
    void onOpenH5();
    void onOpenRecent();
    void onRecentFileSelected();
    void onSaveJson();
    void onSaveH5();
    void onSaveJsonAs();
    void onSaveH5As();
    void onOpenExample();
    void onDriverStatusChanged();
    void exampleSelected();
    void onFileNameChanged();

private:
    QPushButton* createButton(const QString& txt, int w, int h, int ch = 0);
    void pushCenterWidget(const QString& title, QWidget * page);

    bool userDiscardCurrentSim(const QString &title);

    void updateRecentFiles();

    void openJson(const QString& path);
    void openH5(const QString& path);

    IonsUI* ionsui;
    QStackedWidget * stackedWidget;
    QListWidget * exampleList;
    QTreeWidget * recentFilesTree;
    JSEdit* jsonView;
    QButtonGroup* buttonGrp;
    QToolButton* btOpen;
    QPushButton* btNew;
    QToolButton* btSave;
    QToolButton* btSaveAs;
    QPushButton* btRecent;
    QPushButton* btOpenRecent;
    QPushButton* btExamples;
    QPushButton* btOpenExample;
    QPushButton* btGettingStarted;
    QPushButton* btAbout;

    QAction* actSaveJson;
    QAction* actSaveJsonAs;
    QAction* actSaveH5;
    QAction* actSaveH5As;

    const int MAX_RECENT_FILES = 50;

};

#endif // WELCOMEVIEW_H
