#include "targetgeometryview.h"
#include "floatlineedit.h"
#include "optionsmodel.h"
#include "mydatawidgetmapper.h"
#include "ionsui.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGridLayout>
#include <QSpacerItem>

#include <QLabel>
#include <QToolButton>
#include <QDoubleSpinBox>
#include <QTableView>
#include <QComboBox>
#include <QInputDialog>
#include <QMessageBox>
#include <QFontMetrics>
#include <QItemSelectionModel>

#include <QJsonArray>
#include <QJsonObject>

TargetGeometryView::TargetGeometryView(IonsUI *iui, QWidget *parent)
    : QWidget{parent}, ionsui(iui)
{

    OptionsModel* model_ = ionsui->optionsModel;

    targetIndex_ = model_->index("Target",1);

    mapper = new MyDataWidgetMapper(model_,this);
    QModelIndex idx = model_->index("Target",0);
    assert(idx.isValid());
    QWidget* widget = new QWidget;
    QFormLayout* flayout = new QFormLayout;
    for(int j=0; j<3; ++j) {
        QModelIndex idx2 = model_->index(j,0,idx);
        OptionsItem* item = model_->getItem(idx2);
        QWidget* w = item->createEditor(widget);
        QLabel* lbl = new QLabel(item->name(),widget);
        lbl->setToolTip(w->toolTip());
        lbl->setWhatsThis(w->whatsThis());
        mapper->addMapping(w,idx2,item->editorSignal());
        flayout->addRow(lbl,w);
    }

    QHBoxLayout* hbox = new QHBoxLayout;
    hbox->addLayout(flayout);
    hbox->addStretch();

    QVBoxLayout* vbox = new QVBoxLayout;
    vbox->addLayout(hbox);
    vbox->addSpacing(20);
    regionsView = new RegionsView(model_);
    vbox->addWidget(regionsView);
    setLayout(vbox);
}


void TargetGeometryView::setWidgetData()
{
    mapper->revert();
    regionsView->revert();
}

/*****************************************************/
RegionsModel::RegionsModel(OptionsModel *m, QObject *parent)
    : QAbstractTableModel(parent), model_(m)
{
    QModelIndex idx = model_->index("Target");
    idx = model_->index("regions",1,idx);
    regionsIndex_ = idx;
}
int RegionsModel::rowCount(const QModelIndex & /* parent */) const
{
    QJsonArray regions = model_->data(regionsIndex_, Qt::EditRole).toJsonArray();
    if (regions.isEmpty()) return 0;
    return regions.count();
}
int RegionsModel::columnCount(const QModelIndex & /* parent */) const
{
    return col_names_.size(); // "id", "material_id", "min", "max"
}
QVariant RegionsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || (role != Qt::DisplayRole && role != Qt::EditRole))
        return QVariant();

    int i = index.row(), j = index.column();
    if (j<0 || j>=columnCount()) return QVariant();
    if (i<0 || i>=rowCount()) return QVariant();

    QJsonArray regions = model_->data(regionsIndex_, Qt::EditRole).toJsonArray();
    QJsonObject reg = regions[i].toObject();

    QVariant V;
    Vector3D vec;

    switch (j) {
    case 0:
        V = reg["id"].toString();
        break;
    case 1:
        V = reg["material_id"].toString();
        break;
    case 2:
        vec = Vector3D::fromJsonValue(reg["min"]);
        V = (role == Qt::DisplayRole) ?  toString(vec) : QVariant::fromValue(vec);
        break;
    case 3:
        vec = Vector3D::fromJsonValue(reg["max"]);
        V = (role == Qt::DisplayRole) ?  toString(vec) : QVariant::fromValue(vec);
        break;
    default:
        assert(0);
    }

    return V;
}
QVariant RegionsModel::headerData(int c,
                                  Qt::Orientation o,
                                  int role) const
{
    if (role == Qt::DisplayRole && o == Qt::Horizontal)
        return col_labels_[c];
    if (role == Qt::DisplayRole && o == Qt::Vertical)
        return c+1;
    if (role == Qt::SizeHintRole && o == Qt::Horizontal) {
        QTableView tv;
        QFontMetrics fm = tv.fontMetrics();
        QSize sz = fm.boundingRect('O').size();
        const int W[] = {20, 20, 20, 20};
        sz.rwidth() = sz.width()*W[c];
        sz.rheight() = fm.lineSpacing()+8;
        return sz;
    }
    return QVariant();
}

