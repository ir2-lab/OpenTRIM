#include "tabularview.h"

#include "mainui.h"
#include "mydatawidgetmapper.h"
#include "optionsmodel.h"
#include "mcdriverobj.h"
#include "tally.h"
#include "error_fmt.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QTabWidget>
#include <QTableWidget>
#include <QTableWidgetItem>

struct erg_dep_hlpr
{
    constexpr static int rows{ 5 };
    constexpr static std::array<int, 4> idx{ tally::eIoniz, tally::eLattice, tally::eStored,
                                             tally::eLost };
    constexpr static const char *rowLabels[]{ "Ionization", "Lattice", "Stored", "Lost", "Total" };
    static QTableWidget *create()
    {
        QTableWidget *tw = new QTableWidget(rows, 1);
        tw->setEditTriggers(QAbstractItemView::NoEditTriggers);
        QStringList lbls;
        for (int i = 0; i < rows; ++i)
            lbls << rowLabels[i];
        tw->setVerticalHeaderLabels(lbls);
        return tw;
    }
    static void init(QTableWidget *tw, const target &t, ArrayNDd &buff)
    {
        auto &atoms = t.atoms();

        tw->setColumnCount(atoms.size() + 1);
        QStringList lbls;
        for (const atom *at : atoms) {
            if (at->id())
                lbls << QString("%1 in %2")
                                .arg(at->symbol().c_str())
                                .arg(at->mat()->name().c_str());
            else
                lbls << QString("%1 ion").arg(at->symbol().c_str());
        }
        lbls << "Total";
        tw->setHorizontalHeaderLabels(lbls);

        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < atoms.size() + 1; ++j) {
                tw->setItem(i, j, new QTableWidgetItem());
            }
        }

        buff = ArrayNDd(2, rows, atoms.size() + 1);
    }
    static void update(QTableWidget *tw, const ArrayNDd &t, const ArrayNDd &dt, ArrayNDd &buff,
                       bool eV)
    {
        double f = t[0]; // N histories
        if (f <= 1.0)
            return;

        double f1 = 1. / (f - 1.); // 1/(N-1)
        f = 1 / f; // 1/N

        int cols = buff.dim()[2];

        for (int i = 0; i < rows - 1; ++i) {
            buff(0, i, cols - 1) = 0;
            buff(1, i, cols - 1) = 0;
            for (int j = 0; j < cols - 1; ++j) {
                double x = t(idx[i], j) * f;
                double dx = dt(idx[i], j) * f - x * x;
                buff(0, i, j) = x;
                buff(0, i, cols - 1) += x;
                buff(1, i, j) = dx;
                buff(1, i, cols - 1) += dx;
            }
        }
        for (int j = 0; j < cols; ++j) {
            buff(0, rows - 1, j) = 0;
            buff(1, rows - 1, j) = 0;
            for (int i = 0; i < rows - 1; ++i) {
                buff(0, rows - 1, j) += buff(0, i, j);
                buff(1, rows - 1, j) += buff(1, i, j);
            }
        }

        double f2 = eV ? 1.0 : 100.0 / buff(0, rows - 1, cols - 1);

        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                double x = buff(0, i, j) * f2;
                double dx = buff(1, i, j) * f1;
                if (dx > 0.) {
                    dx = std::sqrt(dx) * f2;
                    tw->item(i, j)->setText(QString::fromStdString(print_with_err(x, dx, 'g', 1)));
                } else {
                    tw->item(i, j)->setText(QString::number(x, 'g'));
                }
            }
        }
    }
};

