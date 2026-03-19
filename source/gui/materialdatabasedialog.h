#ifndef MATERIALDATABASEDIALOG_H
#define MATERIALDATABASEDIALOG_H

#include <QDialog>
#include <QSet>
#include <QStringList>

#include "json_defs_p.h"

class QListWidget;
class QTextBrowser;
class QLineEdit;
class QDialogButtonBox;
class QLabel;
class QTableWidget;

class MaterialDatabaseDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MaterialDatabaseDialog(const QStringList &existingMaterialIds = {},
                                    QWidget *parent = nullptr);

    ojson getSelectedMaterial() const;

private slots:
    void onMaterialSelected(int currentRow);
    void onFilterTextChanged(const QString &text);

private:
    void loadDatabase();
    QWidget *createDetailsPanel();
    void clearDetails();

    QListWidget *materialListWidget;
    QLineEdit *filterLineEdit;
    QDialogButtonBox *buttonBox;
    std::vector<ojson> materials;
    QSet<QString> existingMaterialIds_;
    QLabel *idLabel;
    QLabel *densityLabel;
    QTableWidget *compositionTable;
};

#endif // MATERIALDATABASEDIALOG_H
