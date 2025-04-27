#include "simcontrolwidget.h"

#include "mcdriverobj.h"
#include "mainui.h"
#include "optionsmodel.h"
#include "simulationoptionsview.h"

#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGridLayout>
#include <QFrame>
#include <QLineEdit>
#include <QToolButton>
#include <QProgressBar>
#include <QSpinBox>
#include <QLabel>
#include <QStyle>
#include <QMessageBox>

SimControlWidget::SimControlWidget(MainUI *ui, QWidget *parent)
    : QWidget{ parent }, mainui_(ui), driver_(ui->driverObj())
{
    /* Create Controls */
    QSize bsz(32, 32);
    btStart = new QToolButton;
    btStart->setIcon(QIcon(":/assets/ionicons/play-circle-outline.png"));
    btStart->setIconSize(bsz);
    btStart->setCheckable(true);
    btStart->setToolTip("Run");

    btReset = new QToolButton;
    btReset->setIcon(QIcon(":/assets/ionicons/refresh-circle-outline.png"));
    btReset->setIconSize(bsz);
    btReset->setToolTip("Reset");
    btReset->setEnabled(false);

    progressBar = new QProgressBar;
    progressBar->setFormat("%p%");
    progressBar->setMinimum(0);
    progressBar->setMaximum(1000);

    runIndicator = new QLabel(QString("⣾"));
    runIndicator->setMinimumSize(QSize(40, 40));
    runIndicator->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    runIndicator->setStyleSheet("font-size: 24px");

    // These /Run/xxx items are NOT connected to mapper
    // because they can be overriden every time we run the sim
    OptionsModel *model = mainui_->optionsModel;
    QModelIndex driverOptionsIdx = model->index("Run");
    QModelIndex idx = model->index("max_no_ions", 0, driverOptionsIdx);
    OptionsItem *item = model->getItem(idx);
    sbIons = (QSpinBox *)item->createEditor(this);
    simCtrls.push_back(sbIons);

    idx = model->index("threads", 0, driverOptionsIdx);
    item = model->getItem(idx);
    sbNThreads = (QSpinBox *)item->createEditor(this);
    simCtrls.push_back(sbNThreads);

    QModelIndex outputOptionsIdx = model->index("Output");
    idx = model->index("storage_interval", 0, outputOptionsIdx);
    item = model->getItem(idx);
    sbUpdInterval = (QSpinBox *)item->createEditor(this);
    simCtrls.push_back(sbUpdInterval);

    idx = model->index("seed", 0, driverOptionsIdx);
    item = model->getItem(idx);
    sbSeed = (QSpinBox *)item->createEditor(this);
    simCtrls.push_back(sbSeed);

    /* Create Info items */

    QStringList ctrlLabels{ "Ions to run", "Threads", "Upd period (ms)", "Seed" };
    QStringList indicatorLabels{ "Ions finished", "Ions/s", "Elapsed", "ETC" };
    QStringList typContent{ "10000000000000000", "100000", "00000:00:00", "00000:00:00" };

    QColor clr = palette().color(QPalette::Window);
    QString styleSheet = QString("background: %1").arg(clr.name());

    for (int i = 0; i < indicatorLabels.size(); ++i) {
        QLineEdit *edt = new QLineEdit;
        edt->setReadOnly(true);
        // edt->setFrame(false);
        edt->setStyleSheet(styleSheet);
        edt->setMinimumWidth(fontMetrics().horizontalAdvance(typContent.at(i)));
        // edt->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
        // edt->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        simIndicators.push_back(edt);
    }

    /* Layout Widgets */
    QVBoxLayout *masterLayout = new QVBoxLayout;
    masterLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(masterLayout);
    { // hline
        QFrame *frm = new QFrame;
        frm->setFrameStyle(QFrame::Raised);
        frm->setFrameShape(QFrame::HLine);
        masterLayout->addWidget(frm);
    }
    {
        QHBoxLayout *hbox = new QHBoxLayout;
        hbox->setContentsMargins(9, 0, 9, 9);
        { // buttons
            QHBoxLayout *hbox2 = new QHBoxLayout;
            hbox2->setContentsMargins(0, 0, 9, 0);
            hbox2->addWidget(btStart);
            hbox2->addWidget(btReset);
            hbox2->addWidget(runIndicator);

            hbox2->setSpacing(0);
            hbox->addLayout(hbox2);
        }
        {
            QVBoxLayout *vbox = new QVBoxLayout;

            //            { // Indicators
            //                QHBoxLayout* hbox2 = new QHBoxLayout;
            //                for(int i=0; i<indicatorLabels.count(); ++i) {
            //                    hbox2->addWidget(new QLabel(indicatorLabels.at(i)));
            //                    hbox2->addWidget(simIndicators[i]);
            //                }
            //                hbox2->addStretch();
            //                vbox->addLayout(hbox2);
            //            }
            {
                QGridLayout *grid = new QGridLayout;
                vbox->addLayout(grid);

                int icol = 0;
                for (int i = 0; i < 4; i++) {
                    grid->addWidget(new QLabel(ctrlLabels.at(i)), 0, icol);
                    grid->addWidget(new QLabel(indicatorLabels.at(i)), 1, icol);
                    // grid->setColumnStretch(icol,1);
                    icol++;
                    grid->addWidget(simCtrls[i], 0, icol);
                    grid->addWidget(simIndicators[i], 1, icol);
                    int w = fontMetrics().horizontalAdvance(typContent.at(i));
                    grid->setColumnMinimumWidth(icol, w);
                    // grid->setColumnStretch(icol,100);
                    icol++;
                }
            }
            vbox->addWidget(progressBar);
            hbox->addLayout(vbox);
        }
        masterLayout->addLayout(hbox);
    }

    /* create my timer */
    simTimer = new QTimer;

    /* Connect signals / slots */

    connect(btStart, &QToolButton::toggled, this, &SimControlWidget::onStart);
    connect(btReset, &QToolButton::clicked, this, &SimControlWidget::onReset);

    connect(simTimer, &QTimer::timeout, this, &SimControlWidget::onSimTimer);

    connect(driver_, &McDriverObj::simulationCreated, this, &SimControlWidget::onSimulationCreated);

    bool ret = connect(driver_, &McDriverObj::statusChanged, this,
                       &SimControlWidget::onDriverStatusChanged, Qt::QueuedConnection);
    assert(ret);

    ret = connect(driver_, &McDriverObj::simulationStarted, this,
                  &SimControlWidget::onSimulationStarted, Qt::QueuedConnection);
    assert(ret);

    connect(driver_, &McDriverObj::configChanged, this, &SimControlWidget::revert);

    connect(sbIons, QOverload<int>::of(&QSpinBox::valueChanged), driver_, &McDriverObj::setMaxIons);
    connect(sbNThreads, QOverload<int>::of(&QSpinBox::valueChanged), driver_,
            &McDriverObj::setNThreads);
    connect(sbSeed, QOverload<int>::of(&QSpinBox::valueChanged), driver_, &McDriverObj::setSeed);
    connect(sbUpdInterval, QOverload<int>::of(&QSpinBox::valueChanged), driver_,
            &McDriverObj::setUpdInterval);
}

