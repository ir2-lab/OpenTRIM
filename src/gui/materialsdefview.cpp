#include "materialsdefview.h"

#include "periodictable.h"
#include "periodictablewidget.h"
#include "floatlineedit.h"
#include "optionsmodel.h"
#include "qjsonarray.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QSpacerItem>

#include <QLabel>
#include <QToolButton>
#include <QDoubleSpinBox>
#include <QTableView>
#include <QInputDialog>
#include <QMessageBox>
#include <QFontMetrics>
#include <QItemSelectionModel>

MaterialsDefView::MaterialsDefView(OptionsModel *m, QWidget *parent)
    : QWidget{parent}, model_(m)
{
    QModelIndex i;
    i = model_->index("Target");
    i = model_->index("materials",1,i);
    materialsIndex_ = i;

    QVBoxLayout* vbox = new QVBoxLayout;

    QHBoxLayout* hbox = new QHBoxLayout;

    QFormLayout* flayout = new QFormLayout;

    cbMaterialID = new MyComboBox;
    cbMaterialID->setPlaceholderText("Material id");
    cbMaterialID->setMinimumContentsLength(15);

    connect(cbMaterialID, &MyComboBox::doubleClicked,
            this, &MaterialsDefView::editMaterialName);
    connect(cbMaterialID, &MyComboBox::currentTextChanged,
            this, &MaterialsDefView::updateSelectedMaterial);

    btAddMaterial = new QToolButton;
    btAddMaterial->setIcon(QIcon(":/icons/assets/ionicons/add-outline.svg"));
    btAddMaterial->setToolTip("Add Material");

    btDelMaterial = new QToolButton;
    btDelMaterial->setIcon(QIcon(":/icons/assets/ionicons/remove-outline.svg"));
    btDelMaterial->setToolTip("Remove Material");
    btDelMaterial->setEnabled(false);
    connect(btAddMaterial, &QToolButton::clicked, this, &MaterialsDefView::addMaterial);
    connect(btDelMaterial, &QToolButton::clicked, this, &MaterialsDefView::removeMaterial);

    sbDensity = new QDoubleSpinBox;
    sbDensity->setMinimum(0.001);
    sbDensity->setMaximum(100.0);
    sbDensity->setDecimals(4);
    connect(sbDensity, SIGNAL(valueChanged(double)),
            this, SLOT(setDensity(double)));

    hbox->addWidget(cbMaterialID);
    hbox->addWidget(btAddMaterial);
    hbox->addWidget(btDelMaterial);
    hbox->setSpacing(0);

    flayout->addRow("Material", hbox);
    flayout->addRow("Density (g/cm³)", sbDensity);

    hbox = new QHBoxLayout;
    hbox->addLayout(flayout);
    hbox->addStretch();

    vbox->addLayout(hbox);
    vbox->addSpacing(20);

    materialsView = new MaterialCompositionView(model_);

    vbox->addWidget(materialsView);
    setLayout(vbox);

}

void MaterialsDefView::addMaterial()
{
    bool ok;
    QString id = QInputDialog::getText(this, tr("Add Material"),
                                         tr("New Material id"), QLineEdit::Normal,
                                         QString("Material #%1").arg(cbMaterialID->count()+1),
                                         &ok);
    if (ok && !id.isEmpty()) {
        auto& materials = model_->options()->Target.materials;
        material::material_desc_t newMaterial;
        newMaterial.id = id.toStdString();
        newMaterial.density = 1.0f;
        QStringList keys;
        keys << "Z" << "M" << "X" << "Ed" << "El" << "Es" << "Er";
        materials.push_back(newMaterial);
        setWidgetData(); // widgets updated
        cbMaterialID->setCurrentText(id);
        // fake setData just to let model_ know that
        // underlying data changed
        model_->setData(materialsIndex_,QVariant());
    }
}

void MaterialsDefView::editMaterialName()
{
    if (cbMaterialID->count()==0) return;

    int i = cbMaterialID->currentIndex();

    bool ok;
    QString id = QInputDialog::getText(this, tr("Edit Material id"),
                                         tr("Enter the new id"), QLineEdit::Normal,
                                         cbMaterialID->currentText(),
                                         &ok);
    if (ok && !id.isEmpty()) {
        cbMaterialID->setItemText(i,id);
        material::material_desc_t& m = model_->options()->Target.materials[i];
        m.id = id.toStdString();
        // fake setData just to let model_ know that
        // underlying data changed
        model_->setData(materialsIndex_,QVariant());
    }
    setValueData(); // update material name
}

