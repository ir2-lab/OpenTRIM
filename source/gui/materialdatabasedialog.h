#ifndef MATERIALDATABASEDIALOG_H
#define MATERIALDATABASEDIALOG_H

#include <QDialog>
#include <QJsonObject>

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
    explicit MaterialDatabaseDialog(QWidget *parent = nullptr);

    QJsonObject getSelectedMaterial() const;

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
    QVector<QJsonObject> materials;
    QLabel *idLabel;
    QLabel *densityLabel;
    QTableWidget *compositionTable;
};

#endif // MATERIALDATABASEDIALOG_H
