#include "materialsdefview.h"

#include "periodic_table.h"
#include "periodictablewidget.h"
#include "optionsmodel.h"
#include "optionsview.h"
#include "mainui.h"
#include "materialdatabasedialog.h"
#include "helppanel.h"

#include <QtVectorEdit/qvectoredit.h>
#include <QtVectorEdit/qnumberedit.h>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QSpacerItem>

#include <QLabel>
#include <QToolButton>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QTableView>
#include <QInputDialog>
#include <QMessageBox>
#include <QFontMetrics>
#include <QItemSelectionModel>
#include <QColorDialog>
#include <QHeaderView>

void MaterialsDefView::setBtMatColor(const QColor &clr)
{
    if (!btMatColor)
        return;
    QPixmap px(btMatColor->iconSize());
    px.fill(clr);
    btMatColor->setIcon(px);
}

MaterialsDefView::MaterialsDefView(OptionsView *v, QWidget *parent)
    : QWidget{ parent }, model_(v->mainui->optionsModel)
{
    QModelIndex i;
    i = model_->index("Target");
    i = model_->index("materials", 1, i);
    materialsIndex_ = i;

    QVBoxLayout *vbox = new QVBoxLayout;

    QHBoxLayout *hbox = new QHBoxLayout;

    QFormLayout *flayout = new QFormLayout;

    cbMaterialID = new MyComboBox;
    cbMaterialID->setPlaceholderText("Material id");
    cbMaterialID->setMinimumContentsLength(15);

    connect(cbMaterialID, &MyComboBox::doubleClicked, this, &MaterialsDefView::editMaterialName);
    connect(cbMaterialID, &MyComboBox::currentTextChanged, this,
            &MaterialsDefView::updateSelectedMaterial);
    v->helpPanel->addStaticHelp(cbMaterialID, "/Target/materials/0/id");

    btDbMaterial = new QToolButton;
    btDbMaterial->setIcon(QIcon(":/assets/lucide/database-search.svg"));
    btDbMaterial->setToolTip("Add material from database");

    btAddMaterial = new QToolButton;
    btAddMaterial->setIcon(QIcon(":/assets/ionicons/add-outline.svg"));
    btAddMaterial->setToolTip("Add new material");

    btDelMaterial = new QToolButton;
    btDelMaterial->setIcon(QIcon(":/assets/ionicons/remove-outline.svg"));
    btDelMaterial->setToolTip("Remove material");
    btDelMaterial->setEnabled(false);

    btEdtMaterial = new QToolButton;
    btEdtMaterial->setIcon(QIcon(":/assets/ionicons/create-outline.svg"));
    btEdtMaterial->setToolTip("Rename material");
    btEdtMaterial->setEnabled(false);

    connect(btDbMaterial, &QToolButton::clicked, this, &MaterialsDefView::addMaterialFromDatabase);
    connect(btAddMaterial, &QToolButton::clicked, this, &MaterialsDefView::addMaterial);
    connect(btDelMaterial, &QToolButton::clicked, this, &MaterialsDefView::removeMaterial);
    connect(btEdtMaterial, &QToolButton::clicked, this, &MaterialsDefView::editMaterialName);

    sbDensity = new QDoubleSpinBox;
    sbDensity->setMinimum(0.001);
    sbDensity->setMaximum(100.0);
    sbDensity->setDecimals(4);
    connect(sbDensity, SIGNAL(valueChanged(double)), this, SLOT(setDensity(double)));
    v->helpPanel->addStaticHelp(sbDensity, "/Target/materials/0/density");

    btMatColor = new QToolButton;
    btMatColor->setText(" Select Display Color ");
    btMatColor->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    setBtMatColor(Qt::darkBlue);
    btMatColor->setToolTip("Select display color for this material");
    connect(btMatColor, &QToolButton::pressed, this, &MaterialsDefView::selectColor);
    v->helpPanel->addStaticHelp(btMatColor, "/Target/materials/0/color");

    hbox->addWidget(cbMaterialID);
    hbox->addWidget(btDbMaterial);
    hbox->addWidget(btAddMaterial);
    hbox->addWidget(btDelMaterial);
    hbox->addWidget(btEdtMaterial);
    hbox->setSpacing(0);

    flayout->addRow("Material", hbox);
    flayout->addRow("Density (g/cm³)", sbDensity);
    flayout->addRow("Color", btMatColor);
    v->helpPanel->addStaticHelp(flayout->labelForField(hbox), "/Target/materials/0/id");
    v->helpPanel->addStaticHelp(flayout->labelForField(sbDensity), "/Target/materials/0/density");
    v->helpPanel->addStaticHelp(flayout->labelForField(btMatColor), "/Target/materials/0/color");

    hbox = new QHBoxLayout;
    hbox->addLayout(flayout);
    hbox->addStretch();

    vbox->addLayout(hbox);
    vbox->addSpacing(12);

    materialsView = new MaterialCompositionView(v);

    vbox->addWidget(materialsView);
    setLayout(vbox);
}