void MaterialsDefView::removeMaterial()
{
    QString id = cbMaterialID->currentText();
    int i = cbMaterialID->currentIndex();
    if (id.isEmpty()) return;
    QMessageBox::StandardButton ret = QMessageBox::warning(this,"Remove Material",
                         QString("%1 is being removed.\nClick OK to proceed.").arg(id),
                         QMessageBox::Ok | QMessageBox::Cancel);
    if (ret == QMessageBox::Ok) {
        auto& materials = model_->options()->Target.materials;
        materials.erase(materials.begin()+i);
        setWidgetData(); // widgets updated
        // fake setData just to let model_ know that
        // underlying data changed
        model_->setData(materialsIndex_,QVariant());
    }
}
// from options to widgets
void MaterialsDefView::setWidgetData()
{
    auto& materials = model_->options()->Target.materials;

    int i = cbMaterialID->currentIndex();
    cbMaterialID->blockSignals(true);

    cbMaterialID->clear();
    sbDensity->clear();
    materialsView->setMaterialIdx();

    if (materials.empty()) {
        cbMaterialID->blockSignals(false);
        return;
    }

    // copy materials to combo box
    int n = materials.size();
    for(int k=0; k<n; ++k) {
        cbMaterialID->addItem(QString::fromStdString(materials[k].id));
    }

    // update selection if out of bounds
    if (i<0) i=0;
    else if (i>=n) i=n-1;

    // set data to selected material
    cbMaterialID->setCurrentIndex(i);
    sbDensity->blockSignals(true);
    sbDensity->setValue(materials[i].density);
    sbDensity->blockSignals(false);
    materialsView->setMaterialIdx(i);

    cbMaterialID->blockSignals(false);
}
void MaterialsDefView::setValueData()
{

}

void MaterialsDefView::updateSelectedMaterial()
{
    int i = cbMaterialID->currentIndex();
    if (i<0) { // no selection
        sbDensity->clear();
        materialsView->setMaterialIdx();
    } else {
        const material::material_desc_t& mat = model_->options()->Target.materials[i];
        sbDensity->blockSignals(true);
        sbDensity->setValue(mat.density);
        sbDensity->blockSignals(false);
        materialsView->setMaterialIdx(i);
    }
    btDelMaterial->setEnabled(i>=0);
    return;
}
void MaterialsDefView::setDensity(double v)
{
    auto& materials = model_->options()->Target.materials;

    if (materials.empty()) return;

    int i = cbMaterialID->currentIndex();
    if (i<0) return;

    QString matid = cbMaterialID->currentText();
    material::material_desc_t& mat = materials[i];
    mat.density = v;
    // fake setData just to let model_ know that
    // underlying data changed
    model_->setData(materialsIndex_,QVariant());
}
/*****************************************************/
MaterialCompositionModel::MaterialCompositionModel(OptionsModel *m, QObject *parent)
    : QAbstractTableModel(parent), model_(m)
{
    QModelIndex i;
    i = model_->index("Target");
    i = model_->index("materials",1,i);
    materialsIndex_ = i;
}
void MaterialCompositionModel::setMaterialIdx(int i)
{
    beginResetModel();
    materialIdx_ = i;
    endResetModel();
}
const material::material_desc_t *MaterialCompositionModel::getMaterial() const
{
    auto& materials = model_->options()->Target.materials;
    if (materials.empty() || materialIdx_<0 || materialIdx_>=materials.size())
        return nullptr;
    return &materials[materialIdx_];
}

