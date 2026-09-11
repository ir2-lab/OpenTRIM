#include "resultsview.h"

#include "mcdriverobj.h"
#include "mcdatamodel.h"

#include <QDataBrowser>

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

ResultsView::ResultsView(MainUI *ui, const QString &title, QWidget *parent)
    : Page(ui, title, true, parent), ionsui(ui)
{
    dataBrowser = new QDataBrowser;
    dataBrowser->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    setContent(dataBrowser);

    /* connect signals */
    connect(ionsui->driverObj(), &McDriverObj::simulationCreated, this,
            &ResultsView::onSimulationCreated);
    connect(ionsui->driverObj(), &McDriverObj::simulationDestroyed, this,
            &ResultsView::onSimulationDestroyed);
    connect(ionsui->driverObj(), &McDriverObj::tallyUpdate, this, &ResultsView::onTallyUpdate,
            Qt::QueuedConnection);
}

void ResultsView::onSimulationCreated()
{
    McDriverObj *D = ionsui->driverObj();

    McDataModel *m = new McDataModel(D->get_mcdriver(), this);
    dataBrowser->setModel(m);

    if (!dataBrowser->hasSavedState()) {
        dataBrowser->setCurrentDataPath("/tally/damage_events/Vacancies");
        dataBrowser->setCurrentViewType(QDataBrowser::Plot);
        dataBrowser->setCurrentPlotType(QDataBrowser::ErrorBar);
    }
}

void ResultsView::onSimulationDestroyed()
{
    // model()->clear();
}

void ResultsView::onTallyUpdate()
{
    dataBrowser->model()->setDatasetChanged();
}