Qt::ItemFlags RegionsModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
}

bool RegionsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    int i = index.row(), j = index.column();
    if (j<0 || j>=columnCount()) return false;
    if (i<0 || i>=rowCount()) return false;

    QJsonArray regions = model_->data(regionsIndex_, Qt::EditRole).toJsonArray();
    QJsonObject reg = regions[i].toObject();

    switch (j) {
    case 0:
        reg["id"] = value.toString();
        break;
    case 1:
        reg["material_id"] = value.toString();
        break;
    case 2:
        reg["min"] = value.value<Vector3D>().toJsonValue();
        break;
    case 3:
        reg["max"] = value.value<Vector3D>().toJsonValue();
        break;
    }

    regions[i] = reg;
    model_->setData(regionsIndex_, regions, Qt::EditRole);

    return true;
}
bool RegionsModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    assert(rows == 1);
    assert(position == rowCount());

    QJsonArray regions = model_->data(regionsIndex_, Qt::EditRole).toJsonArray();
    QJsonObject target = model_->data(model_->index("Target",1), Qt::EditRole).toJsonObject();
    QJsonArray materials = target["materials"].toArray();
    QJsonObject reg;
    reg["id"] = "Region XXX";
    reg["material_id"] = materials.count() ? materials[0].toObject()["id"].toString() : "";
    reg["min"] = Vector3D().toJsonValue();
    reg["max"] = Vector3D(100,100,100).toJsonValue();
    regions.push_back(reg);

    beginInsertRows(parent, position, position);
    bool ret = model_->setData(regionsIndex_,regions);
    endInsertRows();

    return ret;
}
bool RegionsModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    assert(rows == 1);

    QJsonArray regions = model_->data(regionsIndex_, Qt::EditRole).toJsonArray();
    if (regions.isEmpty() || position>=regions.count()) return false;

    regions.removeAt(position);

    beginRemoveRows(parent, position, position);
    model_->setData(regionsIndex_,regions);
    endRemoveRows();

    return true;
}
bool RegionsModel::moveRow(int from, int to)
{
    QJsonArray regions = model_->data(regionsIndex_, Qt::EditRole).toJsonArray();
    if (regions.isEmpty()) return 0;

    int n = regions.count();
    if (from < 0 || from >= n) return false;
    if (to < 0 || to >= n) return false;
    if (to == from) return false;

    QJsonValue reg = regions[from];


    QModelIndex parent = regionsIndex_.parent();
    beginRemoveRows(parent, from, from);
    regions.removeAt(from);
    model_->setData(regionsIndex_, regions);
    endRemoveRows();

    // if (to > from) to = to-1;
    beginInsertRows(parent, to, to);
    regions.insert(to,reg);
    model_->setData(regionsIndex_,regions);
    endInsertRows();

//    if (to < from) insertRows(to, 1);

//    QJsonValue r = regions[to];
//    regions[to] = regions[from];
//    regions[from] = r;

//    QModelIndex parent = regionsIndex_.parent();
//    beginMoveRows(parent,from,from,parent,to);
//    model_->setData(regionsIndex_,regions);
//    endMoveRows();

    return true;
}

//bool RegionsModel::moveRows(const QModelIndex &sourceParent, int sourceRow, int count,
//              const QModelIndex &destinationParent, int destinationChild) override;

