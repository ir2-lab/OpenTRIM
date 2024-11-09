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

    // status bar
    QProgressBar* progressBar;
    QLabel* statusLabel;


    explicit IonsUI(QWidget *parent = nullptr);
    ~IonsUI();

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

protected:
    void closeEvent(QCloseEvent *event) override;


private:
    QToolButton * createSidebarButton(const QString& iconPath, const QString& title);
    QStackedWidget * _stackedWidget;
    QStatusBar * statusBar;
    QThread runnerThread;
    QButtonGroup* pageButtonGrp;
};



#endif // IONSUI_H