material::material_desc_t *MaterialCompositionModel::getMaterial()
{
    auto& materials = model_->options()->Target.materials;
    if (materials.empty() || materialIdx_<0 || materialIdx_>=materials.size())
        return nullptr;
    return &materials[materialIdx_];

}
void MaterialCompositionModel::setMaterial(const material::material_desc_t &mat)
{
    auto& materials = model_->options()->Target.materials;
    if (materials.empty() || materialIdx_<0 || materialIdx_>=materials.size())
        return;
    materials[materialIdx_] = mat;
}
int MaterialCompositionModel::rowCount(const QModelIndex & /* parent */) const
{
    auto mat = getMaterial();
    if (mat==nullptr) return 0;
    return mat->Z.size();
}
int MaterialCompositionModel::columnCount(const QModelIndex & /* parent */) const
{
    return col_names_.size(); // Z, M, X, Ed, El, Es, Er
}
QVariant MaterialCompositionModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    int i = index.row(), j = index.column();
    if (j<0 || j>=columnCount()) return QVariant();

    auto mat = getMaterial();
    if (mat==nullptr) return QVariant();

    if (i<0 || i>=mat->Z.size()) return QVariant();

    switch (j) {
    case 0: return PeriodicTable::at(mat->Z[i]).symbol();
    case 1: return mat->M[i];
    case 2: return mat->X[i];
    case 3: return mat->Ed[i];
    case 4: return mat->El[i];
    case 5: return mat->Es[i];
    case 6: return mat->Er[i];
    default: return QVariant();
    }
}
QVariant MaterialCompositionModel::headerData(int c,
                                Qt::Orientation o,
                                int role) const
{
    if (role == Qt::DisplayRole && o == Qt::Horizontal)
        return col_labels_[c];
    if (role == Qt::DisplayRole && o == Qt::Vertical)
        return c+1;
    return QVariant();
}

Qt::ItemFlags MaterialCompositionModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    Qt::ItemFlags f = QAbstractItemModel::flags(index);
    return index.column() ? Qt::ItemIsEditable | f : f;
}

bool MaterialCompositionModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    int i = index.row(), j = index.column();
    if (j<0 || j>=columnCount()) return false;

    auto mat = getMaterial();
    if (mat==nullptr) return false;

    if (i<0 || i>=mat->Z.size()) return false;

    switch (j) {
    case 0: mat->Z[i] = value.toInt(); break;
    case 1: mat->M[i] = value.toFloat(); break;
    case 2: mat->X[i] = value.toFloat(); break;
    case 3: mat->Ed[i] = value.toFloat(); break;
    case 4: mat->El[i] = value.toFloat(); break;
    case 5: mat->Es[i] = value.toFloat(); break;
    case 6: mat->Er[i] = value.toFloat(); break;
    default: ;
    }

    // fake setData just to let model_ know that
    // underlying data changed
    model_->setData(materialsIndex_,QVariant());

    return true;
}


bool MaterialCompositionModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    assert(rows == 1);
    assert(position == rowCount());

    auto mat = getMaterial();
    if (mat==nullptr) return false;

    beginInsertRows(parent, position, position);
    mat->Z.push_back(0);
    mat->M.push_back(0.f);
    mat->X.push_back(0.f);
    mat->Ed.push_back(0.f);
    mat->Es.push_back(0.f);
    mat->El.push_back(0.f);
    mat->Er.push_back(0.f);
    endInsertRows();

    // fake setData just to let model_ know that
    // underlying data changed
    model_->setData(materialsIndex_,QVariant());

    return true;
}

bool MaterialCompositionModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    assert(rows == 1);

    auto mat = getMaterial();
    if (mat==nullptr) return false;

    if (position>=mat->Z.size()) return false;

    beginRemoveRows(parent, position, position);
    mat->Z.erase(mat->Z.begin()+position);
    mat->M.erase(mat->M.begin()+position);
    mat->X.erase(mat->X.begin()+position);
    mat->Ed.erase(mat->Ed.begin()+position);
    mat->El.erase(mat->El.begin()+position);
    mat->Es.erase(mat->Es.begin()+position);
    mat->Er.erase(mat->Er.begin()+position);
    endRemoveRows();

    // fake setData just to let model_ know that
    // underlying data changed
    model_->setData(materialsIndex_,QVariant());

    return true;
}

