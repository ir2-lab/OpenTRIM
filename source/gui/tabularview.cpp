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
#include <QButtonGroup>
#include <QLineEdit>
#include <QTabWidget>
#include <QTableWidget>
#include <QTableWidgetItem>

class data_table
{
protected:
    int rows_;
    QTableWidget *widget_;
    QButtonGroup *unitSelector_;
    QString title_;
    ArrayNDd buff;

public:
    explicit data_table(const char *t, int r)
        : rows_(r), widget_(nullptr), unitSelector_(nullptr), title_(t)
    {
    }
    QTableWidget *widget() const { return widget_; }
    QButtonGroup *unitSelector() const { return unitSelector_; }
    const QString &title() const { return title_; }
    int rows() const { return rows_; }
    virtual const char *rowLabel(int i) const = 0;
    virtual void create()
    {
        widget_ = new QTableWidget(rows_, 1);
        widget_->setEditTriggers(QAbstractItemView::NoEditTriggers);
        QStringList lbls;
        for (int i = 0; i < rows_; ++i)
            lbls << rowLabel(i);
        widget_->setVerticalHeaderLabels(lbls);
    }
    virtual void init(const target &t)
    {
        if (!widget_)
            return;
        auto &atoms = t.atoms();

        widget_->setColumnCount(atoms.size() + 1);
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
        widget_->setHorizontalHeaderLabels(lbls);

        for (int i = 0; i < rows_; ++i) {
            for (int j = 0; j < atoms.size() + 1; ++j) {
                widget_->setItem(i, j, new QTableWidgetItem());
            }
        }

        buff = ArrayNDd(2, rows_, atoms.size() + 1);
    }
    virtual void clear()
    {
        if (!widget_)
            return;
        widget_->clearContents();
        widget_->setColumnCount(1);
    }
    virtual void update(const ArrayNDd &t, const ArrayNDd &dt) = 0;
};

class erg_table : public data_table
{
    constexpr static std::array<int, 4> idx{ tally::eIoniz, tally::eLattice, tally::eStored,
                                             tally::eLost };

public:
    enum { tblErg, tblPercent };
    erg_table() : data_table("Energy Deposition", 5) { }

    virtual const char *rowLabel(int i) const override
    {
        return i < 4 ? tally::arrayName(idx[i]) : "Total";
    }
    virtual void create() override
    {
        data_table::create();
        unitSelector_ = new QButtonGroup(widget_);
        QPushButton *bt = new QPushButton("eV/ion");
        bt->setCheckable(true);
        bt->setChecked(true);
        unitSelector_->addButton(bt, tblErg);
        bt = new QPushButton("Percent (%)");
        bt->setCheckable(true);
        bt->setChecked(false);
        unitSelector_->addButton(bt, tblPercent);
    }
    virtual void update(const ArrayNDd &t, const ArrayNDd &dt) override
    {
        double f = t[0]; // N histories
        if (f <= 1.0)
            return;

        double f1 = 1. / (f - 1.); // 1/(N-1)
        f = 1 / f; // 1/N

        int cols = buff.dim()[2];

        for (int i = 0; i < rows_ - 1; ++i) {
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
            buff(0, rows_ - 1, j) = 0;
            buff(1, rows_ - 1, j) = 0;
            for (int i = 0; i < rows_ - 1; ++i) {
                buff(0, rows_ - 1, j) += buff(0, i, j);
                buff(1, rows_ - 1, j) += buff(1, i, j);
            }
        }

        double f2 =
                (unitSelector_->checkedId() == tblErg) ? 1.0 : 100.0 / buff(0, rows_ - 1, cols - 1);

        for (int i = 0; i < rows_; ++i) {
            for (int j = 0; j < cols; ++j) {
                double x = buff(0, i, j) * f2;
                double dx = buff(1, i, j) * f1;
                if (dx > 0.) {
                    dx = std::sqrt(dx) * f2;
                    widget_->item(i, j)->setText(
                            QString::fromStdString(print_with_err(x, dx, 'g', 1)));
                } else {
                    widget_->item(i, j)->setText(QString::number(x, 'g'));
                }
            }
        }
    }
};

class dmg_events_table : public data_table
{
    constexpr static int drows{ 6 };
    constexpr static std::array<int, drows> idx{ tally::cP, tally::cD, tally::cV,
                                                 tally::cI, tally::cR, tally::cRecombinations };

public:
    enum { tblCntsPerIon, tblCntsPerPka, tblPercent };
    dmg_events_table() : data_table("Damage Events", drows) { }