void SimControlWidget::onStart(bool b)
{
    McDriverObj::DriverStatus st = driver_->status();
    if (b) {
        // already running ?
        if (st == McDriverObj::mcRunning)
            return;

        // check if there are unsaved options
        if (mainui_->optionsView->modified()) {
            int ret =
                    QMessageBox::warning(window(), "Run Simulation",
                                         "Changes to some options have not been applied.\n"
                                         "Apply them before starting the simulation?",
                                         QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

            if (ret == QMessageBox::Yes) {
                mainui_->optionsView->submit();

            } else if (ret == QMessageBox::No) {

            } else {
                btStart->setChecked(false);
                return;
            }
        }
        // validate
        QString msg;
        bool ret = driver_->validateOptions(&msg);
        if (!ret) {
            QMessageBox::warning(window(), "Run Simulation", msg);
            btStart->setChecked(false);
            return;
        }
        driver_->start(b);
        // simTimer->start(100);
        btStart->setIcon(QIcon(":/assets/ionicons/pause-circle-outline.png"));
    } else {
        driver_->start(b);
        btStart->setIcon(QIcon(":/assets/ionicons/play-circle-outline.png"));
    }
}

void SimControlWidget::onReset()
{
    int ret = QMessageBox::warning(window(), "Reset",
                                   "Reset simulation?\n"
                                   "This will discard all data.",
                                   QMessageBox::Ok | QMessageBox::Cancel);

    if (ret != QMessageBox::Ok)
        return;

    McDriverObj *D = driver_;
    if (D->status() == McDriverObj::mcRunning)
        onStart(false);

    D->reset();

    progressBar->setValue(0);
    for (auto edt : simIndicators)
        edt->setText("");
}

void SimControlWidget::onDriverStatusChanged()
{
    const McDriverObj *D = driver_;
    McDriverObj::DriverStatus st = D->status();
    btReset->setEnabled(st != McDriverObj::mcReset);
    if (st != McDriverObj::mcRunning) {
        // simTimer->stop();
        btStart->setIcon(QIcon(":/assets/ionicons/play-circle-outline.png"));
        if (btStart->isChecked())
            btStart->setChecked(false);
        // runIndicator->setText("");
    }
    if (st == McDriverObj::mcReset) {
        for (auto edt : simIndicators)
            edt->setText("");
        progressBar->setValue(0);
    }
    sbIons->setEnabled(st != McDriverObj::mcRunning);
    sbNThreads->setEnabled(st != McDriverObj::mcRunning);
    sbUpdInterval->setEnabled(st != McDriverObj::mcRunning);
    sbSeed->setEnabled(st == McDriverObj::mcReset);
}

QString mytimefmt_(double t, bool ceil = false)
{
    unsigned long ti = ceil ? std::ceil(t) : std::floor(t);
    unsigned long s = ti % 60;
    ti = (ti - s) / 60;
    unsigned long m = ti % 60;
    ti = (ti - m) / 60;
    return QString("%1:%2:%3")
            .arg(ti, 2, 10, QChar('0'))
            .arg(m, 2, 10, QChar('0'))
            .arg(s, 2, 10, QChar('0'));
}

void SimControlWidget::onSimTimer()
{
    const McDriverObj::running_sim_info &info = driver_->sim_info();

    QString cool_chars = "⣾⣷⣯⣟⡿⢿⣻⣽";
    static int k = 0;
    runIndicator->setText(QString(cool_chars[k++ & 7]));

    progressBar->setValue(info.progress());
    simIndicators[0]->setText(QString::number(info.nions()));
    simIndicators[1]->setText(QString::number(info.ips()));
    simIndicators[2]->setText(mytimefmt_(info.elapsed()));
    simIndicators[3]->setText(mytimefmt_(info.etc(), true));
}

void SimControlWidget::onSimulationStarted(bool b)
{
    if (b) {
        simTimer->start(100);
    } else {
        simTimer->stop();
        onSimTimer();
    }
}

void SimControlWidget::onSimulationCreated()
{
    const McDriverObj::running_sim_info &info = driver_->sim_info();

    progressBar->setValue(info.progress());
    simIndicators[0]->setText(QString::number(info.nions()));
}

void SimControlWidget::revert()
{
    sbIons->setValue(driver_->maxIons());
    sbNThreads->setValue(driver_->nThreads());
    sbSeed->setValue(driver_->seed());
    sbUpdInterval->setValue(driver_->updInterval());
}
