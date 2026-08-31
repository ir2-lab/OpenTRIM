#ifndef MATERIALDATABASEDIALOG_H
#define MATERIALDATABASEDIALOG_H

#include <QDialog>
#include <QSet>
#include <QStringList>

#include "target.h"

class QLineEdit;
class QDialogButtonBox;
class QLabel;
class QTreeView;
class QStandardItemModel;
class QSortFilterProxyModel;

class MaterialDatabaseDialog : public QDialog
{
    Q_OBJECT

public:
    enum composition_type_t { atomic, mass };

    struct material_datasheet
    {
        std::string id;
        std::string title;
        float density;
        composition_type_t composition_type;
        std::vector<int> Z;
        std::vector<float> X;
        std::string comment;
        std::string source;
    };

    explicit MaterialDatabaseDialog(const QStringList &existingMaterialIds = {},
                                    QWidget *parent = nullptr);

    material::material_desc_t getSelectedMaterial() const;

private slots:
    void onMaterialSelected(const QModelIndex &selected, const QModelIndex &deselected);
    void onFilterTextChanged(const QString &text);

private:
    void loadMaterialsDb();

    QLineEdit *filterLineEdit;
    QDialogButtonBox *buttonBox;
    QSet<QString> existingMaterialIds_;
    QLabel *materialLabel;

    QStandardItemModel *materials_db;
    QSortFilterProxyModel *proxyModel;
    QString material_card_tmpl;
    QTreeView *materialsTreeView;
    material_datasheet selected_;
};

Q_DECLARE_METATYPE(MaterialDatabaseDialog::material_datasheet)

#endif // MATERIALDATABASEDIALOG_H
