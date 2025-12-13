#include "tabularview.h"

#include "mainui.h"
#include "mydatawidgetmapper.h"
#include "optionsmodel.h"
#include "mcdriverobj.h"
#include "tally.h"
#include "value_with_error.h"

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
    QWidget *widget_;
    QTableWidget *tblWidget_;
    QButtonGroup *unitSelector_;
    QString title_;
    ArrayNDd buff;

public:
    explicit data_table(const char *t, int r)
        : rows_(r), tblWidget_(nullptr), unitSelector_(nullptr), title_(t)
    {
    }
    QWidget *widget() const { return widget_; }
    QTableWidget *tblWidget() const { return tblWidget_; }
    QButtonGroup *unitSelector() const { return unitSelector_; }
    const QString &title() const { return title_; }
    int rows() const { return rows_; }
    virtual const char *rowLabel(int i) const = 0;
    virtual void create()
    {
        tblWidget_ = new QTableWidget(rows_, 1);
        tblWidget_->setEditTriggers(QAbstractItemView::NoEditTriggers);
        QStringList lbls;
        for (int i = 0; i < rows_; ++i)
            lbls << rowLabel(i);
        tblWidget_->setVerticalHeaderLabels(lbls);
    }
    virtual void init(const target &t)
    {
        if (!tblWidget_)
            return;
        auto &atoms = t.atoms();

        tblWidget_->setColumnCount(atoms.size() + 1);
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
        tblWidget_->setHorizontalHeaderLabels(lbls);

        for (int i = 0; i < rows_; ++i) {
            for (int j = 0; j < atoms.size() + 1; ++j) {
                tblWidget_->setItem(i, j, new QTableWidgetItem());
            }
        }

        buff = ArrayNDd(2, rows_, atoms.size() + 1);
    }
    virtual void clear()
    {
        if (!tblWidget_)
            return;
        tblWidget_->clearContents();
        tblWidget_->setColumnCount(1);
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

        unitSelector_ = new QButtonGroup(tblWidget_);
        QPushButton *bt = new QPushButton("eV/ion");
        bt->setCheckable(true);
        bt->setChecked(true);
        unitSelector_->addButton(bt, tblErg);
        bt = new QPushButton("Percent (%)");
        bt->setCheckable(true);
        bt->setChecked(false);
        unitSelector_->addButton(bt, tblPercent);

        widget_ = new QWidget;
        QVBoxLayout *vbox = new QVBoxLayout;
        widget_->setLayout(vbox);
        QHBoxLayout *hbox = new QHBoxLayout;
        hbox->addWidget(new QLabel("Units: "));
        for (QAbstractButton *bt : unitSelector_->buttons())
            hbox->addWidget(bt);
        hbox->addStretch();
        hbox->setSpacing(0);
        vbox->addLayout(hbox);
        vbox->addWidget(tblWidget_);
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
                    tblWidget_->item(i, j)->setText(QString::fromStdString(
                            value_with_error(x, dx, 1, std::defaultfloat, true).to_string()));
                } else {
                    tblWidget_->item(i, j)->setText(QString::number(x, 'g'));
                }
            }
        }
    }
};

class dmg_events_table : public data_table
{
    constexpr static int drows{ 4 };
    constexpr static std::array<int, drows> idx{ tally::cV, tally::cI, tally::cR,
                                                 tally::cRecombinations };

public:
    enum { tblCntsPerIon, tblCntsPerPka, tblPercent };
    dmg_events_table() : data_table("Damage Events", drows) { }

