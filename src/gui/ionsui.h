#ifndef IONSUI_H
#define IONSUI_H

#include <QWidget>
#include <QThread>

#include "mcdriverobj.h"
#include "welcomeview.h"
#include "simulationoptionsview.h"
#include "runview.h"
#include "resultsview.h"

class QStackedWidget;
class QTextBrowser;
class QToolButton;
class QStatusBar;
class QProgressBar;
class QLabel;

class OptionsModel;

class IonsUI : public QWidget
{
    Q_OBJECT

public:

    McDriverObj* ions_driver;

    OptionsModel* optionsModel;

    WelcomeView* welcomeView;
    SimulationOptionsView* optionsView;
    RunView* runView;
    ResultsView* resultsView;
    QProgressBar* progressBar;
    QLabel* statusLabel;


    explicit IonsUI(QWidget *parent = nullptr);
    ~IonsUI();

    void push(const QString& title, QWidget * page);
    void pop();

public slots:
    void changeCenterWidget(bool);

protected:
    void closeEvent(QCloseEvent *event) override;


private:
    QToolButton * createSidebarButton(const QString& iconPath, const QString& title, int idx);
    QStackedWidget * _stackedWidget;
    QToolButton * _activeButton;
    QStatusBar * statusBar;
    QThread runnerThread;
};



#endif // IONSUI_H