void MaterialsDefView::addMaterialFromDatabase()
{
    QStringList existingMaterialIds;
    for (const auto &material : model_->options()->Target.materials)
        existingMaterialIds << QString::fromStdString(material.id);

    MaterialDatabaseDialog dlg(existingMaterialIds, this);
    if (dlg.exec() == QDialog::Accepted) {
        const auto &md = dlg.getSelectedMaterial();
        assert(!md.id.empty());
        mcconfig cfg = *model_->options();
        cfg.Target.materials.push_back(md);
        model_->setOptions(cfg);
        setWidgetData();
        cbMaterialID->setCurrentText(md.id.c_str());
        emit materialsChanged();
    }
}

void MaterialsDefView::addMaterial()
{
    QInputDialog dlg(this);
    dlg.setWindowFlags(dlg.windowFlags() & ~Qt::WindowContextHelpButtonHint
                       & ~Qt::WindowMinMaxButtonsHint);
    QString wTitle = tr("OpenTRIM - Add Material");
    dlg.setWindowTitle(wTitle);
    dlg.setLabelText(tr("New Material id:"));
    dlg.setTextValue(QString("Material%1").arg(cbMaterialID->count() + 1));
    QFontMetrics fm = fontMetrics();
    QRect rect = fm.boundingRect(wTitle);
    int minW = rect.width() * 2;
    if (auto *edit = dlg.findChild<QLineEdit *>()) {
        edit->setMinimumWidth(minW);
    }
    if (dlg.exec() == QDialog::Rejected)
        return;

    QString id = dlg.textValue();
    if (id.isEmpty())
        return;

    auto &materials = model_->options()->Target.materials;
    material::material_desc_t newMaterial;
    newMaterial.id = id.toStdString();
    materials.push_back(newMaterial);
    setWidgetData(); // widgets updated
    cbMaterialID->setCurrentText(id);
    // let model_ know that underlying data changed
    model_->dataChanged(materialsIndex_, materialsIndex_);

    emit materialsChanged();
}

void MaterialsDefView::editMaterialName()
{
    if (cbMaterialID->count() == 0)
        return;

    QInputDialog dlg(this);
    dlg.setWindowFlags(dlg.windowFlags() & ~Qt::WindowContextHelpButtonHint
                       & ~Qt::WindowMinMaxButtonsHint);
    QString wTitle = tr("OpenTRIM - Edit Material id");
    dlg.setWindowTitle(wTitle);
    dlg.setLabelText(tr("Enter the new id:"));
    dlg.setTextValue(cbMaterialID->currentText());
    QFontMetrics fm = fontMetrics();
    QRect rect = fm.boundingRect(wTitle);
    int minW = rect.width() * 2;
    if (auto *edit = dlg.findChild<QLineEdit *>()) {
        edit->setMinimumWidth(minW);
    }
    if (dlg.exec() == QDialog::Rejected)
        return;

    QString id = dlg.textValue();
    if (id.isEmpty())
        return;

    int i = cbMaterialID->currentIndex();

    cbMaterialID->setItemText(i, id);
    material::material_desc_t &m = model_->options()->Target.materials[i];
    m.id = id.toStdString();

    // let model_ know that underlying data changed
    model_->dataChanged(materialsIndex_, materialsIndex_);
}

