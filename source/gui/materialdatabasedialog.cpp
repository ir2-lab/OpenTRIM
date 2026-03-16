#include "materialdatabasedialog.h"

#include <QListWidget>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QTableWidget>
#include <QHeaderView>
#include <QFile>
#include <QMessageBox>
#include <QPushButton>

MaterialDatabaseDialog::MaterialDatabaseDialog(const QStringList &existingMaterialIds,
                                               QWidget *parent)
    : QDialog(parent), existingMaterialIds_(QSet<QString>(existingMaterialIds.begin(),
                                                          existingMaterialIds.end()))
{
    setWindowTitle("Select Material from Database");
    setMinimumSize(600, 400);

    filterLineEdit = new QLineEdit;
    filterLineEdit->setPlaceholderText("Filter materials...");

    materialListWidget = new QListWidget;
        materialListWidget->setStyleSheet(
            "QListWidget::item:disabled {"
            " background-color: palette(alternate-base);"
            " }");
    
    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

    connect(filterLineEdit, &QLineEdit::textChanged, this, &MaterialDatabaseDialog::onFilterTextChanged);
    connect(materialListWidget, &QListWidget::currentRowChanged, this, &MaterialDatabaseDialog::onMaterialSelected);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QWidget *detailsPanel = createDetailsPanel();

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(materialListWidget, 1);
    mainLayout->addWidget(detailsPanel, 2);

    QVBoxLayout *containerLayout = new QVBoxLayout(this);
    containerLayout->addWidget(filterLineEdit);
    containerLayout->addLayout(mainLayout);
    containerLayout->addWidget(buttonBox);

    loadDatabase();
}

void MaterialDatabaseDialog::loadDatabase()
{
    // Note: The path should point to the resource file once added to the .qrc
    QFile file(":/assets/materials.json");
    if (!file.exists()) {
        // Fallback for testing if not in resources yet
        file.setFileName("../source/gui/assets/materials.json"); // Corrected path
    }

    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "Error", "Could not open material database.");
        return;
    }

    ojson doc;
    try {
        doc = ojson::parse(file.readAll().toStdString(), nullptr, true, true);
    }
    catch (const std::exception &) {
        QMessageBox::critical(this, "Error", "Could not parse material database.");
        return;
    }

    for (const auto &obj : doc) {
        const ojson &mat = obj["data"];
        materials.push_back(mat);

        QString itemName = QString::fromStdString(obj["name"].get<std::string>());
        QString materialId = QString::fromStdString(mat["id"].get<std::string>());
        if (existingMaterialIds_.contains(materialId)) {
            itemName += " [selected]";
        }

        QListWidgetItem *item = new QListWidgetItem(itemName);
        if (existingMaterialIds_.contains(materialId)) {
            item->setFlags(item->flags() & ~Qt::ItemIsEnabled & ~Qt::ItemIsSelectable);
            item->setToolTip("Already added");
        }
        materialListWidget->addItem(item);
    }
}

void MaterialDatabaseDialog::onMaterialSelected(int currentRow)
{
    if (currentRow < 0 || currentRow >= materials.size()) {
        clearDetails();
        buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        return;
    }

    QListWidgetItem *currentItem = materialListWidget->item(currentRow);
    if (!currentItem || !(currentItem->flags() & Qt::ItemIsEnabled)) {
        clearDetails();
        buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
        return;
    }

    const ojson &selected = materials.at(currentRow);
    idLabel->setText(QString::fromStdString(selected["id"].get<std::string>()));
    densityLabel->setText(QString::number(selected["density"].get<double>()) + " g/cm³");

    compositionTable->setRowCount(0);
    for (const auto &compObj : selected["composition"]) {
        const ojson &elemObj = compObj["element"];
        
        int row = compositionTable->rowCount();
        compositionTable->insertRow(row);
        compositionTable->setItem(
                row, 0,
                new QTableWidgetItem(
                        QString("%1 (%2)").arg(QString::fromStdString(elemObj["symbol"].get<std::string>()))
                                .arg(elemObj["atomic_number"].get<int>())));
        compositionTable->setItem(row, 1,
                                  new QTableWidgetItem(QString::number(compObj["X"].get<double>())));
        compositionTable->setItem(row, 2,
                                  new QTableWidgetItem(QString::number(compObj["Ed"].get<double>())));
        compositionTable->setItem(row, 3,
                                  new QTableWidgetItem(QString::number(compObj["El"].get<double>())));
        compositionTable->setItem(row, 4,
                                  new QTableWidgetItem(QString::number(compObj["Es"].get<double>())));
    }
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(true);
}

ojson MaterialDatabaseDialog::getSelectedMaterial() const
{
    int currentRow = materialListWidget->currentRow();
    if (currentRow < 0 || currentRow >= materials.size()) {
        return ojson();
    }
    return materials.at(currentRow);
}

void MaterialDatabaseDialog::onFilterTextChanged(const QString &text)
{
    for (int i = 0; i < materialListWidget->count(); ++i) {
        QListWidgetItem *item = materialListWidget->item(i);
        // Simple case-insensitive filter
        bool match = item->text().contains(text, Qt::CaseInsensitive);
        item->setHidden(!match);
    }
    clearDetails();
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}

QWidget *MaterialDatabaseDialog::createDetailsPanel()
{
    QWidget *panel = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(panel);
    layout->setContentsMargins(0, 0, 0, 0);

    QFormLayout *formLayout = new QFormLayout;
    idLabel = new QLabel;
    densityLabel = new QLabel;
    formLayout->addRow("<b>ID:</b>", idLabel);
    formLayout->addRow("<b>Density:</b>", densityLabel);

    compositionTable = new QTableWidget;
    compositionTable->setColumnCount(5);
    compositionTable->setHorizontalHeaderLabels({"Element", "X", "Ed (eV)", "El (eV)", "Es (eV)"});
    compositionTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    compositionTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    compositionTable->verticalHeader()->setVisible(false);
    compositionTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    layout->addLayout(formLayout);
    layout->addWidget(new QLabel("<b>Composition:</b>"));
    layout->addWidget(compositionTable);

    return panel;
}

void MaterialDatabaseDialog::clearDetails()
{
    idLabel->clear();
    densityLabel->clear();
    compositionTable->setRowCount(0);
}
