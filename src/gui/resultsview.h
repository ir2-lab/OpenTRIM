#ifndef RESULTSVIEW_H
#define RESULTSVIEW_H

#include <QSplitter>

#include "tally.h"

class QTreeWidget;
class QTreeWidgetItem;
class QToolButton;
class QComboBox;
class QListWidget;
class QLabel;
class QButtonGroup;
class QMatPlotWidget;
class QListWidgetItem;

class IonsUI;

class ResultsView : public QSplitter
{

    Q_OBJECT
public:

    QTreeWidget* tallyTree;
    QMatPlotWidget* plotWidget;
    QToolButton* axButton[3];
    QButtonGroup* axisButtonGrp;
    QToolButton* btExport;
    QComboBox* axPts[2];
    QLabel* axPtsLbls[2];
    QListWidget* plotSelect;

    explicit ResultsView(IonsUI* iui, QWidget *parent = nullptr);

    int currentTable() const { return currentTable_; }
    void setCurrentTable(int i);

signals:

public slots:
    void onSimulationCreated();
    void onSimulationDestroyed();
    void onItemChanged();
    void onTallyUpdate();

private slots:
    void updatePlotSeries();
    void updateAxisSelection();
    void onPlotSelectChanged(QListWidgetItem* i);
    void onExportCSV();
    void onExportPlot();

private:

    IonsUI* ionsui;
    tally tally_;
    int currentTable_{-1};

    // plot data
    QVector<ArrayNDd> X;
    size_t Nx[3];
    QVector<ArrayNDd> Data;
    QVector<int> plotFlag;
    QStringList atomLabels;

    int idx_(int i, int j, int k) const {
        return  (i*Nx[1]+j)*Nx[2]+k;
    }
    int idx_(int i[3]) const {
        return  (i[0]*Nx[1]+i[1])*Nx[2]+i[2];
    }
    void copyTable();
    void updateTableData();
    void updateDataSelection();
    void updatePlot();

    QTreeWidgetItem* createItem(const QString &text, QTreeWidgetItem *parent, int index);
    void updateChildItems(QTreeWidgetItem *parent);
    int findChild(QTreeWidgetItem *parent, const QString &text,
                  int startIndex) const;
};

#endif // RESULTSVIEW_H
