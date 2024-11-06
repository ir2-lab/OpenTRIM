#include "resultsview.h"

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QComboBox>
#include <QToolButton>
#include <QListWidget>
#include <QListWidgetItem>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QButtonGroup>
#include <QPixmap>
#include <QBitmap>

#include "qmatplotwidget/src/QMatPlotWidget.h"

#include "ionsui.h"

ResultsView::ResultsView(IonsUI *iui, QWidget *parent)
    : QSplitter{parent}, ionsui(iui)
{

    /* create left-side tree widget */
    tallyTree = new QTreeWidget;
    tallyTree->setColumnCount(1);
    QTreeWidgetItem* hdr = tallyTree->headerItem();
    hdr->setText(0,"Data Tables");
    QTreeWidgetItem* curr;
    for (int i=1; i<tally::std_tallies; ++i) {
        QString name = tally::arrayName(i);
        QString group = tally::arrayGroup(i);
        QString desc = tally::arrayDescription(i);
        int igrp = findChild(nullptr, group, 0);
        QTreeWidgetItem* groupItem;
        if (igrp >= 0) groupItem = tallyTree->topLevelItem(igrp);
        else {
            groupItem = new QTreeWidgetItem(tallyTree,QStringList({group}));
            groupItem->setFlags(Qt::ItemIsEnabled);
            groupItem->setData(0,Qt::UserRole,-1);
        }

        QTreeWidgetItem* arrayItem = new QTreeWidgetItem(groupItem,QStringList({name}));
        arrayItem->setData(0,Qt::UserRole,i);

        if (i==1) curr = arrayItem;
    }

    /* Create plot widget with controls */
    plotWidget = new QMatPlotWidget;
    plotWidget->setTitle("Title");
    plotWidget->setXlabel("X (nm)");
    plotWidget->setYlabel("count / ion");


    QStringList btLabels{"X", "Y", "Z"};
    axisButtonGrp = new QButtonGroup(this);
    for(int i=0; i<3; i++) {
        axButton[i] = new QToolButton;
        axButton[i]->setText(btLabels.at(i));
        axButton[i]->setCheckable(true);
        axisButtonGrp->addButton(axButton[i],i);
    }
    axButton[0]->setChecked(true);

    int w = axButton[0]->sizeHint().height();
    for(int i=0; i<3; i++) axButton[i]->setMinimumWidth(2*w);
    for(int i=0; i<2; i++) {
        axPts[i] = new QComboBox;
        axPtsLbls[i] = new QLabel(btLabels.at(i+1) + " pts.");
        axPts[i]->addItems({"1: 0.0 nm", "2: 1.0 nm", "1: 2.0 nm"});
    }

    plotSelect = new QListWidget;
    plotSelect->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    plotSelect->setMinimumHeight(3*25);
    plotSelect->setIconSize(QSize(16,16));
    plotSelect->addItem("A");
    plotSelect->addItem("B");
    plotSelect->addItem("C");

    /* Layout Widgets */
    QWidget* rightWidget = new QWidget;

    QVBoxLayout* vbox = new QVBoxLayout;
    vbox->addWidget(plotWidget);
    vbox->addSpacing(40);
    plotWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    // bottom layout
    QHBoxLayout* hbox = new QHBoxLayout;
    //hbox->setSpacing(10);
    {
        QFormLayout* fbox = new QFormLayout;
        {
            QHBoxLayout* hbox = new QHBoxLayout;
            for(int i=0; i<3; i++) hbox->addWidget(axButton[i]);
            hbox->setSpacing(0);
            fbox->addRow("Horiz. axis",hbox);
        }
        for(int i=0; i<2; ++i) {
            fbox->addRow(axPtsLbls[i],axPts[i]);
        }
        hbox->addLayout(fbox);
        fbox->setHorizontalSpacing(6);
        fbox->setVerticalSpacing(2);
    }
    hbox->addSpacing(10);
    hbox->addWidget(plotSelect);
    hbox->addStretch();


    vbox->addLayout(hbox);
    rightWidget->setLayout(vbox);
    vbox->setStretch(0,10);
    vbox->setStretch(1,0);
    vbox->setStretch(2,1);
    vbox->setSpacing(0);


    addWidget(tallyTree);
    addWidget(rightWidget);

    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    /* connect signals */
    connect(ionsui->ions_driver, &McDriverObj::simulationCreated,
            this, &ResultsView::onSimulationCreated);
    connect(ionsui->ions_driver, &McDriverObj::simulationDestroyed,
            this, &ResultsView::onSimulationDestroyed);
    connect(ionsui->ions_driver, &McDriverObj::tallyUpdate,
            this, &ResultsView::onTallyUpdate, Qt::QueuedConnection);
    connect(tallyTree, &QTreeWidget::currentItemChanged,
            this, & ResultsView::onItemChanged);
    connect(plotSelect, &QListWidget::itemChanged,
            this, &ResultsView::onPlotSelectChanged);


    /* set the current item to "Vacancies" */
    tallyTree->setCurrentItem(curr);
}

void ResultsView::setCurrentTable(int i)
{
    currentTable_ = i;

    QStringList lbls{"Y pts.", "Z pts."};
    for(int i=0; i<2; ++i) {
        axPts[i]->clear();
        axPtsLbls[i]->setText(lbls.at(i));
    }

    plotSelect->clear();

    Data.clear();

    if (tally_.at(currentTable_).isNull())
        return;

    copyTable();
    updatePlotSeries();
    updateTableData();
    updatePlot();

}

