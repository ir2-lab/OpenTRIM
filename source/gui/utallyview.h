#ifndef UTALLYVIEW_H
#define UTALLYVIEW_H

#include <QWidget>
#include <QPlainTextEdit>
#include <QAbstractTableModel>
#include <QVector>
#include <QPersistentModelIndex>

#include "validatingitemdelegate.h"

class QComboBox;
class QToolButton;
class QLabel;
class QGroupBox;
class QTableView;
class QItemSelection;
class QItemSelectionModel;
class QFocusEvent;
class QVectorEdit;

class MyComboBox;
class OptionsModel;
class OptionsView;
class UserTallyBinsModel;
class UserTallyBinDelegate;

// A QPlainTextEdit that reports commit on focus-out, matching the
// codebase's convention that string fields commit on "editingFinished"
// (see StringOptionsItem::editorSignal()) rather than on every keystroke.
class DescriptionTextEdit : public QPlainTextEdit
{
    Q_OBJECT

public:
    DescriptionTextEdit(QWidget *parent = nullptr) : QPlainTextEdit(parent) { }

signals:
    void editingFinished();

protected:
    void focusOutEvent(QFocusEvent *event) override
    {
        QPlainTextEdit::focusOutEvent(event);
        emit editingFinished();
    }
};

// Table model for the "bins" section of a single UserTally entry.
//
// user_tally::parameters::bins (bin_var_t) is a fixed struct of 14 named
// std::vector<float> members, not a real array. Each table row picks one
// of those 14 named variables and shows/edits its vector. Row order has
// no backing storage in the config -- it is UI-only bookkeeping
// (rowVar_), normally restored from the owning combo box's per-item data
// (see UserTallyView) so that within a session the user's insertion order
// is preserved; falling back to canonical (declaration) order otherwise.
//
// Bin values are read/written through mcconfig::get()/set() with a JSON
// pointer path built from the variable's name -- the JSON field names are
// identical to the C++ member names, so no pointer-to-member table is
// needed.
class UserTallyBinsModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    UserTallyBinsModel(OptionsModel *m, QObject *parent = nullptr);

    // Switch to editing UserTally[i]'s bins (i<0 : none).
    // savedOrder, if non-empty, is used as-is for rowVar_ (the order the
    // user built up interactively). If empty, row order is rebuilt by
    // scanning the 14 variables in canonical order for non-empty ones.
    void setTallyIdx(int i = -1, const QVector<int> &savedOrder = QVector<int>());
    int tallyIdx() const { return tallyIdx_; }

    // Current row order (indices into the 14-variable metadata table),
    // exposed so the view can persist it into the owning combo box.
    const QVector<int> &rowVar() const { return rowVar_; }

    bool isFull() const { return rowVar_.size() >= varSpecs_.size(); }

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    bool insertRows(int position, int rows, const QModelIndex &parent = QModelIndex()) override;
    bool removeRows(int position, int rows, const QModelIndex &parent = QModelIndex()) override;

private:
    friend class UserTallyBinDelegate;

    struct bin_var_spec_t
    {
        QString name;
        QString label;
        QString toolTip;
        float min{ 0 };
        float max{ 0 };
        int digits{ 6 };
    };

    QString binPath(int varIdx) const;
    // notify parent OptionsModel that a change occured in UserTally
    void notifyChanged();

    OptionsModel *model_;
    QPersistentModelIndex utallyIndex_;
    int tallyIdx_{ -1 };
    // row -> index into varSpecs_
    QVector<int> rowVar_;
    // static metadata for the 14 possible bin variables, canonical order
    QVector<bin_var_spec_t> varSpecs_;
};

class UserTallyBinDelegate : public ValidatingItemDelegate
{
    Q_OBJECT

public:
    UserTallyBinDelegate(QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override;

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const override;
};

class UserTallyView : public QWidget
{
    Q_OBJECT

public:
    UserTallyView(OptionsView *v, QWidget *parent = nullptr);

public slots:
    void setWidgetData();

    void addTally();
    void removeTally();
    void editTallyName();
    void updateSelectedTally();

    void setDescription();
    void setEvent(int idx);
    void setOrigin();
    void setZAxis();
    void setXZVector();

    void addBin();
    void removeBin();
    void onBinSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

private slots:
    void onBinsChanged();

private:
    OptionsModel *model_;
    // index of the UserTally array in the main options model, used only
    // to fake-notify OptionsModel::dataChanged after direct edits
    QPersistentModelIndex utallyIndex_;

    // header row: id combo + add/remove/rename buttons
    MyComboBox *cbTallyID;
    QToolButton *btAddTally;
    QToolButton *btDelTally;
    QToolButton *btEdtTally;

    DescriptionTextEdit *edtDescription;
    QComboBox *cbEvent;

    QGroupBox *coordBox;
    QVectorEdit *edtOrigin;
    QVectorEdit *edtZAxis;
    QVectorEdit *edtXZVector;

    QToolButton *btAddBin;
    QToolButton *btRemoveBin;
    QTableView *binsTable;
    UserTallyBinsModel *binsModel;
    UserTallyBinDelegate *binsDelegate;
    QItemSelectionModel *binsSelectionModel;

    // notify parent OptionsModel that a change occured in UserTally
    void notifyChanged();
};

#endif // UTALLYVIEW_H
