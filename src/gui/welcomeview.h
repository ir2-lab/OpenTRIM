#ifndef WELCOMEVIEW_H
#define WELCOMEVIEW_H

#include <QWidget>

class QPushButton;
class QStackedWidget;
class QListWidget;
class QTextBrowser;
class QButtonGroup;

class IonsUI;

class WelcomeView : public QWidget
{
    Q_OBJECT

public:

    QPushButton* btOpen;
    QPushButton* btNew;
    QPushButton* btSave;
    QPushButton* btSaveAs;
    QPushButton* btRecent;
    QPushButton* btExamples;
    QPushButton* btGettingStarted;
    QPushButton* btAbout;

    WelcomeView(IonsUI* iui, QWidget *parent = nullptr);

signals:

private slots:
    void changeCenterWidget(int id);
    void onOpenExample();
    void onOpenJson();
    void onNew();

private:
    QPushButton* createButton(const QString& txt, int w, int h, int ch = 0);
    void pushCenterWidget(const QString& title, QWidget * page);
    void exampleSelected();

    IonsUI* ionsui;
    QStackedWidget * stackedWidget;
    QListWidget * exampleList;
    QTextBrowser* jsonView;
    QButtonGroup* buttonGrp;
};

#endif // WELCOMEVIEW_H
