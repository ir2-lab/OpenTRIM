#ifndef IONSUI_H
#define IONSUI_H

#include <QWidget>
#include <QThread>

class QStackedWidget;
class QTextBrowser;
class QToolButton;
class QLabel;
class QButtonGroup;
class QSplitter;

class OptionsModel;
class SimControlWidget;
class McDriverObj;
class WelcomeView;
class OptionsView;
class RunView;
class ResultsView;
class TabularView;
class Track3DViewport;

#define V_SPACING 15

class MainUI : public QWidget
{
    Q_OBJECT

public:
    OptionsModel *optionsModel;
    OptionsView *optionsView;

    explicit MainUI(QWidget *parent = nullptr);
    ~MainUI();

    McDriverObj *driverObj() { return driverObj_; }

    void push(const QString &title, QWidget *page);
    void pop();

    enum PageId {
        idWelcomePage = 0,
        idConfigPage = 1,
        idTrackViewPage = 2,
        idSummaryPage = 3,
        idResultsPage = 4
    };

    PageId currentPage() const;

public slots:
    void setCurrentPage(PageId id);
    void showQuickStartWidget();

private slots:
    void changePage(int idx);
    void updateWindowTitle();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    QToolButton *createSidebarButton(const QString &iconPath, const QString &title);
    QWidget *createTrackViewPage();

    McDriverObj *driverObj_;

    WelcomeView *welcomeView;
    // RunView *runView;
    Track3DViewport *trackView;
    TabularView *tblView;
    ResultsView *resultsView;
    QStackedWidget *_stackedWidget;
    QThread runnerThread;
    QButtonGroup *pageButtonGrp;
    QWidget *quickStartWidget;

    SimControlWidget *ctrlWidget;
};

#endif // IONSUI_H