/*********************************************************/
RegionDelegate::RegionDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}
QWidget *RegionDelegate::createEditor(QWidget *parent,
                                      const QStyleOptionViewItem &/* option */,
                                      const QModelIndex &index) const
{
    if (!index.isValid()) return nullptr;

    int col = index.column();
    if (col<0 || col>=index.model()->columnCount()) return nullptr;

    const RegionsModel* rmodel = qobject_cast<const RegionsModel*>(index.model());
    if (!rmodel) return nullptr;

    QWidget* w;
    switch (col)
    {
    case 0:
        w = new QLineEdit(parent);
        break;
    case 1:
        {
            QComboBox* cb = new QComboBox(parent);
            OptionsModel* omodel = rmodel->model_;
            QModelIndex targetIdx = omodel->index("Target",1);
            QJsonObject target = omodel->data(targetIdx, Qt::EditRole).toJsonObject();
            QJsonArray materials = target["materials"].toArray();
            for(int i=0; i<materials.count(); ++i) {
                cb->addItem(materials[i].toObject()["id"].toString());
            }
            w = cb;
        }
        break;
    case 2:
    case 3:
        {
            Vector3dLineEdit* edt = new Vector3dLineEdit(0.f,1.e6f,3,parent);
            w = edt;
        }
        break;
    }

    return w;
}
void RegionDelegate::setEditorData(QWidget *editor,
                                   const QModelIndex &index) const
{
    if (!index.isValid()) return;

    int col = index.column();
    if (col<0 || col>=index.model()->columnCount()) return;

    QVariant v = index.model()->data(index, Qt::EditRole);

    switch (col)
    {
    case 0:
        {
            QLineEdit* ledt = (QLineEdit*)editor;
            ledt->setText(v.toString());
        }
        break;
    case 1:
        {
            QComboBox* cb = (QComboBox*)(editor);
            cb->setCurrentText(v.toString());
        }
        break;
    case 2:
    case 3:
        {
            Vector3dLineEdit* edt = (Vector3dLineEdit*)(editor);
            edt->setValue(v.value<Vector3D>());
        }
        break;
    }
}
void RegionDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                               const QModelIndex &index) const
{
    if (!index.isValid()) return;

    int col = index.column();
    if (col<0 || col>=index.model()->columnCount()) return;

    QVariant v;

    switch (col)
    {
    case 0:
    {
        QLineEdit* ledt = (QLineEdit*)editor;
        v = ledt->text();
    }
    break;
    case 1:
    {
        QComboBox* cb = (QComboBox*)(editor);
        v = cb->currentText();
    }
    break;
    case 2:
    case 3:
    {
        Vector3dLineEdit* edt = (Vector3dLineEdit*)(editor);
        v = QVariant::fromValue<Vector3D>(edt->value());
    }
    break;
    }

    model->setData(index, v, Qt::EditRole);
}
void RegionDelegate::updateEditorGeometry(QWidget *editor,
                                                       const QStyleOptionViewItem &option,
                                                       const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}