    virtual const char *rowLabel(int i) const override { return tally::arrayName(idx[i]); }
    virtual void create() override
    {
        data_table::create();

        unitSelector_ = new QButtonGroup(tblWidget_);
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

        widget_ = new QWidget;
        QVBoxLayout *vbox = new QVBoxLayout;
        widget_->setLayout(vbox);
        QHBoxLayout *hbox = new QHBoxLayout;
        hbox->addWidget(new QLabel("Units: "));
        for (QAbstractButton *bt : unitSelector_->buttons())
            hbox->addWidget(bt);
        hbox->addStretch();
        hbox->setSpacing(0);
        vbox->addLayout(hbox);
        vbox->addWidget(tblWidget_);
    }
    virtual void update(const ArrayNDd &t, const ArrayNDd &dt) override
    {
        double f = t[0]; // N histories
        if (f <= 1.0)
            return;

        double f1 = 1. / (f - 1.); // 1/(N-1)
        f = 1 / f; // 1/N

        int cols = buff.dim()[2];
        double Npka = 0.;
        for (int i = 1; i < cols; ++i)
            Npka += t(tally::cPKA, i) * f;

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
                if (Npka > 0.0)
                    f2 = 1.0 / Npka;
                break;
            default:
                break;
            }
            for (int j = 0; j < cols; ++j) {
                double x = buff(0, i, j) * f2;
                double dx = buff(1, i, j) * f1;
                if (dx > 0.) {
                    dx = std::sqrt(dx) * f2;
                    tblWidget_->item(i, j)->setText(QString::fromStdString(
                            value_with_error(x, dx, 1, std::defaultfloat, true).to_string()));
                } else {
                    tblWidget_->item(i, j)->setText(QString::number(x, 'g'));
                }
            }
        }
    }
};

class dmg_parameters_table : public data_table
{
    constexpr static int drows{ 6 };
    constexpr static std::array<int, drows> idx{
        tally::cPKA, tally::ePKA, tally::dpTdam, tally::dpTdam_LSS, tally::dpVnrt, tally::dpVnrt_LSS
    };
    constexpr static std::array<const char *, drows> rowLabels_{ "PKAs",
                                                                 "PKA energy (eV)",
                                                                 "Damage energy (eV)",
                                                                 "LSS Damage energy (eV)",
                                                                 "NRT displacements",
                                                                 "NRT-LSS displacements" };

public:
    enum { tblCntsPerIon, tblCntsPerPka, tblPercent };
    dmg_parameters_table() : data_table("PKA damage", drows) { }

