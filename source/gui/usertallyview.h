#ifndef USERTALLYVIEW_H
#define USERTALLYVIEW_H

#include <QAbstractTableModel>
#include <QDialog>
#include <QPersistentModelIndex>
#include <QWidget>

#include "mcdriver.h"

class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QSpinBox;
class QTableView;
class QToolButton;
class QModelIndex;
class QStringList;
class QListWidget;
class QTextBrowser;

class OptionsModel;

class BinVariableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    struct Row {
        QString variable;
        QString edges;
    };

    explicit BinVariableModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

    void setRows(const std::vector<Row> &rows);
    std::vector<Row> rows() const;
    void clear();

    static QStringList variableNames();
    static QString variableDescription(const QString &name);
    static std::vector<float> parseEdges(const QString &text);
    static QString edgesToString(const std::vector<float> &edges);
    static bool isStrictlyMonotonic(const std::vector<float> &edges);

signals:
    void rowsChanged();

private:
    std::vector<Row> rows_;
};

class LinspaceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LinspaceDialog(QWidget *parent = nullptr);
    std::vector<float> edges() const;

private slots:
    void updatePreview();

private:
    QDoubleSpinBox *min_;
    QDoubleSpinBox *max_;
    QSpinBox *bins_;
    QLabel *preview_;
};

class TallyTemplateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TallyTemplateDialog(QWidget *parent = nullptr);
    user_tally::parameters selectedTemplate() const;

private slots:
    void onCurrentTemplateChanged(int row);

private:
    struct TemplateEntry {
        user_tally::parameters parameters;
    };

    bool loadTemplates();
    void refreshPreview(int row);

    QListWidget *list_;
    QTextBrowser *preview_;
    QPushButton *accept_;
    std::vector<TemplateEntry> entries_;
};

class UserTallyView : public QWidget
{
    Q_OBJECT

public:
    explicit UserTallyView(OptionsModel *m, QWidget *parent = nullptr);

public slots:
    void setWidgetData();
    void setValueData();
    void addTally();
    void removeTally();
    void renameTally();
    void duplicateTally();
    void addBinVariable();
    void loadFromTemplate();
    void updateSelectedTally();
    void updateSummaryLabel();

private slots:
    void applyLinspaceToCurrentRow();
    void onBinTableClicked(const QModelIndex &index);
    void onUseLabFrameChanged(int state = 0);

private:
    Event currentEvent() const;
    void setCurrentEvent(Event ev);
    user_tally::parameters *currentTally();
    const user_tally::parameters *currentTally() const;

    QComboBox *cbTallyID_;
    QToolButton *btDbTally_;
    QToolButton *btAddTally_;
    QToolButton *btDupTally_;
    QToolButton *btDelTally_;
    QToolButton *btEdtTally_;
    QToolButton *btAddBin_;

    QLabel *summaryLabel_;
    QLabel *memoryLabel_;
    QLabel *warningLabel_;

    QLineEdit *leDescription_;
    QComboBox *cbEvent_;

    QCheckBox *cbUseLabFrame_;
    QWidget *coordWidget_;
    QDoubleSpinBox *sbOrigin_[3];
    QDoubleSpinBox *sbZaxis_[3];
    QDoubleSpinBox *sbXZvec_[3];

    BinVariableModel *binModel_;
    QTableView *binTableView_;
    bool syncingUi_ = false;

    OptionsModel *model_;
    QPersistentModelIndex tallyIndex_;
};

#endif // USERTALLYVIEW_H
