#ifndef RESULTSVIEW_H
#define RESULTSVIEW_H

#include "mainui.h"

class QTreeWidget;
class QTreeWidgetItem;
class QToolButton;
class QComboBox;
class QListWidget;
class QLabel;
class QButtonGroup;
class QDataBrowser;

class ResultsView : public Page
{

    Q_OBJECT
public:
    explicit ResultsView(MainUI *ui, const QString &title, QWidget *parent = nullptr);

signals:

public slots:
    void onSimulationCreated();
    void onSimulationDestroyed();
    void onTallyUpdate();

private:
    MainUI *ionsui{ nullptr };
    QDataBrowser *dataBrowser;
};

#endif // RESULTSVIEW_H