    virtual const char *rowLabel(int i) const override { return rowLabels_[i]; }
    virtual void create() override
    {
        data_table::create();

        unitSelector_ = new QButtonGroup(tblWidget_);
        QPushButton *bt;
        bt = new QPushButton("per Ion");
        bt->setCheckable(true);
        bt->setChecked(true);
        unitSelector_->addButton(bt, tblCntsPerIon);
        bt = new QPushButton("per PKA");
        bt->setCheckable(true);
        bt->setChecked(false);
        unitSelector_->addButton(bt, tblCntsPerPka);
        bt = new QPushButton("Percent (%)");
        bt->setCheckable(true);
        bt->setChecked(false);
        unitSelector_->addButton(bt, tblPercent);

        widget_ = new QWidget;
        QVBoxLayout *vbox = new QVBoxLayout;
        widget_->setLayout(vbox);
        QHBoxLayout *hbox = new QHBoxLayout;
        hbox->addWidget(new QLabel("Units: "));
        for (QAbstractButton *bt : unitSelector_->buttons())
            hbox->addWidget(bt);
        hbox->addStretch();
        hbox->setSpacing(0);
        vbox->addLayout(hbox);
        vbox->addWidget(tblWidget_);
    }
    virtual void init(const target &t) override
    {
        if (!tblWidget_)
            return;
        auto &atoms = t.atoms();

        tblWidget_->setColumnCount(atoms.size()); // atoms - projectile + total
        QStringList lbls;
        for (const atom *at : atoms) {
            if (at->id())
                lbls << QString("%1 in %2")
                                .arg(at->symbol().c_str())
                                .arg(at->mat()->name().c_str());
        }
        lbls << "Total";
        tblWidget_->setHorizontalHeaderLabels(lbls);

        for (int i = 0; i < rows_; ++i) {
            for (int j = 0; j < atoms.size(); ++j) {
                tblWidget_->setItem(i, j, new QTableWidgetItem());
            }
        }

        buff = ArrayNDd(2, rows_, atoms.size());
    }
    virtual void update(const ArrayNDd &t, const ArrayNDd &dt) override
    {
        double f = t[0]; // N histories
        if (f <= 1.0)
            return;

        double f1 = 1. / (f - 1.); // 1/(N-1)
        f = 1 / f; // 1/N

        int cols = buff.dim()[2];
        double Npka = 0.;
        for (int i = 1; i < cols; ++i)
            Npka += t(tally::cPKA, i) * f;

        // 1st pass
        // compute row data. The last column is the sum
        for (int i = 0; i < rows_; ++i) {
            buff(0, i, cols - 1) = 0;
            buff(1, i, cols - 1) = 0;
            for (int j = 0; j < cols - 1; ++j) {
                double x = t(idx[i], j + 1) * f;
                double dx = dt(idx[i], j + 1) * f - x * x;
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
                if (Npka > 0.0)
                    f2 = 1.0 / Npka;
                break;
            default:
                break;
            }
            for (int j = 0; j < cols; ++j) {
                double x = buff(0, i, j) * f2;
                double dx = buff(1, i, j) * f1;
                if (dx > 0.) {
                    dx = std::sqrt(dx) * f2;
                    tblWidget_->item(i, j)->setText(QString::fromStdString(
                            value_with_error(x, dx, 1, std::defaultfloat, true).to_string()));
                } else {
                    tblWidget_->item(i, j)->setText(QString::number(x, 'g'));
                }
            }
        }
    }
};