void MaterialsDefView::removeMaterial()
{
    QString id = cbMaterialID->currentText();
    int i = cbMaterialID->currentIndex();
    if (id.isEmpty())
        return;
    QMessageBox::StandardButton ret = QMessageBox::warning(
            this, "Remove Material", QString("%1 is being removed.\nClick OK to proceed.").arg(id),
            QMessageBox::Ok | QMessageBox::Cancel);
    if (ret == QMessageBox::Ok) {
        auto &materials = model_->options()->Target.materials;
        materials.erase(materials.begin() + i);
        setWidgetData(); // widgets updated
        // let model_ know that underlying data changed
        model_->dataChanged(materialsIndex_, materialsIndex_);

        emit materialsChanged();
    }
}
// from options to widgets
void MaterialsDefView::setWidgetData()
{
    auto &materials = model_->options()->Target.materials;

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
    for (int k = 0; k < n; ++k) {
        cbMaterialID->addItem(QString::fromStdString(materials[k].id));
    }

    // update selection if out of bounds
    if (n) {
        if (i < 0)
            i = 0;
        else if (i >= n)
            i = n - 1;
    } else
        i = -1;

    // set data to selected material
    cbMaterialID->setCurrentIndex(i);
    btDelMaterial->setEnabled(i >= 0);
    btEdtMaterial->setEnabled(i >= 0);
    sbDensity->blockSignals(true);
    sbDensity->setValue(materials[i].density);
    sbDensity->blockSignals(false);
    setBtMatColor(QColor(materials[i].color.c_str()));
    materialsView->setMaterialIdx(i);

    cbMaterialID->blockSignals(false);
}

void MaterialsDefView::updateSelectedMaterial()
{
    int i = cbMaterialID->currentIndex();
    if (i < 0) { // no selection
        sbDensity->clear();
        materialsView->setMaterialIdx();
    } else {
        const material::material_desc_t &mat = model_->options()->Target.materials[i];
        sbDensity->blockSignals(true);
        sbDensity->setValue(mat.density);
        sbDensity->blockSignals(false);
        setBtMatColor(QColor(mat.color.c_str()));
        materialsView->setMaterialIdx(i);
    }
    btDelMaterial->setEnabled(i >= 0);
    btEdtMaterial->setEnabled(i >= 0);
    return;
}
void MaterialsDefView::setDensity(double v)
{
    auto &materials = model_->options()->Target.materials;

    if (materials.empty())
        return;

    int i = cbMaterialID->currentIndex();
    if (i < 0)
        return;

    QString matid = cbMaterialID->currentText();
    material::material_desc_t &mat = materials[i];
    mat.density = v;
    // let model_ know that underlying data changed
    model_->dataChanged(materialsIndex_, materialsIndex_);
}

void MaterialsDefView::selectColor()
{
    auto &materials = model_->options()->Target.materials;

    if (materials.empty())
        return;

    int i = cbMaterialID->currentIndex();
    if (i < 0)
        return;

    material::material_desc_t &mat = materials[i];

    QColor clr = QColorDialog::getColor(QColor(mat.color.c_str()), this,
                                        QString("Select display color for %1").arg(mat.id.c_str()),
                                        QColorDialog::ShowAlphaChannel);
    if (clr.isValid()) {
        mat.color = clr.name(QColor::HexArgb).toStdString();
        setBtMatColor(clr);
        // let model_ know that underlying data changed
        model_->dataChanged(materialsIndex_, materialsIndex_);

        emit materialsChanged();
    }
}

/*****************************************************/
MaterialCompositionModel::MaterialCompositionModel(OptionsModel *m, QObject *parent)
    : QAbstractTableModel(parent), model_(m)
{
    QModelIndex i;
    i = model_->index("Target");
    i = model_->index("materials", 1, i);
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
    auto &materials = model_->options()->Target.materials;
    if (materials.empty() || materialIdx_ < 0 || materialIdx_ >= materials.size())
        return nullptr;
    return &materials[materialIdx_];
}