void ResultsView::updateTableData()
{
    const ArrayNDd& A = tally_.at(currentTable_);
    size_t Nh = tally_.at(0)[0];

    int natoms = A.dim()[0];
    int axId = axisButtonGrp->checkedId();
    int idx[3] = {0, 0, 0};

    Data.clear();

    for(int ia=0; ia<natoms; ++ia) {
        int n = Nx[axId];
        ArrayNDd Y(n);
        for(int i=0; i<n; ++i) {
            idx[axId] = i;
            Y[i] = A(ia, idx_(idx))/Nh;
        }
        Data.push_back(Y);
    }
}

void ResultsView::copyTable()
{
    ArrayNDd A0 = tally_.at(0);
    ionsui->ions_driver->getSim()->copyTallyTable(0, A0);
    ArrayNDd A = tally_.at(currentTable_);
    ionsui->ions_driver->getSim()->copyTallyTable(currentTable_, A);
}

void ResultsView::onTallyUpdate()
{
    copyTable();
    updateTableData();
    updatePlot();
}

void ResultsView::updatePlotSeries()
{
    static const int useFirstCol[] = {
        0,
        0, 1, 1, 0, 1,  // defects
        1, 1, 1, 1, 0, 0, 1,
        0, 0, 0, 0, 1, 1
    };

    plotSelect->clear();
    plotFlag.clear();

    QIcon icon(":/icons/assets/lucide/chart-line.svg");

    for(int i=0; i<atomLabels.count(); ++i) {

        QPixmap pix = icon.pixmap(32,32);
        QBitmap mask = pix.createMaskFromColor(
            QColor("black"),
            Qt::MaskOutColor);
        pix.fill(plotWidget->colorOrder()[i]);
        pix.setMask(mask);
        QListWidgetItem* item =
            new QListWidgetItem(QIcon(pix),atomLabels.at(i));

        item->setFlags(Qt::ItemIsSelectable |
                       Qt::ItemIsUserCheckable |
                       Qt::ItemIsEnabled);

        if (i==0 && !useFirstCol[currentTable_]) {
            item->setCheckState(Qt::Unchecked);
            plotFlag.push_back(0);
        } else {
            item->setCheckState(Qt::Checked);
            plotFlag.push_back(1);
        }

        item->setData(Qt::UserRole,i);
        plotSelect->addItem(item);
    }



}

void ResultsView::updateAxisSelection()
{

}

void ResultsView::onPlotSelectChanged(QListWidgetItem *i)
{
    int idx = i->data(Qt::UserRole).toInt();
    plotFlag[idx] = i->checkState() == Qt::Checked;
    updatePlot();
}


void ResultsView::updateDataSelection()
{

}

void ResultsView::updatePlot()
{
    plotWidget->clear();

    int axId = axisButtonGrp->checkedId();
    auto clrs = plotWidget->colorOrder();

    for(int i=0; i<Data.size(); i++) {
        QColor clr = clrs[i % clrs.size()];
        if (plotFlag[i])
            plotWidget->plot(X[axId],Data[i],QString(),clr);
    }

}

void ResultsView::onSimulationCreated()
{
    McDriverObj* D = ionsui->ions_driver;

    tally_ = D->getTally().clone();

    // get spatial grid
    auto grid = D->getSim()->getTarget().grid();
    X.clear();
    int k = 0;
    {
        Nx[k] = grid.x().size()-1;
        X.push_back(ArrayNDd(Nx[k]));
        for(int i=0; i<Nx[k]; ++i) X[k][i] = grid.x()[i];
    }
    k=1;
    {
        Nx[k] = grid.y().size()-1;
        X.push_back(ArrayNDd(Nx[k]));
        for(int i=0; i<Nx[k]; ++i) X[k][i] = grid.y()[i];
    }
    k=2;
    {
        Nx[k] = grid.z().size()-1;
        X.push_back(ArrayNDd(Nx[k]));
        for(int i=0; i<Nx[k]; ++i) X[k][i] = grid.z()[i];
    }

    // atom labels
    auto atom_labels = D->getSim()->getTarget().atom_labels();
    atomLabels.clear();
    for(auto s : atom_labels) atomLabels << s.c_str();

    // update the current table
    setCurrentTable(currentTable_);
}

void ResultsView::onSimulationDestroyed()
{
    tally_ = tally();
    setCurrentTable(currentTable_);
}

void ResultsView::onItemChanged()
{
    QTreeWidgetItem* item = tallyTree->currentItem();
    QString name = item->data(0,Qt::DisplayRole).toString();
    int iTable = item->data(0,Qt::UserRole).toInt();
    if (iTable<0) return;
    if (iTable!=currentTable_) {
        setCurrentTable(iTable);
    }
}


QTreeWidgetItem *ResultsView::createItem(const QString &text,
                                          QTreeWidgetItem *parent, int index)
{
    QTreeWidgetItem *after = nullptr;
    if (index != 0)
        after = parent ? parent->child(index - 1) : tallyTree->topLevelItem(index - 1);

    QTreeWidgetItem *item;
    if (parent)
        item = new QTreeWidgetItem(parent, after);
    else
        item = new QTreeWidgetItem(tallyTree, after);

    item->setText(0, text);
    // item->setFlags(item->flags() | Qt::ItemIsEditable);
    return item;
}

int ResultsView::findChild(QTreeWidgetItem *parent, const QString &text,
                            int startIndex) const
{
    int count = parent ? parent->childCount() : tallyTree->topLevelItemCount();
    for (int i = startIndex; i < count; ++i) {
        QTreeWidgetItem *child = parent ? parent->child(i) :
                                     tallyTree->topLevelItem(i);
        if (child->text(0) == text) return i;
    }
    return -1;
}


