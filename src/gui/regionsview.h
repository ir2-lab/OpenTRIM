#ifndef REGIONSVIEW_H
#define REGIONSVIEW_H

#include <QWidget>
#include <QStyledItemDelegate>

class QToolButton;
class QItemSelection;
class QItemSelectionModel;
class QTableView;

class Vector3dLineEdit;
class IntVector3dLineEdit;
class OptionsModel;
class MyDataWidgetMapper;
class RegionsView;
class IonsUI;

class RegionsModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    RegionsModel(OptionsModel* m, QObject *parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

    bool insertRows(int position, int rows,
                    const QModelIndex &parent = QModelIndex()) override;
    bool removeRows(int position, int rows,
                    const QModelIndex &parent = QModelIndex()) override;
    bool moveRow(int from, int to);

    void resetModel();

private:
    OptionsModel* model_;
    QPersistentModelIndex regionsIndex_;
    QStringList col_names_{"id", "material_id", "min", "max"};
    QStringList col_labels_{"Region id", "Material id", "𝖱₀", "𝖱₁"};
    friend class RegionDelegate;
};

class RegionDelegate : public QStyledItemDelegate
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

    QToolButton* btAdd;
    QToolButton* btRemove;
    QToolButton* btUp;
    QToolButton* btDown;
    QItemSelectionModel* selectionModel;
    QTableView* tableView;

    RegionDelegate* delegate_;
    RegionsModel* model_;

public:
    RegionsView(OptionsModel* m, QObject *parent = nullptr);

public slots:
    void revert();
    void addRegion();
    void removeRegion();
    void moveRegionUp();
    void moveRegionDown();
    void onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
};

#endif // REGIONSVIEW_H
