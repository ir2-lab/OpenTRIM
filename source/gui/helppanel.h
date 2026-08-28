#ifndef HELPPANEL_H
#define HELPPANEL_H

#include <QTextBrowser>

class QModelIndex;
class OptionsModel;
class OptionsItem;

class OptionWidgetMapper;

class QTimer;

class HelpPanel : public QTextBrowser
{
    Q_OBJECT

public:
    explicit HelpPanel(QWidget *parent = nullptr);
    ~HelpPanel() override;

    void setWidgetMapper(OptionWidgetMapper *m);

    // add a static help display for widget w
    // path points to the options_spec section
    void addStaticHelp(QWidget *w, const QString &path);

    // add a static help display for widget w
    // w may be a table, tab etc that has many sections
    // paths point to the options_spec sections
    void addStaticHelp(QWidget *w, const QStringList &paths);

    bool eventFilter(QObject *watched, QEvent *event) override;

protected:
    void enterEvent(QEvent *ev) override;
    void leaveEvent(QEvent *ev) override;

private:
    // data for static help
    struct HelpData;
    QMap<QWidget *, HelpData *> static_help_map;

    // mapper for dynamic help display (including actual values)
    OptionWidgetMapper *mapper_{ nullptr };
    // The current currentWidget we display help for
    QWidget *currentWidget{ nullptr };
    // combo popup stored while active
    QWidget *comboPopup{ nullptr };
    // help data for static current widget
    HelpData *currentHelpData{ nullptr };
    int currentSection{ 0 };
    // Hides the help text setInterval(1s) after the widget loses focus/hover
    QTimer *hideTimer_{ nullptr };

    void clearCurrent()
    {
        currentWidget = nullptr;
        currentHelpData = nullptr;
        comboPopup = nullptr;
        currentSection = 0;
    }

    // get mouse position for various event types
    static QPoint eventPos(QEvent *e);

    // generic html formatting
    static QString sectionHeader(const QString &title);
    static QString preample(const QString &label, const QString &path, const QString &desc,
                            const QString &type);
    static QString enumHelp(const QStringList &values, const QStringList &labels,
                            const QStringList &valueDescriptions);
    static QString notesHelp(const QStringList &notes);
    static QString numericHelp(const QString &sectionTitle, double minVal, double maxVal);
    static QString vectorHelp(int size, double minVal, double maxVal);

    // html from OptionsItem (dynamic)
    static QString currentValue(OptionsItem *item);
    static QString detectCurrentValue(OptionsItem *item);
    static QString generateHelpHtml(OptionsItem *item);
    static QString enumHelp(OptionsItem *item);
    static QString numericHelp(OptionsItem *item);
    static QString vectorHelp(OptionsItem *item);

    // html from HelpData (static)
    static QString generateHelpHtml(HelpData *item, int section = 0);
    static QString enumHelp(HelpData *item, int section = 0);
    static QString numericHelp(HelpData *item, int section = 0);
    static QString vectorHelp(HelpData *item, int section = 0);
};

#endif 
