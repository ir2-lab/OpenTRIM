#ifndef IONSUI_H
#define IONSUI_H

#include <QWidget>
#include <QThread>

class QStackedWidget;
class QTextBrowser;
class QToolButton;
class QLabel;
class QButtonGroup;

class OptionsModel;
class SimControlWidget;
class McDriverObj;
class WelcomeView;
class SimulationOptionsView;
class RunView;
class ResultsView;
class TabularView;

#define V_SPACING 15

class MainUI : public QWidget
{
    Q_OBJECT

public:
    OptionsModel *optionsModel;

    SimulationOptionsView *optionsView;

    explicit MainUI(QWidget *parent = nullptr);
    ~MainUI();

    // const McDriverObj* driverObj() const { return ions_driver; }
    McDriverObj *driverObj() { return driverObj_; }

    void push(const QString &title, QWidget *page);
    void pop();

    enum PageId { idWelcomePage = 0, idConfigPage = 1, idRunPage = 2, idResultsPage = 3 };

    PageId currentPage() const;

public slots:
    void setCurrentPage(PageId id);

private slots:
    void changePage(int idx);
    void updateWindowTitle();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    QToolButton *createSidebarButton(const QString &iconPath, const QString &title);

    McDriverObj *driverObj_;

    WelcomeView *welcomeView;
    // RunView *runView;
    TabularView *tblView;
    ResultsView *resultsView;
    QStackedWidget *_stackedWidget;
    QThread runnerThread;
    QButtonGroup *pageButtonGrp;

    SimControlWidget *ctrlWidget;
};

#endif // IONSUI_H
