#ifndef RESULTSVIEW_H
#define RESULTSVIEW_H

#include "qdatabrowser.h"

class mcplotinfo;

class QTreeWidget;
class QTreeWidgetItem;
class QToolButton;
class QComboBox;
class QListWidget;
class QLabel;
class QButtonGroup;

class QListWidgetItem;

class MainUI;

class ResultsView : public QDataBrowser
{

    Q_OBJECT
public:
    explicit ResultsView(MainUI *iui, QWidget *parent = nullptr);

signals:

public slots:
    void onSimulationCreated();
    void onSimulationDestroyed();
    void onTallyUpdate();

private:
    MainUI *ionsui{ nullptr };

    void addPlotItem(const QString &name, mcplotinfo *i, const QString loc = "");
};

#endif // RESULTSVIEW_H
