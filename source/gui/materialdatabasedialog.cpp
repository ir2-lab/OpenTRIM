#include "materialdatabasedialog.h"

#include "json_defs_p.h"
#include "periodic_table.h"

#include <QDialogButtonBox>
#include <QFile>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QStringList>
#include <QTableWidget>
#include <QVBoxLayout>

namespace {
const char *kDbPath = ":/assets/data/materials_database.json";

bool containsCaseInsensitive(const QString &haystack, const QString &needle)
{
    if (needle.trimmed().isEmpty())
        return true;
    return haystack.contains(needle, Qt::CaseInsensitive);
}

QString joinMaterialElements(const material::material_desc_t &mat)
{
    QStringList symbols;
    for (const auto &a : mat.composition)
        symbols << QString::fromStdString(a.element.symbol);
    symbols.removeDuplicates();
    return symbols.join(", ");
}

} // namespace

MaterialDatabaseDialog::MaterialDatabaseDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Material Database");
    resize(900, 520);

    QVBoxLayout *root = new QVBoxLayout(this);

    QHBoxLayout *filters = new QHBoxLayout;
    filters->addWidget(new QLabel("Name filter:"));
    nameFilter_ = new QLineEdit;
    nameFilter_->setPlaceholderText("iron");
    filters->addWidget(nameFilter_, 1);
    filters->addSpacing(10);
    filters->addWidget(new QLabel("Element filter:"));
    elementFilter_ = new QLineEdit;
    elementFilter_->setPlaceholderText("Fe");
    filters->addWidget(elementFilter_);
    root->addLayout(filters);

    QHBoxLayout *content = new QHBoxLayout;
    listWidget_ = new QListWidget;
    listWidget_->setMinimumWidth(320);
    content->addWidget(listWidget_, 1);

    QVBoxLayout *preview = new QVBoxLayout;
    previewTitle_ = new QLabel("Preview");
    densityLabel_ = new QLabel("Density:");
    elementsLabel_ = new QLabel("Elements:");

    preview->addWidget(previewTitle_);
    preview->addWidget(densityLabel_);
    preview->addWidget(elementsLabel_);

    compositionTable_ = new QTableWidget;
    compositionTable_->setColumnCount(7);
    compositionTable_->setHorizontalHeaderLabels(
            { "Symbol", "X", "Ed", "El", "Es", "Er", "Rc" });
    compositionTable_->verticalHeader()->setVisible(false);
    compositionTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    compositionTable_->setSelectionMode(QAbstractItemView::NoSelection);
    compositionTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    preview->addWidget(compositionTable_, 1);

    content->addLayout(preview, 2);
    root->addLayout(content, 1);

    QDialogButtonBox *box = new QDialogButtonBox(QDialogButtonBox::Cancel);
    acceptButton_ = box->addButton("Add to simulation", QDialogButtonBox::AcceptRole);
    acceptButton_->setEnabled(false);
    connect(box, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(box, &QDialogButtonBox::rejected, this, &QDialog::reject);
    root->addWidget(box);

    connect(nameFilter_, &QLineEdit::textChanged, this, &MaterialDatabaseDialog::applyFilters);
    connect(elementFilter_, &QLineEdit::textChanged, this, &MaterialDatabaseDialog::applyFilters);
    connect(listWidget_, &QListWidget::currentRowChanged, this,
            &MaterialDatabaseDialog::onCurrentMaterialChanged);

    if (!loadDatabase()) {
        QMessageBox::warning(this, "Material database",
                             "Failed to load embedded materials database.");
    }
    rebuildList();
}

material::material_desc_t MaterialDatabaseDialog::selectedMaterial() const
{
    int row = listWidget_->currentRow();
    if (row < 0 || row >= static_cast<int>(visibleRows_.size()))
        return material::material_desc_t();

    return allEntries_[visibleRows_[row]].mat;
}

