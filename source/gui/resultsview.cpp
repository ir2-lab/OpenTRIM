#include "resultsview.h"

#include "mcdriverobj.h"
#include "mcdatamodel.h"

#include <QBitmap>
#include <QButtonGroup>
#include <QComboBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMenu>
#include <QPixmap>
#include <QToolButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

#include "mainui.h"

ResultsView::ResultsView(MainUI *iui, QWidget *parent) : QDataBrowser{ parent }, ionsui(iui)
{
    // setTreeTitle("Data Tables");
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    /* connect signals */
    connect(ionsui->driverObj(), &McDriverObj::simulationCreated, this,
            &ResultsView::onSimulationCreated);
    connect(ionsui->driverObj(), &McDriverObj::simulationDestroyed, this,
            &ResultsView::onSimulationDestroyed);
    connect(ionsui->driverObj(), &McDriverObj::tallyUpdate, this, &ResultsView::onTallyUpdate,
            Qt::QueuedConnection);

    /* set the current item to "Vacancies" */
    // tallyTree->setCurrentItem(curr);
}

void ResultsView::onSimulationCreated()
{
    McDriverObj *D = ionsui->driverObj();

    McDataModel *m = new McDataModel(D->get_mcdriver(), this);
    setModel(m);

    if (!hasSavedState()) {
        setCurrentDataPath("/tally/damage_events/Vacancies");
        setCurrentViewType(Plot);
        setCurrentPlotType(ErrorBar);
    }

    // selectItem("/tally/damage_events/Vacancies");
}

void ResultsView::onSimulationDestroyed()
{
    // model()->clear();
}

void ResultsView::onTallyUpdate()
{
    model()->setDatasetChanged();
}