/*****************************************************************************/
RegionsView::RegionsView(OptionsModel *m, QObject *parent) :
    model_(new RegionsModel(m,this)),
    delegate_(new RegionDelegate(this))
{
    btAdd = new QToolButton;
    btAdd->setIcon(QIcon(":/icons/assets/ionicons/add-outline.svg"));
    btAdd->setToolTip("Add region");
    btRemove = new QToolButton;
    btRemove->setIcon(QIcon(":/icons/assets/ionicons/remove-outline.svg"));
    btRemove->setToolTip("Remove region");
    btUp = new QToolButton;
    btUp->setIcon(QIcon(":/icons/assets/ionicons/arrow-up-outline.svg"));
    btUp->setToolTip("Move region up");
    btDown = new QToolButton;
    btDown->setIcon(QIcon(":/icons/assets/ionicons/arrow-down-outline.svg"));
    btDown->setToolTip("Move region down");

    connect(btAdd, &QToolButton::clicked, this, &RegionsView::addRegion);
    connect(btRemove, &QToolButton::clicked, this, &RegionsView::removeRegion);
    connect(btUp, &QToolButton::clicked, this, &RegionsView::moveRegionUp);
    connect(btDown, &QToolButton::clicked, this, &RegionsView::moveRegionDown);
    //btAdd->setEnabled(false);
    btRemove->setEnabled(false);
    btUp->setEnabled(false);
    btDown->setEnabled(false);

    tableView = new QTableView;
    tableView->setModel(model_);
    tableView->setItemDelegate(delegate_);
    QFontMetrics fm = tableView->fontMetrics();
    QSize sz = fm.boundingRect('O').size();
    //const int W[] = {20,20,20,20};
    int W = 10;
    for(int col=0; col<4; ++col)
        tableView->setColumnWidth(col, sz.width()*W);

    selectionModel = tableView->selectionModel();
    connect(selectionModel, &QItemSelectionModel::selectionChanged,
            this, &RegionsView::onSelectionChanged);

    QHBoxLayout* hbox = new QHBoxLayout;
    hbox->addWidget(new QLabel("Regions "));
    QGridLayout* grid = new QGridLayout;
    grid->setSizeConstraint(QLayout::SetFixedSize);
    grid->addWidget(btAdd,0,0);
    grid->addWidget(btRemove,0,1);
    grid->addWidget(btUp,0,2);
    grid->addWidget(btDown,0,3);
    hbox->addLayout(grid);
    hbox->addStretch();

    QSize fsz(24,24);
    btAdd->setFixedSize(fsz);
    btRemove->setFixedSize(fsz);
    btUp->setFixedSize(fsz);
    btDown->setFixedSize(fsz);

    QVBoxLayout* vbox = new QVBoxLayout;
    vbox->addLayout(hbox);
    vbox->addWidget(tableView);

    setLayout(vbox);

}

void RegionsView::revert()
{
    tableView->setModel(0);
    tableView->setModel(model_);
    disconnect(selectionModel, &QItemSelectionModel::selectionChanged,
            this, &RegionsView::onSelectionChanged);
    selectionModel = tableView->selectionModel();
    connect(selectionModel, &QItemSelectionModel::selectionChanged,
            this, &RegionsView::onSelectionChanged);
}


void RegionsView::addRegion()
{
    int r = model_->rowCount();
    model_->insertRows(r,1,QModelIndex());
}

void RegionsView::removeRegion()
{
    int i=0, n = model_->rowCount();
    while (!selectionModel->isRowSelected(i) && i<n) i++;
    if (i<n) {
        model_->removeRows(i,1);
    }
}

void RegionsView::moveRegionUp()
{
    int i=1, n = model_->rowCount();
    while (!selectionModel->isRowSelected(i) && i<n) i++;
    if (i<n) {
        model_->moveRow(i,i-1);
        selectionModel->select(model_->index(i-1,0),
                               QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
        selectionModel->setCurrentIndex(model_->index(i-1,0), QItemSelectionModel::Select);
    }
}

void RegionsView::moveRegionDown()
{
    int n = model_->rowCount(), i=n-2;
    while (!selectionModel->isRowSelected(i) && i>=0) i--;
    if (i>=0) {
        model_->moveRow(i,i+1);
        selectionModel->select(model_->index(i+1,0),
                               QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
        selectionModel->setCurrentIndex(model_->index(i+1,0), QItemSelectionModel::Select);
    }
}

void RegionsView::onSelectionChanged(const QItemSelection &selected, const QItemSelection &)
{
    int i=0, n = model_->rowCount();
    for(int i=0; i<n; ++i) {
        if (selectionModel->isRowSelected(i)) {
            btRemove->setEnabled(true);
            btUp->setEnabled(i>0);
            btDown->setEnabled(i<n-1);
            return;
        }
    }
}