    virtual const char *rowLabel(int i) const override { return tally::arrayName(idx[i]); }
    virtual void create() override
    {
        data_table::create();
        unitSelector_ = new QButtonGroup(widget_);
        QPushButton *bt;
        bt = new QPushButton("cnts/ion");
        bt->setCheckable(true);
        bt->setChecked(true);
        unitSelector_->addButton(bt, tblCntsPerIon);
        bt = new QPushButton("cnts/pka");
        bt->setCheckable(true);
        bt->setChecked(false);
        unitSelector_->addButton(bt, tblCntsPerPka);
        bt = new QPushButton("Percent (%)");
        bt->setCheckable(true);
        bt->setChecked(false);
        unitSelector_->addButton(bt, tblPercent);
    }
    virtual void update(const ArrayNDd &t, const ArrayNDd &dt) override
    {
        double f = t[0]; // N histories
        if (f <= 1.0)
            return;

        double f1 = 1. / (f - 1.); // 1/(N-1)
        f = 1 / f; // 1/N

        int cols = buff.dim()[2];

        // 1st pass
        // compute row data. The last column is the sum
        for (int i = 0; i < rows_; ++i) {
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

        // 2nd pass
        // format and print data with error
        // if unit=cnts, data is in counts/ion
        // otherwise is percentage of total = last column
        for (int i = 0; i < rows_; ++i) {
            double f2{ 1.0 };
            switch (unitSelector_->checkedId()) {
            case tblPercent:
                if (buff(0, i, cols - 1) > 0.0)
                    f2 = 100.0 / buff(0, i, cols - 1);
                break;
            case tblCntsPerPka:
                if (buff(0, 0, cols - 1) > 0.0)
                    f2 = 1.0 / buff(0, 0, cols - 1);
                break;
            default:
                break;
            }
            for (int j = 0; j < cols; ++j) {
                double x = buff(0, i, j) * f2;
                double dx = buff(1, i, j) * f1;
                if (dx > 0.) {
                    dx = std::sqrt(dx) * f2;
                    widget_->item(i, j)->setText(
                            QString::fromStdString(print_with_err(x, dx, 'g', 1)));
                } else {
                    widget_->item(i, j)->setText(QString::number(x, 'g'));
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
    tables_[idxErgTbl] = new erg_table;
    tables_[idxDmgEvntsTbl] = new dmg_events_table;
    for (int itbl = 0; itbl < idxNTbls; ++itbl) {
        data_table *tbl = tables_[itbl];
        tbl->create();
        QWidget *child = new QWidget;
        {
            QVBoxLayout *vbox = new QVBoxLayout;
            child->setLayout(vbox);

            if (tbl->unitSelector()) {
                QHBoxLayout *hbox = new QHBoxLayout;
                hbox->addWidget(new QLabel("Units: "));
                auto btns = tbl->unitSelector()->buttons();
                for (QAbstractButton *bt : btns)
                    hbox->addWidget(bt);
                hbox->addStretch();
                hbox->setSpacing(0);
                vbox->addLayout(hbox);
            }
            vbox->addWidget(tbl->widget());
        }
        tabWidget_->addTab(child, tbl->title());
    }

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

    connect(mainui_->driverObj(), &McDriverObj::tallyUpdate, this, &TabularView::onTallyUpdate,
            Qt::QueuedConnection);

    connect(mainui_->driverObj(), &McDriverObj::configChanged, this, &TabularView::revert);

    connect(mainui_->driverObj(), &McDriverObj::simulationCreated, this,
            &TabularView::onSimulationCreated);

    connect(mainui_->driverObj(), &McDriverObj::simulationDestroyed, this,
            &TabularView::onSimulationDestroyed);

    for (int i = 0; i < idxNTbls; ++i) {
        if (tables_[i]->unitSelector())
            connect(tables_[i]->unitSelector(), &QButtonGroup::idToggled, this,
                    &TabularView::onTallyUpdate);
        // connect(tables_[i]->unitSelector()->buttons().first(), &QPushButton::toggled, this,
        //         &TabularView::onTallyUpdate);
    }
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

    for (int i = 0; i < idxNTbls; ++i)
        tables_[i]->update(T, dT);
}

void TabularView::onSimulationCreated()
{
    McDriverObj *D = mainui_->driverObj();
    for (int i = 0; i < idxNTbls; ++i)
        tables_[i]->init(D->getSim()->getTarget());
}

void TabularView::onSimulationDestroyed()
{
    for (int i = 0; i < idxNTbls; ++i)
        tables_[i]->clear();
}
