#ifndef TABULARVIEW_H
#define TABULARVIEW_H

#include <QWidget>

class QTabWidget;
class QTableWidget;
class QLineEdit;
class QLabel;
class QPushButton;

class MyDataWidgetMapper;
class MainUI;

class data_table;

class TabularView : public QWidget
{
    Q_OBJECT
public:
    explicit TabularView(MainUI *ui, QWidget *parent = nullptr);

public slots:
    void revert();
    void onTallyUpdate();
    void onSimulationCreated();
    void onSimulationDestroyed();

private:
    MainUI *mainui_;
    MyDataWidgetMapper *mapper_;
    QLineEdit *simTitle_;
    QTabWidget *tabWidget_;

    enum { idxErgTbl = 0, idxDmgEvntsTbl, idxNTbls };
    std::array<data_table *, idxNTbls> tables_;
};

#endif // TABULARVIEW_H
