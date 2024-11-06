#ifndef MATERIALSDEFVIEW_H
#define MATERIALSDEFVIEW_H

#include <QWidget>
#include <QComboBox>
#include <QStyledItemDelegate>

class MyComboBox;
class QTableWidget;
class QDoubleSpinBox;
class QLineEdit;
class QToolButton;
class QItemSelection;
class QItemSelectionModel;
class MaterialCompositionView;
class OptionsModel;

class MaterialsDefView : public QWidget
{
    Q_OBJECT

    MyComboBox* cbMaterialID;
    QToolButton* btAddMaterial;
    QToolButton* btDelMaterial;
    QDoubleSpinBox* sbDensity;
    MaterialCompositionView* materialsView;
    OptionsModel* model_;

    QPersistentModelIndex targetIndex_;

    //json_ref value_ref_;

public:
    MaterialsDefView(OptionsModel* m, QWidget *parent = nullptr);

    //void setValueRef(json_ref& vref);
    //const json_ref& valueRef() const { return value_ref_; }

signals:

public slots:
    void addMaterial();
    void removeMaterial();
    void editMaterialName();
    void setWidgetData();
    void setValueData();
    void updateSelectedMaterial();
    void setDensity(double v);
};

class MyComboBox : public QComboBox
{
    Q_OBJECT

public:
    MyComboBox() : QComboBox()
    {}

signals:
    void doubleClicked();

protected:
    void mouseDoubleClickEvent(QMouseEvent *event) override
    {
        emit doubleClicked();
    }
};

class OptionsModel;

class MaterialCompositionModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    MaterialCompositionModel(OptionsModel* m, QObject *parent = 0);

    void setMaterialIdx(int i = -1);
    int materialIdx() const { return materialIdx_; }

    QJsonObject getMaterial() const;

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

private:

    void setMaterial(const QJsonObject& mat);
    int materialIdx_;
    OptionsModel* model_;
    QPersistentModelIndex materialsIndex_;
    QStringList col_names_{"Z", "M", "X", "Ed", "El", "Es", "Er"};
    QStringList col_labels_{"id", "M", "X", "Ed", "El", "Es", "Er"};
};

class MaterialCompositionDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    MaterialCompositionDelegate(QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override;

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const override;
};

class MaterialCompositionView : public QWidget
{
    Q_OBJECT

    MaterialCompositionModel* model_;
    MaterialCompositionDelegate* delegate_;
    QToolButton* btAdd;
    QToolButton* btRemove;
    QItemSelectionModel* selectionModel;

public:
    MaterialCompositionView(OptionsModel* m, QObject *parent = nullptr);

    void setMaterialIdx(int i = -1);

public slots:
    void addElement();
    void removeElement();
    void onSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
};
#endif // MATERIALSDEFVIEW_H
