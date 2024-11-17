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
class QButtonGroup;

class OptionsModel;
class SimControlWidget;

class IonsUI : public QWidget
{
    Q_OBJECT

public:

    OptionsModel* optionsModel;

    WelcomeView* welcomeView;
    SimulationOptionsView* optionsView;
    RunView* runView;
    ResultsView* resultsView;

    // status bar
    QProgressBar* progressBar;
    QLabel* statusLabel;


    explicit IonsUI(QWidget *parent = nullptr);
    ~IonsUI();

    //const McDriverObj* driverObj() const { return ions_driver; }
    McDriverObj* driverObj() { return driverObj_; }

    void push(const QString& title, QWidget * page);
    void pop();

    enum PageId {
        idWelcomePage = 0,
        idConfigPage = 1,
        idRunPage = 2,
        idResultsPage = 3
    };

    PageId currentPage() const;

public slots:
    void setCurrentPage(PageId id);

private slots:
    void changePage(int idx);
    void updateWindowTitle();

protected:
    void closeEvent(QCloseEvent *event) override;


private:
    QToolButton * createSidebarButton(const QString& iconPath, const QString& title);
    McDriverObj* driverObj_;
    QStackedWidget * _stackedWidget;
    QStatusBar * statusBar;
    QThread runnerThread;
    QButtonGroup* pageButtonGrp;
    SimControlWidget* ctrlWidget;
};



#endif // IONSUI_H
