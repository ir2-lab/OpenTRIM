#include "summaryview.h"

#include <algorithm>

#include "mainui.h"
#include "optionwidgetmapper.h"
#include "optionsmodel.h"
#include "mcdriverobj.h"
#include "tally.h"
#include "value_with_error.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QButtonGroup>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QFontMetrics>
#include <QFontDatabase>
#include <QToolButton>
#include <QMenu>
#include <QAction>
#include <QIcon>
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QApplication>
#include <QClipboard>
#include <QItemSelectionModel>
#include <QDebug>

// Fixed-width font used for the table's numeric data cells, so digits line
// up across rows/columns regardless of the application's default UI font.
static const QFont &tabularDataFont()
{
    static const QFont f = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    return f;
}

// A data_table represents one logical "section" (Damage Events, Energy Deposition,
// PKA damage, Ion Statistics) of the single shared QTableWidget owned by TabularView.
// Each section occupies a fixed row range: a title row, a unit-selector row, and its
// data rows. Columns are shared by all sections (one per atom + a trailing "Total"
// column); colShift_ says where a section's own column 0 lands in that shared space,
// so a section that doesn't use every column (e.g. PKA damage has no projectile
// column) simply never writes to the columns before its shift.
class data_table
{
protected:
    int rows_;
    int rowOffset_{ 0 };
    int titleRow_{ 0 };
    int buttonRow_{ 0 };
    int colShift_;
    QTableWidget *tbl_{ nullptr };
    QButtonGroup *unitSelector_{ nullptr };
    QString title_;
    ArrayNDd buff;

public:
    explicit data_table(const char *t, int r, int colShift = 0)
        : rows_(r), colShift_(colShift), title_(t)
    {
    }
    QButtonGroup *unitSelector() const { return unitSelector_; }
    const QString &title() const { return title_; }
    int rows() const { return rows_; }
    int rowOffset() const { return rowOffset_; }
    int titleRow() const { return titleRow_; }
    int buttonRow() const { return buttonRow_; }

    // Text describing the currently selected units, e.g. "Units: eV/ion".
    virtual QString unitsText() const
    {
        QAbstractButton *b = unitSelector_ ? unitSelector_->checkedButton() : nullptr;
        return b ? QString("Units: %1").arg(b->text()) : QString();
    }

    virtual const char *rowLabel(int i) const = 0;
    virtual QWidget *createButtonsWidget() = 0;
    virtual int dataColCount(int atomCount) const { return atomCount + 1; }

    // The unit-selector/label widget installed in the button row, or nullptr
    // before attach() has run.
    QWidget *buttonsWidget() const { return tbl_ ? tbl_->cellWidget(buttonRow_, 0) : nullptr; }

    // Installs this section's title label and unit-selector row into the shared
    // table at fixed rows, and sets the vertical header labels for its data rows.
    void attach(QTableWidget *tbl, int titleRow, int buttonRow, int rowOffset)
    {
        tbl_ = tbl;
        titleRow_ = titleRow;
        buttonRow_ = buttonRow;
        rowOffset_ = rowOffset;

        tbl_->setVerticalHeaderItem(titleRow_, new QTableWidgetItem());
        tbl_->setVerticalHeaderItem(buttonRow_, new QTableWidgetItem());
        for (int i = 0; i < rows_; ++i)
            tbl_->setVerticalHeaderItem(rowOffset_ + i, new QTableWidgetItem(rowLabel(i)));

        QLabel *lbl = new QLabel(title_);
        MainUI::setHeadingFont(lbl, 3);
        lbl->setAutoFillBackground(true);
        lbl->setStyleSheet("background-color: palette(alternate-base); padding: 3px;");
        tbl_->setCellWidget(titleRow_, 0, lbl);

        tbl_->setCellWidget(buttonRow_, 0, createButtonsWidget());
    }

