#include "regionsview.h"
#include "floatlineedit.h"
#include "optionsmodel.h"

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

RegionsModel::RegionsModel(OptionsModel *m, QObject *parent)
    : QAbstractTableModel(parent), model_(m)
{
    QModelIndex idx = model_->index("Target");
    idx = model_->index("regions", 1, idx);
    regionsIndex_ = idx;
}
int RegionsModel::rowCount(const QModelIndex & /* parent */) const
{
    auto &regions = model_->options()->Target.regions;
    return regions.size();
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
    if (j < 0 || j >= columnCount())
        return QVariant();
    if (i < 0 || i >= rowCount())
        return QVariant();

    const mcdriver::options *opt = model_->options();
    const target::region &reg = opt->Target.regions[i];

    QVariant V;

    switch (j) {
    case 0:
        V = QString::fromStdString(reg.id);
        break;
    case 1:
        V = QString::fromStdString(reg.material_id);
        break;
    case 2:
        V = (role == Qt::DisplayRole) ? qstring_serialize<vector3>::toString(reg.origin)
                                      : QVariant::fromValue(reg.origin);
        break;
    case 3:
        V = (role == Qt::DisplayRole) ? qstring_serialize<vector3>::toString(reg.size)
                                      : QVariant::fromValue(reg.size);
        break;
    default:
        assert(0);
    }

    return V;
}
QVariant RegionsModel::headerData(int c, Qt::Orientation o, int role) const
{
    if (role == Qt::DisplayRole && o == Qt::Horizontal)
        return col_labels_[c];
    if (role == Qt::DisplayRole && o == Qt::Vertical)
        return c + 1;
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
    if (j < 0 || j >= columnCount())
        return false;
    if (i < 0 || i >= rowCount())
        return false;

    mcdriver::options *opt = model_->options();
    target::region &reg = opt->Target.regions[i];

    switch (j) {
    case 0:
        reg.id = value.toString().toStdString();
        break;
    case 1:
        reg.material_id = value.toString().toStdString();
        break;
    case 2:
        reg.origin = value.value<vector3>();
        break;
    case 3:
        reg.size = value.value<vector3>();
        break;
    }

    // fake setData just to let model_ know that
    // underlying data changed
    model_->setData(regionsIndex_, QVariant());

    return true;
}
bool RegionsModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    assert(rows == 1);
    assert(position == rowCount());

    beginInsertRows(parent, position, position);
    mcdriver::options *opt = model_->options();
    auto &materials = opt->Target.materials;
    target::region reg;
    reg.id = "Region XX";
    reg.material_id = materials.size() > 0 ? materials[0].id : "";
    reg.origin = { 0.f, 0.f, 0.f };
    reg.size = { 100.f, 100.f, 100.f };
    opt->Target.regions.push_back(reg);
    endInsertRows();

    // fake setData just to let model_ know that
    // underlying data changed
    model_->setData(regionsIndex_, QVariant());

    return true;
}
bool RegionsModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    assert(rows == 1);

    mcdriver::options *opt = model_->options();
    auto &regions = opt->Target.regions;
    if (regions.empty() || position >= regions.size())
        return false;

    beginRemoveRows(parent, position, position);
    auto it = regions.begin() + position;
    regions.erase(it);
    endRemoveRows();

    // fake setData just to let model_ know that
    // underlying data changed
    model_->setData(regionsIndex_, QVariant());

    return true;
}
bool RegionsModel::moveRow(int from, int to)
{
    mcdriver::options *opt = model_->options();
    auto &regions = opt->Target.regions;

    if (regions.empty())
        return false;

    int n = regions.size();
    if (from < 0 || from >= n)
        return false;
    if (to < 0 || to >= n)
        return false;
    if (to == from)
        return false;

    target::region reg = regions[from];

    QModelIndex parent = regionsIndex_.parent();
    beginRemoveRows(parent, from, from);
    regions.erase(regions.begin() + from);
    endRemoveRows();

    // if (to > from) to = to-1;
    beginInsertRows(parent, to, to);
    regions.insert(regions.begin() + to, reg);
    endInsertRows();

    // fake setData just to let model_ know that
    // underlying data changed
    model_->setData(regionsIndex_, QVariant());

    return true;
}

