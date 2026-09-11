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
class SummaryView;
class TrackViewWidget;

#define V_SPACING 10

class Page;

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
    void addPage(Page *page);
    void pop();

    // Sets a heading-style font on w, sized relative to its current font.
    // headingLevel follows HTML conventions: 1 = h1 (largest) ... 3 = h3.
    static void setHeadingFont(QWidget *w, int headingLevel, bool bold = true);

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

    McDriverObj *driverObj_;

    WelcomeView *welcomeView;
    TrackViewWidget *trackView;
    SummaryView *tblView;
    ResultsView *resultsView;

    QStackedWidget *_stackedWidget;

    QThread runnerThread;
    QButtonGroup *pageButtonGrp;

    QWidget *quickStartWidget;

    SimControlWidget *ctrlWidget;
};

class QLabel;
class QLineEdit;
class OptionWidgetMapper;

class Page : public QWidget
{
    Q_OBJECT

public:
    Page(MainUI *ui, const QString &title, bool hasSimTitle = true, QWidget *parent = nullptr);
    void setContent(QWidget *w);
    QLabel *lblTitle{ nullptr };
    QLabel *lblSimTitle{ nullptr };
    QLineEdit *edtSimTitle{ nullptr };
    QWidget *content{ nullptr };
    OptionWidgetMapper *mapper;

public slots:
    virtual void revert();
};

#endif // IONSUI_H