/*********************************************************/
MaterialCompositionDelegate::MaterialCompositionDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}
QWidget *MaterialCompositionDelegate::createEditor(QWidget *parent,
                                       const QStyleOptionViewItem &/* option */,
                                       const QModelIndex &index) const
{
    FloatLineEdit* editor = new FloatLineEdit(0.f,100.f,
                                              index.column()==1 ? 10 : 3,
                                              parent);
    return editor;
}
void MaterialCompositionDelegate::setEditorData(QWidget *editor,
                                    const QModelIndex &index) const
{
    double value = index.model()->data(index, Qt::DisplayRole).toDouble();

    FloatLineEdit* fedt = static_cast<FloatLineEdit*>(editor);
    fedt->setValue(value);
}
void MaterialCompositionDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                   const QModelIndex &index) const
{
    FloatLineEdit* fedt = static_cast<FloatLineEdit*>(editor);
    model->setData(index, fedt->value(), Qt::EditRole);
}
void MaterialCompositionDelegate::updateEditorGeometry(QWidget *editor,
                                           const QStyleOptionViewItem &option,
                                           const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}
/*****************************************************************************/
MaterialCompositionView::MaterialCompositionView(OptionsModel *m, QObject *parent) :
    model_(new MaterialCompositionModel(m,this)),
    delegate_(new MaterialCompositionDelegate(this))
{
    btAdd = new QToolButton;
    btAdd->setIcon(QIcon(":/icons/assets/ionicons/add-outline.svg"));
    btAdd->setToolTip("Add Element");
    btRemove = new QToolButton;
    btRemove->setIcon(QIcon(":/icons/assets/ionicons/remove-outline.svg"));
    btRemove->setToolTip("Remove Element");
    connect(btAdd, &QToolButton::clicked, this, &MaterialCompositionView::addElement);
    connect(btRemove, &QToolButton::clicked, this, &MaterialCompositionView::removeElement);
    btAdd->setEnabled(false);
    btRemove->setEnabled(false);

    QTableView* view = new QTableView;
    view->setModel(model_);
    view->setItemDelegate(delegate_);
    QFontMetrics fm = view->fontMetrics();
    int sz = fm.averageCharWidth();
    const int W[] = {2, 8, 6, 6, 6, 6, 6};
    for(int col=0; col<7; ++col)
        view->setColumnWidth(col, sz*W[col]);

    selectionModel = view->selectionModel();
    connect(selectionModel, &QItemSelectionModel::selectionChanged,
            this, &MaterialCompositionView::onSelectionChanged);

    QHBoxLayout* hbox = new QHBoxLayout;
    hbox->addWidget(new QLabel("Composition "));
    hbox->addWidget(btAdd);
    hbox->addWidget(btRemove);
    hbox->addStretch();

    QVBoxLayout* vbox = new QVBoxLayout;
    vbox->addLayout(hbox);
    vbox->addWidget(view);

    setLayout(vbox);

}

void MaterialCompositionView::addElement()
{
    PeriodicTableDialog dlg;
    if(dlg.exec()==QDialog::Accepted) {
        int Z = dlg.selectedZ();
        double mass = dlg.selectedMass();
        QString symb = dlg.selectedIonSymbol();

        int r = model_->rowCount();
        bool ret = model_->insertRows(r,1,QModelIndex());
        if (ret) {
            int i=0;
            model_->setData(model_->index(r,i++),Z);
            model_->setData(model_->index(r,i++),mass);
            model_->setData(model_->index(r,i++),1.);
            model_->setData(model_->index(r,i++),40.);
            model_->setData(model_->index(r,i++),3.);
            model_->setData(model_->index(r,i++),3.);
            model_->setData(model_->index(r,i++),40.);
        }
    }
}

void MaterialCompositionView::removeElement()
{
    int i=0, n = model_->rowCount();
    while (!selectionModel->isRowSelected(i) && i<n) i++;
    if (i<n) {
        model_->removeRows(i,1);
    }
}

void MaterialCompositionView::setMaterialIdx(int i)
{
    model_->setMaterialIdx(i);
    bool empty = model_->getMaterial() == nullptr;
    btAdd->setEnabled(!empty);
}

void MaterialCompositionView::onSelectionChanged(const QItemSelection &selected, const QItemSelection &)
{
    int i=0, n = model_->rowCount();
    for(int i=0; i<n; ++i) {
        if (selectionModel->isRowSelected(i)) {
            btRemove->setEnabled(true);
            return;
        }
    }
    btRemove->setEnabled(false);
}