class ion_stat_table : public data_table
{
    constexpr static int drows{ 4 };
    constexpr static std::array<const char *, drows> rowLabels_{ "Flight path (nm)", "Collisions",
                                                                 "Mean free path (nm)",
                                                                 "Lost ions" };

public:
    ion_stat_table() : data_table("Ion Statistics", drows) { }
    virtual void create() override
    {
        data_table::create();

        widget_ = new QWidget;
        QVBoxLayout *vbox = new QVBoxLayout;
        widget_->setLayout(vbox);
        // QHBoxLayout *hbox = new QHBoxLayout;
        vbox->addWidget(new QLabel("Units: per ion"));
        vbox->addWidget(tblWidget_);
    }
    virtual const char *rowLabel(int i) const override { return rowLabels_[i]; }
    virtual void init(const target &t) override
    {
        if (!tblWidget_)
            return;
        auto &atoms = t.atoms();

        tblWidget_->setColumnCount(atoms.size());
        QStringList lbls;
        for (const atom *at : atoms) {
            if (at->id())
                lbls << QString("%1 in %2")
                                .arg(at->symbol().c_str())
                                .arg(at->mat()->name().c_str());
            else
                lbls << QString("%1 ion").arg(at->symbol().c_str());
        }
        tblWidget_->setHorizontalHeaderLabels(lbls);

        for (int i = 0; i < rows_; ++i) {
            for (int j = 0; j < atoms.size() + 1; ++j) {
                tblWidget_->setItem(i, j, new QTableWidgetItem());
            }
        }

        buff = ArrayNDd(2, rows_, atoms.size());
    }
    virtual void update(const ArrayNDd &t, const ArrayNDd &dt) override
    {
        double f = t[0]; // N histories
        if (f <= 1.0)
            return;

        double f1 = 1. / (f - 1.); // 1/(N-1)
        f = 1 / f; // 1/N

        int cols = buff.dim()[2];

        double x, dx;
        int i, j;

        // projectile flight path
        i = 0;
        j = 0;
        x = t(tally::isFlightPath, 0) * f;
        dx = dt(tally::isFlightPath, 0) * f - x * x;
        dx = (dx > 0.0) ? std::sqrt(dx * f1) : 0.0;
        buff(0, i, j) = x;
        buff(1, i, j) = dx;

        // recoil atoms flight path
        for (j = 1; j < cols; ++j) {
            x = t(tally::isFlightPath, j) * f;
            dx = dt(tally::isFlightPath, j) * f - x * x;
            dx = (dx > 0.0) ? std::sqrt(dx * f1) : 0.0;
            // if (t(tally::cD, j) > 0.0) {
            //     x /= (t(tally::cD, j) * f);
            //     dx /= (t(tally::cD, j) * f);
            // }
            buff(0, i, j) = x;
            buff(1, i, j) = dx;
        }

        // projectile collisions
        i = 1;
        j = 0;
        x = t(tally::isCollision, 0) * f;
        dx = dt(tally::isCollision, 0) * f - x * x;
        dx = (dx > 0.0) ? std::sqrt(dx * f1) : 0.0;
        buff(0, i, j) = x;
        buff(1, i, j) = dx;

        // recoil atoms collisions
        for (j = 1; j < cols; ++j) {
            x = t(tally::isCollision, j) * f;
            dx = dt(tally::isCollision, j) * f - x * x;
            dx = (dx > 0.0) ? std::sqrt(dx * f1) : 0.0;
            // if (t(tally::cD, j) > 0.0) {
            //     x /= (t(tally::cD, j) * f);
            //     dx /= (t(tally::cD, j) * f);
            // }
            buff(0, i, j) = x;
            buff(1, i, j) = dx;
        }

        // recoil atoms mfp
        i = 2;
        for (j = 0; j < cols; ++j) {
            x = t(tally::isFlightPath, j) * f;
            dx = dt(tally::isFlightPath, j) * f - x * x;
            dx = (dx > 0.0) ? std::sqrt(dx * f1) : 0.0;
            if (t(tally::isCollision, j) > 0.0) {
                x /= (t(tally::isCollision, j) * f);
                dx /= (t(tally::isCollision, j) * f);
            }
            buff(0, i, j) = x;
            buff(1, i, j) = dx;
        }

        // lost ions
        i = 3;
        for (j = 0; j < cols; ++j) {
            x = t(tally::cL, j) * f;
            dx = dt(tally::cL, j) * f - x * x;
            dx = (dx > 0.0) ? std::sqrt(dx * f1) : 0.0;
            buff(0, i, j) = x;
            buff(1, i, j) = dx;
        }

        // 2nd pass
        // format and print data with error
        for (int i = 0; i < rows_; ++i) {
            for (int j = 0; j < cols; ++j) {
                double x = buff(0, i, j);
                double dx = buff(1, i, j);
                if (dx > 0.) {
                    tblWidget_->item(i, j)->setText(QString::fromStdString(
                            value_with_error(x, dx, 1, std::defaultfloat, true).to_string()));
                } else {
                    tblWidget_->item(i, j)->setText(QString::number(x, 'g'));
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
    tables_[idxDmgParTbl] = new dmg_parameters_table;
    tables_[idxIonStatTbl] = new ion_stat_table;
    for (int itbl = 0; itbl < idxNTbls; ++itbl) {
        data_table *tbl = tables_[itbl];
        tbl->create();
        tabWidget_->addTab(tbl->widget(), tbl->title());
    }

    QVBoxLayout *vbox = new QVBoxLayout;

    {
        QHBoxLayout *hbox = new QHBoxLayout;
        hbox->addWidget(simTitleLabel);
        hbox->addWidget(simTitle_);
        hbox->addStretch();
        vbox->addLayout(hbox);
    }
    vbox->addSpacing(V_SPACING);
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

    onTallyUpdate();
}

void TabularView::onSimulationDestroyed()
{
    for (int i = 0; i < idxNTbls; ++i)
        tables_[i]->clear();
}