struct defect_hlpr
{
    constexpr static int rows{ 4 };
    constexpr static std::array<int, 4> idx{ tally::cV, tally::cI, tally::cR,
                                             tally::cRecombinations };
    constexpr static const char *rowLabels[]{ "Vacancies", "Implantations", "Replacements",
                                              "Recombinations" };
    static QTableWidget *create()
    {
        QTableWidget *tw = new QTableWidget(rows, 1);
        tw->setEditTriggers(QAbstractItemView::NoEditTriggers);
        QStringList lbls;
        for (int i = 0; i < rows; ++i)
            lbls << rowLabels[i];
        tw->setVerticalHeaderLabels(lbls);
        return tw;
    }
    static void init(QTableWidget *tw, const target &t, ArrayNDd &buff)
    {
        auto &atoms = t.atoms();

        tw->setColumnCount(atoms.size() + 1);
        QStringList lbls;
        for (const atom *at : atoms) {
            if (at->id())
                lbls << QString("%1 in %2")
                                .arg(at->symbol().c_str())
                                .arg(at->mat()->name().c_str());
            else
                lbls << QString("%1 ion").arg(at->symbol().c_str());
        }
        lbls << "Total";
        tw->setHorizontalHeaderLabels(lbls);

        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < atoms.size() + 1; ++j) {
                tw->setItem(i, j, new QTableWidgetItem());
            }
        }

        buff = ArrayNDd(2, rows, atoms.size() + 1);
    }
    static void update(QTableWidget *tw, const ArrayNDd &t, const ArrayNDd &dt, ArrayNDd &buff,
                       bool cnts)
    {
        double f = t[0]; // N histories
        if (f <= 1.0)
            return;

        double f1 = 1. / (f - 1.); // 1/(N-1)
        f = 1 / f; // 1/N

        int cols = buff.dim()[2];

        for (int i = 0; i < rows; ++i) {
            buff(0, i, cols - 1) = 0;
            buff(1, i, cols - 1) = 0;
            for (int j = 0; j < cols - 1; ++j) {
                double x = t(idx[i], j) * f;
                double dx = dt(idx[i], j) * f - x * x;
                buff(0, i, j) = x;
                buff(0, i, cols - 1) += x;
                buff(1, i, j) = dx;
                buff(1, i, cols - 1) += dx;
            }
        }

        for (int i = 0; i < rows; ++i) {
            double f2{ 1.0 };
            if (!cnts && buff(0, i, cols - 1) > 0.0)
                f2 = 100.0 / buff(0, i, cols - 1);
            for (int j = 0; j < cols; ++j) {
                double x = buff(0, i, j) * f2;
                double dx = buff(1, i, j) * f1;
                if (dx > 0.) {
                    dx = std::sqrt(dx) * f2;
                    tw->item(i, j)->setText(QString::fromStdString(print_with_err(x, dx, 'g', 1)));
                } else {
                    tw->item(i, j)->setText(QString::number(x, 'g'));
                }
            }
        }
    }
};