material::material_desc_t *MaterialCompositionModel::getMaterial()
{
    auto &materials = model_->options()->Target.materials;
    if (materials.empty() || materialIdx_ < 0 || materialIdx_ >= materials.size())
        return nullptr;
    return &materials[materialIdx_];
}
void MaterialCompositionModel::setMaterial(const material::material_desc_t &mat)
{
    auto &materials = model_->options()->Target.materials;
    if (materials.empty() || materialIdx_ < 0 || materialIdx_ >= materials.size())
        return;
    materials[materialIdx_] = mat;
}
int MaterialCompositionModel::rowCount(const QModelIndex & /* parent */) const
{
    auto mat = getMaterial();
    if (mat == nullptr)
        return 0;
    return mat->composition.size();
}
int MaterialCompositionModel::columnCount(const QModelIndex & /* parent */) const
{
    return col_labels_.size(); // symbol, M, X, Ed, El, Es, Er, Rc
}
QVariant MaterialCompositionModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    int i = index.row(), j = index.column();
    if (j < 0 || j >= columnCount())
        return QVariant();

    auto mat = getMaterial();
    if (mat == nullptr)
        return QVariant();

    if (i < 0 || i >= mat->composition.size())
        return QVariant();

    auto &at = mat->composition[i];

    switch (j) {
    case 0:
        return at.element.symbol.c_str();
    case 1:
        return at.element.atomic_mass;
    case 2:
        return at.X;
    case 3:
        return at.Ed;
    case 4:
        return at.El;
    case 5:
        return at.Es;
    case 6:
        return at.Er;
    case 7:
        return at.Rc;
    default:
        return QVariant();
    }
}
QVariant MaterialCompositionModel::headerData(int c, Qt::Orientation o, int role) const
{
    if (role == Qt::DisplayRole && o == Qt::Horizontal)
        return col_labels_[c];
    if (role == Qt::ToolTipRole && o == Qt::Horizontal)
        return col_tooltip_[c];
    if (role == Qt::DisplayRole && o == Qt::Vertical)
        return c + 1;
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
    if (j < 0 || j >= columnCount())
        return false;

    auto mat = getMaterial();
    if (mat == nullptr)
        return false;

    if (i < 0 || i >= mat->composition.size())
        return false;

    auto &at = mat->composition[i];

    switch (j) {
    case 0:
        at.element.symbol = value.toString().toStdString();
        at.element.atomic_number = periodic_table::at(at.element.symbol).Z;
        break;
    case 1:
        at.element.atomic_mass = value.toFloat();
        break;
    case 2:
        at.X = value.toFloat();
        break;
    case 3:
        at.Ed = value.toFloat();
        break;
    case 4:
        at.El = value.toFloat();
        break;
    case 5:
        at.Es = value.toFloat();
        break;
    case 6:
        at.Er = value.toFloat();
        break;
    case 7:
        at.Rc = value.toFloat();
        break;
    default:;
    }

    // let model_ know that underlying data changed
    model_->dataChanged(materialsIndex_, materialsIndex_);

    return true;
}

bool MaterialCompositionModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    assert(rows == 1);
    assert(position == rowCount());

    auto mat = getMaterial();
    if (mat == nullptr)
        return false;

    beginInsertRows(parent, position, position);
    mat->composition.push_back(atom::parameters());
    endInsertRows();

    // let model_ know that underlying data changed
    model_->dataChanged(materialsIndex_, materialsIndex_);

    return true;
}

bool MaterialCompositionModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    assert(rows == 1);

    auto mat = getMaterial();
    if (mat == nullptr)
        return false;

    if (position >= mat->composition.size())
        return false;

    beginRemoveRows(parent, position, position);
    mat->composition.erase(mat->composition.begin() + position);
    endRemoveRows();

    // let model_ know that underlying data changed
    model_->dataChanged(materialsIndex_, materialsIndex_);

    return true;
}

