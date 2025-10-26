#include "resultsview.h"

#include "mcdriverobj.h"
#include "mcplotinfo.h"

#include <QBitmap>
#include <QButtonGroup>
#include <QComboBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMenu>
#include <QMessageBox>
#include <QPixmap>
#include <QToolButton>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

#include <fstream>

#include "mainui.h"

ResultsView::ResultsView(MainUI *iui, QWidget *parent) : QDataBrowser{ parent }, ionsui(iui)
{
    setTreeTitle("Simulation Data");
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

void ResultsView::addPlotItem(const QString &name, mcplotinfo *i, const QString loc)
{
    switch (i->type()) {
    case mcplotinfo::group:
        addGroup(name, loc);
        for (auto &ch : i->children()) {
            addPlotItem(ch.first.c_str(), ch.second, loc + "/" + name);
        }
        break;
    case mcplotinfo::tally_score:
        addData(i->takeData(), loc);
        break;
    default:
        break;
    }
}

void ResultsView::onSimulationCreated()
{
    McDriverObj *D = ionsui->driverObj();

    // if (info_) {
    //     clear();
    //     delete info_;
    //     info_ = nullptr;
    // }

    mcplotinfo info(D);

    QString loc = "/";
    for (auto &ch : info.children()) {
        addPlotItem(ch.first.c_str(), ch.second);
    }

    selectItem("/tally/damage_events/Vacancies");
}

void ResultsView::onSimulationDestroyed()
{
    clear();
    // if (info_)
    //     delete info_;
    // info_ = nullptr;
}

void ResultsView::onTallyUpdate()
{
    dataUpdated();
}