TabularView::TabularView(MainUI *ui, QWidget *parent) : QWidget{ parent }, mainui_(ui)
{
    /* Create & Map widgets to OptionsModel */

    OptionsModel *model = mainui_->optionsModel;
    mapper_ = new MyDataWidgetMapper(model, this);

    // main title widget
    QLabel *simTitleLabel = new QLabel("Simulation title:");
    {
        QModelIndex idxOut = model->index("Output", 0);
        QModelIndex idxTitle = model->index("title", 0, idxOut);
        OptionsItem *item = model->getItem(idxTitle);
        simTitle_ = (QLineEdit *)item->createEditor(this);
        simTitle_->setReadOnly(true);
        simTitleLabel->setToolTip(simTitle_->toolTip());
        simTitleLabel->setWhatsThis(simTitle_->whatsThis());
        mapper_->addMapping(simTitle_, idxTitle, item->editorSignal());
        simTitleLabel->setStyleSheet("font-size : 14pt; font-weight : bold;");
        simTitle_->setStyleSheet("font-size : 14pt");
    }

    // tabs with different tables
    tabWidget_ = new QTabWidget;

    // energy table
    tblErgDep_ = erg_dep_hlpr::create();
    QWidget *child = new QWidget;
    {
        QVBoxLayout *vbox = new QVBoxLayout;
        child->setLayout(vbox);

        {
            btErgUnits = new QPushButton("eV/ion");
            QPushButton *btPercent = new QPushButton("Percent (%)");
            btErgUnits->setCheckable(true);
            btErgUnits->setChecked(true);
            btErgUnits->setAutoExclusive(true);
            btPercent->setCheckable(true);
            btPercent->setChecked(false);
            btPercent->setAutoExclusive(true);
            QHBoxLayout *hbox = new QHBoxLayout;
            hbox->addWidget(new QLabel("Units: "));
            hbox->addWidget(btErgUnits);
            hbox->addWidget(btPercent);
            hbox->addStretch();
            hbox->setSpacing(0);
            vbox->addLayout(hbox);
        }
        vbox->addWidget(tblErgDep_);
    }
    tabWidget_->addTab(child, "Energy Deposition");

    // defect table
    tblDef_ = defect_hlpr::create();
    child = new QWidget;
    {
        QVBoxLayout *vbox = new QVBoxLayout;
        child->setLayout(vbox);

        {
            btDefUnits = new QPushButton("cnts/ion");
            QPushButton *btPercent = new QPushButton("Percent (%)");
            btDefUnits->setCheckable(true);
            btDefUnits->setChecked(true);
            btDefUnits->setAutoExclusive(true);
            btPercent->setCheckable(true);
            btPercent->setChecked(false);
            btPercent->setAutoExclusive(true);
            QHBoxLayout *hbox = new QHBoxLayout;
            hbox->addWidget(new QLabel("Units: "));
            hbox->addWidget(btDefUnits);
            hbox->addWidget(btPercent);
            hbox->addStretch();
            hbox->setSpacing(0);
            vbox->addLayout(hbox);
        }
        vbox->addWidget(tblDef_);
    }
    tabWidget_->addTab(child, "Defect Generation");

    QVBoxLayout *vbox = new QVBoxLayout;
    {
        QHBoxLayout *hbox = new QHBoxLayout;
        hbox->addWidget(simTitleLabel);
        hbox->addWidget(simTitle_);
        hbox->addStretch();
        vbox->addLayout(hbox);
    }
    vbox->addWidget(tabWidget_);
    vbox->addStretch();
    setLayout(vbox);
    vbox->setContentsMargins(0, 0, 0, 0);

    /* Connect Signals */

    bool ret = connect(mainui_->driverObj(), &McDriverObj::statusChanged, this,
                       &TabularView::onDriverStatusChanged, Qt::QueuedConnection);
    assert(ret);

    connect(mainui_->driverObj(), &McDriverObj::tallyUpdate, this, &TabularView::onTallyUpdate,
            Qt::QueuedConnection);

    connect(mainui_->driverObj(), &McDriverObj::configChanged, this, &TabularView::revert);

    connect(mainui_->driverObj(), &McDriverObj::simulationCreated, this,
            &TabularView::onSimulationCreated);

    connect(mainui_->driverObj(), &McDriverObj::simulationDestroyed, this,
            &TabularView::onSimulationDestroyed);

    connect(btErgUnits, &QPushButton::toggled, this, &TabularView::onErgUnits_eV);

    connect(btDefUnits, &QPushButton::toggled, this, &TabularView::onDefUnits_cnts);
}

void TabularView::revert()
{
    mapper_->revert();
}

void TabularView::onTallyUpdate()
{
    auto T = mainui_->driverObj()->totals().copy();
    auto dT = mainui_->driverObj()->dtotals().copy();

    if (T.isNull())
        return;

    erg_dep_hlpr::update(tblErgDep_, T, dT, ergBuff_, btErgUnits->isChecked());
    defect_hlpr::update(tblDef_, T, dT, defBuff_, btDefUnits->isChecked());
}

void TabularView::onDriverStatusChanged()
{
    const McDriverObj *D = mainui_->driverObj();
    McDriverObj::DriverStatus st = D->status();
}

void TabularView::onSimulationCreated()
{
    McDriverObj *D = mainui_->driverObj();

    erg_dep_hlpr::init(tblErgDep_, D->getSim()->getTarget(), ergBuff_);
    erg_dep_hlpr::init(tblDef_, D->getSim()->getTarget(), defBuff_);
}

void TabularView::onSimulationDestroyed()
{
    tblErgDep_->clearContents();
    tblErgDep_->setColumnCount(1);
    QStringList lbls;
    lbls << "Ion";
    tblErgDep_->setHorizontalHeaderLabels(lbls);

    tblDef_->clearContents();
    tblDef_->setColumnCount(1);
    lbls.clear();
    lbls << "Ion";
    tblDef_->setHorizontalHeaderLabels(lbls);
}

void TabularView::onErgUnits_eV(bool b)
{
    onTallyUpdate();
}

void TabularView::onDefUnits_cnts(bool b)
{
    onTallyUpdate();
}