void RegionsModel::resetModel()
{
    beginResetModel();
    endResetModel();
}

/*********************************************************/
RegionDelegate::RegionDelegate(QObject *parent) : QStyledItemDelegate(parent) { }
QWidget *RegionDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem & /* option */,
                                      const QModelIndex &index) const
{
    if (!index.isValid())
        return nullptr;

    int col = index.column();
    if (col < 0 || col >= index.model()->columnCount())
        return nullptr;

    const RegionsModel *rmodel = qobject_cast<const RegionsModel *>(index.model());
    if (!rmodel)
        return nullptr;

    QWidget *w = nullptr;
    switch (col) {
    case 0:
        w = new QLineEdit(parent);
        break;
    case 1: {
        QComboBox *cb = new QComboBox(parent);
        const mcdriver::options *opt = rmodel->model_->options();
        auto &materials = opt->Target.materials;
        for (int i = 0; i < materials.size(); ++i) {
            cb->addItem(QString::fromStdString(materials[i].id));
        }
        w = cb;
    } break;
    case 2:
    case 3: {
        Vector3dLineEdit *edt = new Vector3dLineEdit(0.f, 1.e6f, 3, parent);
        w = edt;
    } break;
    }

    return w;
}
void RegionDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if (!index.isValid())
        return;

    int col = index.column();
    if (col < 0 || col >= index.model()->columnCount())
        return;

    QVariant v = index.model()->data(index, Qt::EditRole);

    switch (col) {
    case 0: {
        QLineEdit *ledt = (QLineEdit *)editor;
        ledt->setText(v.toString());
    } break;
    case 1: {
        QComboBox *cb = (QComboBox *)(editor);
        cb->setCurrentText(v.toString());
    } break;
    case 2:
    case 3: {
        Vector3dLineEdit *edt = (Vector3dLineEdit *)(editor);
        edt->setValue(v.value<vector3>());
    } break;
    }
}
void RegionDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                  const QModelIndex &index) const
{
    if (!index.isValid())
        return;

    int col = index.column();
    if (col < 0 || col >= index.model()->columnCount())
        return;

    QVariant v;

    switch (col) {
    case 0: {
        QLineEdit *ledt = (QLineEdit *)editor;
        v = ledt->text();
    } break;
    case 1: {
        QComboBox *cb = (QComboBox *)(editor);
        v = cb->currentText();
    } break;
    case 2:
    case 3: {
        Vector3dLineEdit *edt = (Vector3dLineEdit *)(editor);
        v = QVariant::fromValue(edt->value());
    } break;
    }

    model->setData(index, v, Qt::EditRole);
}
void RegionDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                                          const QModelIndex & /* index */) const
{
    editor->setGeometry(option.rect);
}
/*****************************************************************************/
RegionsView::RegionsView(OptionsModel *m, QObject *parent)
    : model_(new RegionsModel(m, this)), delegate_(new RegionDelegate(this))
{
    btAdd = new QToolButton;
    btAdd->setIcon(QIcon(":/assets/ionicons/add-outline.svg"));
    btAdd->setToolTip("Add region");
    btRemove = new QToolButton;
    btRemove->setIcon(QIcon(":/assets/ionicons/remove-outline.svg"));
    btRemove->setToolTip("Remove region");
    btUp = new QToolButton;
    btUp->setIcon(QIcon(":/assets/ionicons/arrow-up-outline.svg"));
    btUp->setToolTip("Move region up");
    btDown = new QToolButton;
    btDown->setIcon(QIcon(":/assets/ionicons/arrow-down-outline.svg"));
    btDown->setToolTip("Move region down");

    connect(btAdd, &QToolButton::clicked, this, &RegionsView::addRegion);
    connect(btRemove, &QToolButton::clicked, this, &RegionsView::removeRegion);
    connect(btUp, &QToolButton::clicked, this, &RegionsView::moveRegionUp);
    connect(btDown, &QToolButton::clicked, this, &RegionsView::moveRegionDown);
    // btAdd->setEnabled(false);
    btRemove->setEnabled(false);
    btUp->setEnabled(false);
    btDown->setEnabled(false);

    tableView = new QTableView;
    tableView->setModel(model_);
    tableView->setItemDelegate(delegate_);
    QFontMetrics fm = tableView->fontMetrics();
    int char_w = fm.averageCharWidth();
    const int field_w[] = { 10, 10, 20, 20 };
    for (int col = 0; col < 4; ++col)
        tableView->setColumnWidth(col, char_w * field_w[col]);

    selectionModel = tableView->selectionModel();
    connect(selectionModel, &QItemSelectionModel::selectionChanged, this,
            &RegionsView::onSelectionChanged);

    QHBoxLayout *hbox = new QHBoxLayout;
    // hbox->addWidget(new QLabel("Regions "));
    QGridLayout *grid = new QGridLayout;
    grid->setSizeConstraint(QLayout::SetFixedSize);
    grid->addWidget(btAdd, 0, 0);
    grid->addWidget(btRemove, 0, 1);
    grid->addWidget(btUp, 0, 2);
    grid->addWidget(btDown, 0, 3);
    hbox->addLayout(grid);
    hbox->addStretch();

    //    QSize fsz(24,24);
    //    btAdd->setFixedSize(fsz);
    //    btRemove->setFixedSize(fsz);
    //    btUp->setFixedSize(fsz);
    //    btDown->setFixedSize(fsz);

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addLayout(hbox);
    vbox->addWidget(tableView);

    setLayout(vbox);
}