    virtual void init(int atomCount)
    {
        if (!tbl_)
            return;

        int cols = dataColCount(atomCount);
        for (int i = 0; i < rows_; ++i)
            for (int j = 0; j < cols; ++j) {
                QTableWidgetItem *item = new QTableWidgetItem();
                item->setFont(tabularDataFont());
                tbl_->setItem(rowOffset_ + i, colShift_ + j, item);
            }

        buff = ArrayNDd(2, rows_, cols);
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
    virtual QWidget *createButtonsWidget() override
    {
        unitSelector_ = new QButtonGroup(tbl_);
        QPushButton *bt = new QPushButton("eV/ion");
        bt->setCheckable(true);
        bt->setChecked(true);
        unitSelector_->addButton(bt, tblErg);
        bt = new QPushButton("Percent (%)");
        bt->setCheckable(true);
        bt->setChecked(false);
        unitSelector_->addButton(bt, tblPercent);

        QWidget *w = new QWidget;
        QHBoxLayout *hbox = new QHBoxLayout(w);
        hbox->setContentsMargins(4, 2, 4, 2);
        hbox->setSpacing(0);
        hbox->addWidget(new QLabel("Units: "));
        for (QAbstractButton *b : unitSelector_->buttons())
            hbox->addWidget(b);
        hbox->addStretch();
        return w;
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
                QTableWidgetItem *item = tbl_->item(rowOffset_ + i, colShift_ + j);
                item->setData(Qt::UserRole, x);
                if (dx > 0.) {
                    dx = std::sqrt(dx) * f2;
                    item->setText(QString::fromStdString(
                            value_with_error(x, dx, 1, std::defaultfloat, true).to_string()));
                } else {
                    item->setText(QString::number(x, 'g'));
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
    virtual QWidget *createButtonsWidget() override
    {
        unitSelector_ = new QButtonGroup(tbl_);
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

        QWidget *w = new QWidget;
        QHBoxLayout *hbox = new QHBoxLayout(w);
        hbox->setContentsMargins(4, 2, 4, 2);
        hbox->setSpacing(0);
        hbox->addWidget(new QLabel("Units: "));
        for (QAbstractButton *b : unitSelector_->buttons())
            hbox->addWidget(b);
        hbox->addStretch();
        return w;
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
                QTableWidgetItem *item = tbl_->item(rowOffset_ + i, colShift_ + j);
                item->setData(Qt::UserRole, x);
                if (dx > 0.) {
                    dx = std::sqrt(dx) * f2;
                    item->setText(QString::fromStdString(
                            value_with_error(x, dx, 1, std::defaultfloat, true).to_string()));
                } else {
                    item->setText(QString::number(x, 'g'));
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
    // colShift = 1: this section has no projectile column, so its own column 0
    // (first recoil) lands on the shared table's column 1; its own last column
    // (Total) lands exactly on the shared Total column.
    dmg_parameters_table() : data_table("PKA damage", drows, 1) { }

    virtual const char *rowLabel(int i) const override { return rowLabels_[i]; }
    virtual int dataColCount(int atomCount) const override { return atomCount; }
    virtual QWidget *createButtonsWidget() override
    {
        unitSelector_ = new QButtonGroup(tbl_);
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

        QWidget *w = new QWidget;
        QHBoxLayout *hbox = new QHBoxLayout(w);
        hbox->setContentsMargins(4, 2, 4, 2);
        hbox->setSpacing(0);
        hbox->addWidget(new QLabel("Units: "));
        for (QAbstractButton *b : unitSelector_->buttons())
            hbox->addWidget(b);
        hbox->addStretch();
        return w;
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
                QTableWidgetItem *item = tbl_->item(rowOffset_ + i, colShift_ + j);
                item->setData(Qt::UserRole, x);
                if (dx > 0.) {
                    dx = std::sqrt(dx) * f2;
                    item->setText(QString::fromStdString(
                            value_with_error(x, dx, 1, std::defaultfloat, true).to_string()));
                } else {
                    item->setText(QString::number(x, 'g'));
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

    virtual const char *rowLabel(int i) const override { return rowLabels_[i]; }
    virtual int dataColCount(int atomCount) const override { return atomCount; }
    virtual QString unitsText() const override { return "Units: per ion"; }
    virtual QWidget *createButtonsWidget() override
    {
        QWidget *w = new QWidget;
        QHBoxLayout *hbox = new QHBoxLayout(w);
        hbox->setContentsMargins(4, 2, 4, 2);
        hbox->addWidget(new QLabel("Units: per ion"));
        hbox->addStretch();
        return w;
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
                QTableWidgetItem *item = tbl_->item(rowOffset_ + i, colShift_ + j);
                item->setData(Qt::UserRole, x);
                if (dx > 0.) {
                    item->setText(QString::fromStdString(
                            value_with_error(x, dx, 1, std::defaultfloat, true).to_string()));
                } else {
                    item->setText(QString::number(x, 'g'));
                }
            }
        }
    }
};

SummaryView::SummaryView(MainUI *ui, const QString &title, QWidget *parent)
    : Page(ui, title, true, parent), mainui_(ui)
{
    /* Create sections */

    tables_[idxErgTbl] = new erg_table;
    tables_[idxDmgEvntsTbl] = new dmg_events_table;
    tables_[idxDmgParTbl] = new dmg_parameters_table;
    tables_[idxIonStatTbl] = new ion_stat_table;

    int totalRows = 0;
    for (int i = 0; i < idxNTbls; ++i)
        totalRows += 2 + tables_[i]->rows();

    table_ = new QTableWidget(totalRows, 1);
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->setSortingEnabled(false);

    int row = 0;
    for (int i = 0; i < idxNTbls; ++i) {
        data_table *tbl = tables_[i];
        tbl->attach(table_, row, row + 1, row + 2);
        row += 2 + tbl->rows();
    }

    // Make all unit-selector buttons the same width: the widest one, initially.
    int maxBtnWidth = 0;
    for (int i = 0; i < idxNTbls; ++i) {
        QButtonGroup *bg = tables_[i]->unitSelector();
        if (!bg)
            continue;
        for (QAbstractButton *b : bg->buttons())
            maxBtnWidth = std::max(maxBtnWidth, b->sizeHint().width());
    }
    for (int i = 0; i < idxNTbls; ++i) {
        QButtonGroup *bg = tables_[i]->unitSelector();
        if (!bg)
            continue;
        for (QAbstractButton *b : bg->buttons())
            b->setFixedWidth(maxBtnWidth);
    }

    // Minimum total column width needed to show the widest unit-buttons row
    // (now that button widths are uniform) at its proper size.
    for (int i = 0; i < idxNTbls; ++i) {
        QWidget *w = tables_[i]->buttonsWidget();
        if (w)
            minRowWidth_ = std::max(minRowWidth_, w->sizeHint().width());
    }

    // Absolute minimum column width: must fit a fully-formatted value+error
    // string, e.g. "1.000000(2)e100", regardless of how few columns there are.
    minDataColWidth_ =
            QFontMetrics(tabularDataFont()).horizontalAdvance("1.000000(2)e100") + 12;

    // Initial empty state: a generic "Ion" column plus "Total".
    QStringList initialLbls;
    initialLbls << "Ion";
    rebuildColumns(1, initialLbls);

    QLabel *infoLbl = new QLabel(
            tr("Results for beam ion and target recoils, integrated over the simulation volume"));

    exportBtn_ = new QToolButton;
    exportBtn_->setText(tr("Export"));
    exportBtn_->setIcon(QIcon(":/assets/ionicons/download-outline.svg"));
    exportBtn_->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    exportBtn_->setPopupMode(QToolButton::InstantPopup);
    exportBtn_->setEnabled(false);

    QMenu *exportMenu = new QMenu(exportBtn_);
    QAction *exportFormattedAct = exportMenu->addAction(tr("Export to CSV, Formatted"));
    QAction *exportNumericAct = exportMenu->addAction(tr("Export to CSV, Numeric"));
    connect(exportFormattedAct, &QAction::triggered, this, [this]() { exportCsv(false); });
    connect(exportNumericAct, &QAction::triggered, this, [this]() { exportCsv(true); });
    exportBtn_->setMenu(exportMenu);

    QHBoxLayout *topHbox = new QHBoxLayout;
    topHbox->addWidget(infoLbl);
    topHbox->addStretch();
    topHbox->addWidget(exportBtn_);

    QVBoxLayout *vbox = new QVBoxLayout(content);
    vbox->addLayout(topHbox);
    vbox->addWidget(table_);
    vbox->setContentsMargins(0, 0, 0, 0);

    /* Context menu (active only when there is data and a selection) */

    table_->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(table_, &QTableWidget::customContextMenuRequested, this, [this](const QPoint &pos) {
        if (!hasData_ || table_->selectedItems().isEmpty())
            return;

        QMenu menu(table_);
        QAction *copyFormattedAct = menu.addAction(tr("Copy"));
        QAction *copyNumericAct = menu.addAction(tr("Copy Numeric"));
        QAction *chosen = menu.exec(table_->viewport()->mapToGlobal(pos));
        if (chosen == copyFormattedAct)
            copySelection(false);
        else if (chosen == copyNumericAct)
            copySelection(true);
    });

    /* Connect Signals */

    connect(mainui_->driverObj(), &McDriverObj::tallyUpdate, this, &SummaryView::onTallyUpdate,
            Qt::QueuedConnection);

    connect(mainui_->driverObj(), &McDriverObj::configChanged, this, &SummaryView::onConfigChanged);

    connect(mainui_->driverObj(), &McDriverObj::simulationDestroyed, this,
            &SummaryView::onSimulationDestroyed);

    for (int i = 0; i < idxNTbls; ++i) {
        if (tables_[i]->unitSelector())
            connect(tables_[i]->unitSelector(), &QButtonGroup::idToggled, this,
                    &SummaryView::onTallyUpdate);
    }
}

void SummaryView::onTallyUpdate()
{
    auto S = mainui_->driverObj()->getSim();
    if (!S) {
        setHasData(false);
        return;
    }

    auto T = S->getTallyTable(0);
    auto dT = S->getTallyTableVar(0);

    if (T.isNull()) {
        setHasData(false);
        return;
    }

    for (int i = 0; i < idxNTbls; ++i)
        tables_[i]->update(T, dT);

    // Matches the "N histories <= 1" guard each section uses internally: only
    // then has anything actually been written into the data cells.
    setHasData(T[0] > 1.0);
}

void SummaryView::setHasData(bool b)
{
    hasData_ = b;
    exportBtn_->setEnabled(hasData_);
}

void SummaryView::rebuildColumns(int atomCount, const QStringList &atomLabels)
{
    int masterCols = atomCount + 1;
    table_->setColumnCount(masterCols);

    QStringList lbls = atomLabels;
    lbls << "Total";
    table_->setHorizontalHeaderLabels(lbls);

    for (int i = 0; i < idxNTbls; ++i) {
        data_table *tbl = tables_[i];
        table_->setSpan(tbl->titleRow(), 0, 1, masterCols);
        table_->setSpan(tbl->buttonRow(), 0, 1, masterCols);
        tbl->init(atomCount);
    }

    // Equal-width columns, wide enough in total to show the widest
    // unit-buttons row at its proper (uniform-button) size, but never
    // narrower than what a fully-formatted data value needs.
    int colWidth = (minRowWidth_ + masterCols - 1) / masterCols;
    colWidth = std::max(colWidth, minDataColWidth_);
    for (int c = 0; c < masterCols; ++c)
        table_->setColumnWidth(c, colWidth);

    table_->resizeRowsToContents();
}

void SummaryView::onConfigChanged()
{
    const mcconfig &opt = mainui_->driverObj()->options();

    // Built from the config directly (whatever atoms/materials are currently
    // defined there), since no simulation/target may exist yet at this point.
    QStringList lbls;
    lbls << QString("%1 ion").arg(opt.IonBeam.ion.symbol.c_str());
    for (const auto &mat : opt.Target.materials)
        for (const auto &at : mat.composition)
            lbls << QString("%1 in %2").arg(at.element.symbol.c_str()).arg(mat.id.c_str());

    rebuildColumns(lbls.size(), lbls);

    onTallyUpdate();
}

void SummaryView::onSimulationDestroyed()
{
    // Deliberately a no-op: the table keeps showing the last computed values
    // until the next onConfigChanged() rebuilds it.
}

// Quotes a CSV field only if it needs it (contains a comma, quote, or newline).
static QString csvField(const QString &s)
{
    if (!s.contains(',') && !s.contains('"') && !s.contains('\n'))
        return s;
    QString q = s;
    q.replace('"', "\"\"");
    return '"' + q + '"';
}

QString SummaryView::buildExportText(bool numeric) const
{
    QStringList lines;
    int cols = table_->columnCount();

    QStringList header;
    header << QString();
    for (int c = 0; c < cols; ++c) {
        QTableWidgetItem *hdr = table_->horizontalHeaderItem(c);
        header << csvField(hdr ? hdr->text() : QString());
    }
    lines << header.join(',');

    for (int i = 0; i < idxNTbls; ++i) {
        data_table *tbl = tables_[i];

        QStringList titleRow;
        titleRow << csvField(tbl->title());
        for (int c = 1; c < cols; ++c)
            titleRow << QString();
        lines << titleRow.join(',');

        QStringList unitsRow;
        unitsRow << csvField(tbl->unitsText());
        for (int c = 1; c < cols; ++c)
            unitsRow << QString();
        lines << unitsRow.join(',');

        for (int r = 0; r < tbl->rows(); ++r) {
            QStringList row;
            row << csvField(tbl->rowLabel(r));
            for (int c = 0; c < cols; ++c) {
                QTableWidgetItem *item = table_->item(tbl->rowOffset() + r, c);
                if (!item) {
                    row << QString();
                    continue;
                }
                if (numeric) {
                    QVariant v = item->data(Qt::UserRole);
                    row << (v.isValid() ? QString::number(v.toDouble(), 'g', 10) : QString());
                } else {
                    row << csvField(item->text());
                }
            }
            lines << row.join(',');
        }
    }

    return lines.join('\n');
}

void SummaryView::exportCsv(bool numeric)
{
    QString path = QFileDialog::getSaveFileName(this, tr("Export table to CSV"), QString(),
                                                 tr("CSV files [*.csv](*.csv);; All files (*.*)"));
    if (path.isEmpty())
        return;
    if (QFileInfo(path).suffix().isEmpty())
        path += ".csv";

    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "TabularView: cannot write CSV file:" << path;
        return;
    }

    QTextStream os(&f);
    os << buildExportText(numeric);
}

void SummaryView::copySelection(bool numeric)
{
    QModelIndexList idxs = table_->selectionModel()->selectedIndexes();
    if (idxs.isEmpty())
        return;

    std::sort(idxs.begin(), idxs.end(), [](const QModelIndex &a, const QModelIndex &b) {
        return a.row() != b.row() ? a.row() < b.row() : a.column() < b.column();
    });

    QString text;
    int prevRow = idxs.first().row();
    for (const QModelIndex &idx : idxs) {
        if (idx.row() != prevRow) {
            text.chop(1);
            text += '\n';
            prevRow = idx.row();
        }
        QTableWidgetItem *item = table_->item(idx.row(), idx.column());
        if (numeric) {
            QVariant v = item ? item->data(Qt::UserRole) : QVariant();
            text += v.isValid() ? QString::number(v.toDouble(), 'g', 10) : QString();
        } else {
            text += item ? item->text() : QString();
        }
        text += '\t';
    }
    text.chop(1);

    QApplication::clipboard()->setText(text);
}