bool MaterialDatabaseDialog::loadDatabase()
{
    allEntries_.clear();

    QFile f(kDbPath);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    ojson j;
    try {
        j = ojson::parse(f.readAll().toStdString(), nullptr, true, true);
    } catch (...) {
        return false;
    }

    if (!j.is_array())
        return false;

    for (const auto &node : j) {
        if (!node.is_object() || !node.contains("id") || !node.contains("composition"))
            continue;

        DbEntry e;
        e.mat.id = node.value("id", std::string("Material"));
        e.mat.density = node.value("density", 1.0f);
        e.mat.color = node.value("color", std::string("#55aaff"));

        const std::string description = node.value("description", std::string());
        e.description = QString::fromStdString(description);

        const auto &comp = node["composition"];
        if (comp.is_array()) {
            for (const auto &atomNode : comp) {
                atom::parameters a;
                std::string symbol = atomNode.value("symbol", std::string("H"));
                a.element.symbol = symbol;
                const auto &pt = periodic_table::at(symbol);
                if (pt.is_valid()) {
                    a.element.atomic_number = pt.Z;
                    a.element.atomic_mass = static_cast<float>(pt.mass);
                }

                a.X = atomNode.value("X", 1.0f);
                a.Ed = atomNode.value("Ed", 40.0f);
                a.El = atomNode.value("El", 3.0f);
                a.Es = atomNode.value("Es", 10.0f);
                a.Er = atomNode.value("Er", 40.0f);
                a.Rc = atomNode.value("Rc", 0.946f);
                e.mat.composition.push_back(a);
            }
        }

        e.displayText = QString::fromStdString(e.mat.id);
        if (!e.description.isEmpty())
            e.displayText += " - " + e.description;

        allEntries_.push_back(e);
    }

    return !allEntries_.empty();
}

void MaterialDatabaseDialog::rebuildList()
{
    listWidget_->clear();
    visibleRows_.clear();

    const QString nameNeedle = nameFilter_->text().trimmed();
    const QString elementNeedle = elementFilter_->text().trimmed();

    for (int i = 0; i < static_cast<int>(allEntries_.size()); ++i) {
        const auto &entry = allEntries_[i];

        QString nameText = entry.displayText + " " + QString::fromStdString(entry.mat.id);
        if (!containsCaseInsensitive(nameText, nameNeedle))
            continue;

        if (!elementNeedle.isEmpty()) {
            bool elementMatch = false;
            for (const auto &a : entry.mat.composition) {
                const QString symbol = QString::fromStdString(a.element.symbol);
                if (symbol.startsWith(elementNeedle, Qt::CaseInsensitive)) {
                    elementMatch = true;
                    break;
                }
            }
            if (!elementMatch)
                continue;
        }

        visibleRows_.push_back(i);
        listWidget_->addItem(entry.displayText);
    }

    if (listWidget_->count() > 0)
        listWidget_->setCurrentRow(0);
    else
        updatePreview(-1);
}

void MaterialDatabaseDialog::updatePreview(int row)
{
    compositionTable_->setRowCount(0);

    if (row < 0 || row >= static_cast<int>(visibleRows_.size())) {
        previewTitle_->setText("Preview");
        densityLabel_->setText("Density:");
        elementsLabel_->setText("Elements:");
        acceptButton_->setEnabled(false);
        return;
    }

    const auto &entry = allEntries_[visibleRows_[row]];
    const auto &mat = entry.mat;

    previewTitle_->setText(QString("Preview: %1").arg(mat.id.c_str()));
    densityLabel_->setText(QString("Density: %1 g/cm^3").arg(mat.density));
    elementsLabel_->setText(QString("Elements: %1").arg(joinMaterialElements(mat)));

    compositionTable_->setRowCount(static_cast<int>(mat.composition.size()));
    for (int i = 0; i < static_cast<int>(mat.composition.size()); ++i) {
        const auto &a = mat.composition[i];
        compositionTable_->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(a.element.symbol)));
        compositionTable_->setItem(i, 1, new QTableWidgetItem(QString::number(a.X)));
        compositionTable_->setItem(i, 2, new QTableWidgetItem(QString::number(a.Ed)));
        compositionTable_->setItem(i, 3, new QTableWidgetItem(QString::number(a.El)));
        compositionTable_->setItem(i, 4, new QTableWidgetItem(QString::number(a.Es)));
        compositionTable_->setItem(i, 5, new QTableWidgetItem(QString::number(a.Er)));
        compositionTable_->setItem(i, 6, new QTableWidgetItem(QString::number(a.Rc)));
    }

    acceptButton_->setEnabled(true);
}

void MaterialDatabaseDialog::applyFilters()
{
    rebuildList();
}

void MaterialDatabaseDialog::onCurrentMaterialChanged(int row)
{
    updatePreview(row);
}