void RegionsView::revert()
{
    // tableView->setModel(0);
    // tableView->setModel(model_);
    // tableView->update(QModelIndex());
    model_->resetModel();
    disconnect(selectionModel, &QItemSelectionModel::selectionChanged, this,
               &RegionsView::onSelectionChanged);
    selectionModel = tableView->selectionModel();
    connect(selectionModel, &QItemSelectionModel::selectionChanged, this,
            &RegionsView::onSelectionChanged);
}

void RegionsView::addRegion()
{
    int r = model_->rowCount();
    model_->insertRows(r, 1, QModelIndex());
}

void RegionsView::removeRegion()
{
    int i = 0, n = model_->rowCount();
    while (!selectionModel->isRowSelected(i) && i < n)
        i++;
    if (i < n) {
        model_->removeRows(i, 1);
    }
}

void RegionsView::moveRegionUp()
{
    int i = 1, n = model_->rowCount();
    while (!selectionModel->isRowSelected(i) && i < n)
        i++;
    if (i < n) {
        model_->moveRow(i, i - 1);
        selectionModel->select(model_->index(i - 1, 0),
                               QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
        selectionModel->setCurrentIndex(model_->index(i - 1, 0), QItemSelectionModel::Select);
    }
}

void RegionsView::moveRegionDown()
{
    int n = model_->rowCount(), i = n - 2;
    while (!selectionModel->isRowSelected(i) && i >= 0)
        i--;
    if (i >= 0) {
        model_->moveRow(i, i + 1);
        selectionModel->select(model_->index(i + 1, 0),
                               QItemSelectionModel::Rows | QItemSelectionModel::ClearAndSelect);
        selectionModel->setCurrentIndex(model_->index(i + 1, 0), QItemSelectionModel::Select);
    }
}

void RegionsView::onSelectionChanged(const QItemSelection &selected, const QItemSelection &)
{
    bool st[3] = { false, false, false };

    int i = 0, n = model_->rowCount();
    for (int i = 0; i < n; ++i) {
        if (selectionModel->isRowSelected(i)) {
            st[0] = true;
            st[1] = i > 0;
            st[2] = i < n - 1;
            break;
        }
    }

    btRemove->setEnabled(st[0]);
    btUp->setEnabled(st[1]);
    btDown->setEnabled(st[2]);
}
