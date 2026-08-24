#ifndef SIMULATIONOPTIONSVIEW_H
#define SIMULATIONOPTIONSVIEW_H

#include <QWidget>

class QPushButton;
class QToolButton;
class QDialogButtonBox;
class QLabel;
class JSEdit;
class QTabWidget;
class QFormLayout;
class QLineEdit;
class QSplitter;

class OptionWidgetMapper;
class MaterialsDefView;
class RegionsView;
class SimBoxView;
class OptionsModel;
class MainUI;
class HelpPanel;

class OptionsView : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(bool modified READ modified WRITE setModified NOTIFY modifiedChanged FINAL)

public:
    OptionsView(MainUI *iui, QWidget *parent = nullptr);

    bool modified() const { return modified_; }
    OptionWidgetMapper *widgetMapper() const { return mapper; }

public slots:
    void setModified(bool b = true)
    {
        modified_ = b;
        applyRules();
        emit modifiedChanged(b);
    }
    void setModified2(const QModelIndex &, const QModelIndex &, const QVector<int> &)
    {
        setModified();
    }

signals:
    void modifiedChanged(bool);
    void optionsChanged();

public slots:
    void revert();
    void submit();
    void selectIonZ();
    void validateOptions();
    void onDriverStatusChanged();

private:
    bool modified_{ false };
    void applyRules();
    void drawSimBox();

    QWidget *createIonBeamTab(const QModelIndex &idx);
    QWidget *createTargetTab(const QModelIndex &idx);
    QWidget *createTab(const QModelIndex &idx);
    QFormLayout *createForm(const QModelIndex &idx, QWidget *widgetParent = nullptr,
                            const QStringList &excludeKeys = QStringList());

private slots:
    void toggleHelpPanel(bool checked);
    void onSplitterMoved(int a, int b);

private:
    // pointer to main app UI
    MainUI *mainui;

    // Widget structure
    // Top: simulation title view/edit
    QLineEdit *simTitle;
    // Main splitter: options (left) | help (right - closable)
    QSplitter *splitter;
    // left panel - option tabs
    QTabWidget *tabWidget;
    // right help panel
    HelpPanel *helpPanel;
    // top right button to open/close help panel
    QToolButton *helpToggle;
    // bottom button box to accept/validate/cancel changes
    QDialogButtonBox *buttonBox;
    QPushButton *btValidate;
    QPushButton *helpButton; // same func as helpToggle - may be removed

    // Special source ion selection
    QPushButton *btSelectIon;
    QLabel *ionLabel;

    // read-only view of config
    JSEdit *jsonView;

    // map (editor widget) <-> (option model index)
    OptionWidgetMapper *mapper;

    // materials view
    MaterialsDefView *materialsView;
    // regions view
    RegionsView *regionsView;
    // sim box geometry view
    SimBoxView *simBoxView;

    // model & mapper used for help display
    OptionsModel *defaultOptionsModel;
    OptionWidgetMapper *helpMapper;

    friend class MaterialsDefView;
    friend class MaterialCompositionView;
    friend class RegionsView;
};

#endif // SIMULATIONOPTIONSVIEW_H
