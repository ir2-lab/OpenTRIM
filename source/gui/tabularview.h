#ifndef TABULARVIEW_H
#define TABULARVIEW_H

#include <QWidget>

#include "arrays.h"

class QTabWidget;
class QTableWidget;
class QLineEdit;
class QLabel;
class QPushButton;

class MyDataWidgetMapper;
class MainUI;

class TabularView : public QWidget
{
    Q_OBJECT
public:
    explicit TabularView(MainUI *ui, QWidget *parent = nullptr);

public slots:
    void revert();
    void onTallyUpdate();
    void onDriverStatusChanged();
    void onSimulationCreated();
    void onSimulationDestroyed();
    void onErgUnits_eV(bool b);
    void onDefUnits_cnts(bool b);

private:
    MainUI *mainui_;
    MyDataWidgetMapper *mapper_;
    QLineEdit *simTitle_;
    QTabWidget *tabWidget_;
    QTableWidget *tblErgDep_;
    QPushButton *btErgUnits;
    QTableWidget *tblDef_;
    QPushButton *btDefUnits;

    ArrayNDd ergBuff_, defBuff_;
};

#endif // TABULARVIEW_H