/*********************************************************/
MaterialCompositionDelegate::MaterialCompositionDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}
QWidget *MaterialCompositionDelegate::createEditor(QWidget *parent,
                                                   const QStyleOptionViewItem & /* option */,
                                                   const QModelIndex &index) const
{
    QNumberEdit *editor = new QNumberEdit(parent);
    editor->setElementType(QNumberEdit::Real);
    editor->setMinimum(0.0f);
    editor->setMaximum(100.0f);
    editor->setPrecision(index.column() == 1 ? 10 : 3);
    return editor;
}
void MaterialCompositionDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    double value = index.model()->data(index, Qt::DisplayRole).toDouble();

    QNumberEdit *fedt = static_cast<QNumberEdit *>(editor);
    fedt->setValue(value);
}
void MaterialCompositionDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                               const QModelIndex &index) const
{
    QNumberEdit *fedt = static_cast<QNumberEdit *>(editor);
    model->setData(index, fedt->value(), Qt::EditRole);
}
void MaterialCompositionDelegate::updateEditorGeometry(QWidget *editor,
                                                       const QStyleOptionViewItem &option,
                                                       const QModelIndex & /* index */) const
{
    editor->setGeometry(option.rect);
}
/*****************************************************************************/
MaterialCompositionView::MaterialCompositionView(OptionsView *v, QObject *parent)
    : model_(new MaterialCompositionModel(v->mainui->optionsModel, this)),
      delegate_(new MaterialCompositionDelegate(this))
{
    btAdd = new QToolButton;
    btAdd->setIcon(QIcon(":/assets/ionicons/add-outline.svg"));
    btAdd->setToolTip("Add Element");
    btRemove = new QToolButton;
    btRemove->setIcon(QIcon(":/assets/ionicons/remove-outline.svg"));
    btRemove->setToolTip("Remove Element");
    connect(btAdd, &QToolButton::clicked, this, &MaterialCompositionView::addElement);
    connect(btRemove, &QToolButton::clicked, this, &MaterialCompositionView::removeElement);
    btAdd->setEnabled(false);
    btRemove->setEnabled(false);

    QLabel *compositionLabel = new QLabel("Composition ");
    v->helpPanel->addStaticHelp(compositionLabel, "/Target/materials/0/composition");

    QTableView *view = new QTableView;
    view->setModel(model_);
    view->setItemDelegate(delegate_);

    QFontMetrics fm = view->fontMetrics();
    int minWidth = fm.horizontalAdvance("000000") + 20; // padding for margins
    view->horizontalHeader()->setMinimumSectionSize(minWidth);
    view->horizontalHeader()->setDefaultSectionSize(minWidth);

    selectionModel = view->selectionModel();
    connect(selectionModel, &QItemSelectionModel::selectionChanged, this,
            &MaterialCompositionView::onSelectionChanged);
    v->helpPanel->addStaticHelp(
            view,
            { "/Target/materials/0/composition/0/element/symbol",
              "/Target/materials/0/composition/0/element/atomic_mass",
              "/Target/materials/0/composition/0/X", "/Target/materials/0/composition/0/Ed",
              "/Target/materials/0/composition/0/El", "/Target/materials/0/composition/0/Es",
              "/Target/materials/0/composition/0/Er", "/Target/materials/0/composition/0/Rc" });

    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->addWidget(compositionLabel);
    hbox->addWidget(btAdd);
    hbox->addWidget(btRemove);
    hbox->addStretch();

    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addLayout(hbox);
    vbox->addWidget(view);

    vbox->setContentsMargins(0, 0, 0, 0);

    setLayout(vbox);
}

void MaterialCompositionView::addElement()
{
    PeriodicTableDialog dlg;
    if (dlg.exec() == QDialog::Accepted) {
        int Z = dlg.selectedZ();
        double mass = dlg.selectedMass();
        QString symb = dlg.selectedIonSymbol();

        int r = model_->rowCount();
        bool ret = model_->insertRows(r, 1, QModelIndex());
        if (ret) {
            int i = 0;
            model_->setData(model_->index(r, i++), symb);
            model_->setData(model_->index(r, i++), mass);
            model_->setData(model_->index(r, i++), 1.);
            model_->setData(model_->index(r, i++), 40.);
            model_->setData(model_->index(r, i++), 3.);
            model_->setData(model_->index(r, i++), 3.);
            model_->setData(model_->index(r, i++), 40.);
            model_->setData(model_->index(r, i++), 0.946);
        }
    }
}

void MaterialCompositionView::removeElement()
{
    int i = 0, n = model_->rowCount();
    while (!selectionModel->isRowSelected(i) && i < n)
        i++;
    if (i < n) {
        model_->removeRows(i, 1);
    }
}

void MaterialCompositionView::setMaterialIdx(int i)
{
    model_->setMaterialIdx(i);
    bool empty = model_->getMaterial() == nullptr;
    btAdd->setEnabled(!empty);
}

void MaterialCompositionView::onSelectionChanged(const QItemSelection &selected,
                                                 const QItemSelection &)
{
    int i = 0, n = model_->rowCount();
    for (int i = 0; i < n; ++i) {
        if (selectionModel->isRowSelected(i)) {
            btRemove->setEnabled(true);
            return;
        }
    }
    btRemove->setEnabled(false);
}
