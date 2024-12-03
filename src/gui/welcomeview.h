#ifndef WELCOMEVIEW_H
#define WELCOMEVIEW_H

#include <QWidget>

class QPushButton;
class QToolButton;
class QStackedWidget;
class QListWidget;
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
    void onSaveJson();
    void onSaveH5();
    void onSaveJsonAs();
    void onSaveH5As();
    void onOpenExample();
    void onDriverStatusChanged();

private:
    QPushButton* createButton(const QString& txt, int w, int h, int ch = 0);
    void pushCenterWidget(const QString& title, QWidget * page);
    void exampleSelected();
    bool userDiscardCurrentSim(const QString &title);

    IonsUI* ionsui;
    QStackedWidget * stackedWidget;
    QListWidget * exampleList;
    JSEdit* jsonView;
    QButtonGroup* buttonGrp;
    QToolButton* btOpen;
    QPushButton* btNew;
    QToolButton* btSave;
    QToolButton* btSaveAs;
    QPushButton* btRecent;
    QPushButton* btExamples;
    QPushButton* btOpenExample;
    QPushButton* btGettingStarted;
    QPushButton* btAbout;

    QAction* actSaveJson;
    QAction* actSaveJsonAs;
    QAction* actSaveH5;
    QAction* actSaveH5As;

};

#endif // WELCOMEVIEW_H
