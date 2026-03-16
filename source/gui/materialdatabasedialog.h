#ifndef MATERIALDATABASEDIALOG_H
#define MATERIALDATABASEDIALOG_H

#include <QDialog>
#include <QString>

#include <vector>

#include "target.h"

class QListWidget;
class QLabel;
class QLineEdit;
class QTableWidget;
class QPushButton;

class MaterialDatabaseDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MaterialDatabaseDialog(QWidget *parent = nullptr);

    material::material_desc_t selectedMaterial() const;

private slots:
    void applyFilters();
    void onCurrentMaterialChanged(int row);

private:
    struct DbEntry {
        material::material_desc_t mat;
        QString description;
        QString displayText;
    };

    bool loadDatabase();
    void rebuildList();
    void updatePreview(int row);

    QLineEdit *nameFilter_;
    QLineEdit *elementFilter_;
    QListWidget *listWidget_;
    QLabel *previewTitle_;
    QLabel *densityLabel_;
    QLabel *elementsLabel_;
    QTableWidget *compositionTable_;
    QPushButton *acceptButton_;

    std::vector<DbEntry> allEntries_;
    std::vector<int> visibleRows_;
};

#endif // MATERIALDATABASEDIALOG_H
