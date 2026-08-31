#ifndef REGIONSVIEW_H
#define REGIONSVIEW_H

#include <QWidget>

#include "validatingitemdelegate.h"

class QToolButton;
class QItemSelection;
class QItemSelectionModel;
class QTableView;

class VectorLineEdit;
class IntVectorLineEdit;
class OptionsModel;
class OptionsView;
class OptionWidgetMapper;
class RegionsView;
class MainUI;

class RegionsModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    RegionsModel(OptionsModel *m, QObject *parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    bool insertRows(int position, int rows, const QModelIndex &parent = QModelIndex()) override;
    bool removeRows(int position, int rows, const QModelIndex &parent = QModelIndex()) override;
    bool moveRow(int from, int to);

    void resetModel();

private:
    OptionsModel *model_;
    QPersistentModelIndex regionsIndex_;
    QStringList col_names_{ "id", "material_id", "origin", "size" };
    QStringList col_labels_{ "Region id", "Material id", "[x₀,y₀,z₀]", "[Lx,Ly,Lz]" };
    friend class RegionDelegate;
    float origin_lim_[2];
    float size_lim_[2];
};

class RegionDelegate : public ValidatingItemDelegate
{
    Q_OBJECT

public:
    RegionDelegate(QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override;

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const override;
};

class RegionsView : public QWidget
{
    Q_OBJECT

public:
    RegionsView(OptionsView *m, QWidget *parent = nullptr);

    RegionsModel *model() const { return model_; }

signals:
    void regionsChanged();

public slots:
    void revert();
    void addRegion();
    void removeRegion();
    void moveRegionUp();
    void moveRegionDown();
    void onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

private slots:
    void onDataChanged(const QModelIndex &, const QModelIndex &) { emit regionsChanged(); }
    void onRowsInserted(const QModelIndex &, int, int) { emit regionsChanged(); }
    void onRowsMoved(const QModelIndex &, int, int, const QModelIndex &, int)
    {
        emit regionsChanged();
    }
    void onRowsRemoved(const QModelIndex &, int, int) { emit regionsChanged(); }

private:
    // Top toolbar for add/remove, move up/down
    QToolButton *btAdd;
    QToolButton *btRemove;
    QToolButton *btUp;
    QToolButton *btDown;

    // regions model/view & selection
    RegionsModel *model_;
    QTableView *tableView;
    RegionDelegate *delegate_;
    QItemSelectionModel *selectionModel;

    // pointer to parent options view
};

#endif // REGIONSVIEW_H
