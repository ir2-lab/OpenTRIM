#include "runview.h"
#include "ionsui.h"
#include "mydatawidgetmapper.h"
#include "optionsmodel.h"

#include "tally.h"

#include <QFile>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QProgressBar>
#include <QToolButton>
#include <QLineEdit>
#include <QSpinBox>
#include <QLabel>
#include <QButtonGroup>
#include <QMessageBox>

#include <QDebug>

RunView::RunView(IonsUI *iui, QWidget *parent)
    : QWidget{parent}, ionsui(iui)
{
    /* Create Controls */

    QSize bsz(32,32);
    startButton = new QToolButton;
    startButton->setIcon(QIcon(":/icons/assets/ionicons/play-circle-outline.svg"));
    startButton->setIconSize(bsz);
    startButton->setCheckable(true);
    startButton->setToolTip("Run");

    stopButton = new QToolButton;
    stopButton->setIcon(QIcon(":/icons/assets/ionicons/pause-circle-outline.svg"));
    stopButton->setIconSize(bsz);
    stopButton->setCheckable(true);
    stopButton->setToolTip("Pause");

    QButtonGroup* btg = new QButtonGroup(this);
    btg->addButton(startButton);
    btg->addButton(stopButton);

    resetButton = new QToolButton;
    resetButton->setIcon(QIcon(":/icons/assets/ionicons/refresh-circle-outline.svg"));
    resetButton->setIconSize(bsz);
    resetButton->setToolTip("Reset");

    /* Create & Map widgets to OptionsModel */

    OptionsModel* model = ionsui->optionsModel;
    //mapper = new MyDataWidgetMapper(model,this);

    QModelIndex driverOptionsIdx = model->index("Driver");
    QModelIndex idx = model->index("max_no_ions",0,driverOptionsIdx);
    OptionsItem* item = model->getItem(idx);
    sbIons = (QSpinBox *)item->createEditor(this);
    //mapper->addMapping(sbIons,idx,item->editorSignal());

    idx = model->index("threads",0,driverOptionsIdx);
    item = model->getItem(idx);
    sbNThreads = (QSpinBox *)item->createEditor(this);
    //mapper->addMapping(sbNThreads,idx,item->editorSignal());

    idx = model->index("seeds",0,driverOptionsIdx);
    item = model->getItem(idx);
    sbSeed = (QSpinBox *)item->createEditor(this);
    //mapper->addMapping(sbSeed,idx,item->editorSignal());

    QModelIndex outputOptionsIdx = model->index("Output");
    idx = model->index("storage_interval",0,outputOptionsIdx);
    item = model->getItem(idx);
    sbUpdInterval = (QSpinBox *)item->createEditor(this);
    //mapper->addMapping(sbUpdInterval,idx,item->editorSignal());

    /* Create Info items */

    QStringList itemLabels{ "Ion histories", "Ions/s", "Elapsed Time (s)", "ETC (s)"};

    for(int i=0; i<itemLabels.size(); ++i) {
        QLineEdit* edt = new QLineEdit;
        edt->setReadOnly(true);
        simInfoItems.push_back(edt);
    }

    int ntotals = tally::std_tallies - 1;
    QStringList totalLabels;
    for(int i=0; i<ntotals; ++i) {
        QLineEdit* edt = new QLineEdit;
        edt->setReadOnly(true);
        simTotals.push_back(edt);
        totalLabels << tally::arrayName(i+1);
    }

    /* Layout items */

    box1 = new QGroupBox("Run Settings");
    {
        QFormLayout* flayout1 = new QFormLayout;
        flayout1->addRow("Ions to simulate",sbIons);
        flayout1->addRow("# of Threads", sbNThreads);

        QFormLayout* flayout2 = new QFormLayout;
        flayout2->addRow("Random seed",sbSeed);
        flayout2->addRow("Update interval (s)",sbUpdInterval);

        QHBoxLayout* hbox = new QHBoxLayout;
        hbox->addLayout(flayout1);
        hbox->addSpacing(20);
        hbox->addLayout(flayout2);
        box1->setLayout(hbox);
        box1->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);
    }

    box2 = new QGroupBox("Simulation Control");
    {

        QHBoxLayout* hbox1 = new QHBoxLayout;
        hbox1->addWidget(startButton);
        hbox1->addWidget(stopButton);
        hbox1->addWidget(resetButton);
        hbox1->setSpacing(0);

        QVBoxLayout* vbox = new QVBoxLayout;
        vbox->addLayout(hbox1);
        vbox->addStretch();

        QFormLayout* flayout1 = new QFormLayout;
        flayout1->addRow(itemLabels.at(0),simInfoItems[0]);
        flayout1->addRow(itemLabels.at(1),simInfoItems[1]);

        QFormLayout* flayout2 = new QFormLayout;
        flayout2->addRow(itemLabels.at(2),simInfoItems[2]);
        flayout2->addRow(itemLabels.at(3),simInfoItems[3]);

        QHBoxLayout* hbox2 = new QHBoxLayout;
        hbox2->addLayout(vbox);
        hbox2->addSpacing(20);
        hbox2->addLayout(flayout1);
        hbox2->addSpacing(20);
        hbox2->addLayout(flayout2);
        box2->setLayout(hbox2);
        box2->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);
    }

    box3 = new QGroupBox("Simulation Results (Values per ion)");
    {
        int k=0;
        QHBoxLayout* hbox = new QHBoxLayout;
        for(int i=0; i<3; ++i) {
            QFormLayout* flayout = new QFormLayout;
            for(int j=0; j<6; ++j)
                flayout->addRow(totalLabels.at(j+i*6),simTotals[j+i*6]);
            hbox->addLayout(flayout);
            if (i!=2) hbox->addStretch(10);
        }
        box3->setLayout(hbox);
        box3->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);
    }


    QVBoxLayout* vbox = new QVBoxLayout;
    vbox->addWidget(box1);
    vbox->addWidget(box2);
    vbox->addWidget(box3);
    vbox->addStretch();
    setLayout(vbox);
    vbox->setContentsMargins(0,0,0,0);

    /* create my timer */
    runTimer = new QTimer;

    /* Connect Signals */
    connect(startButton, &QToolButton::toggled, this, &RunView::start);
    connect(resetButton, &QToolButton::clicked, this, &RunView::reset);
    connect(ionsui->ions_driver, &McDriverObj::statusUpdate,
            this, &RunView::onUpdateView, Qt::QueuedConnection);
    connect(ionsui->ions_driver, &McDriverObj::tallyUpdate,
            this, &RunView::onTallyUpdate, Qt::QueuedConnection);
    connect(runTimer, &QTimer::timeout, this, &RunView::onRunTimer);

}

size_t RunView::max_ions() const
{
    return sbIons->value();
}

int RunView::nthreads() const
{
    return sbNThreads->value();
}

unsigned int RunView::seed() const
{
    return sbSeed->value();
}

size_t RunView::updInterval() const
{
    return sbUpdInterval->value();
}

void RunView::revert()
{
    //mapper->revert();

    OptionsModel* model = ionsui->optionsModel;
    QModelIndex driverOptionsIdx = model->index("Driver");
    QModelIndex idx = model->index("max_no_ions",0,driverOptionsIdx);
    OptionsItem* item = model->getItem(idx);
    sbIons->setValue(item->value().toUInt());
    idx = model->index("threads",0,driverOptionsIdx);
    item = model->getItem(idx);
    sbNThreads->setValue(item->value().toInt());
    idx = model->index("seeds",0,driverOptionsIdx);
    item = model->getItem(idx);
    sbSeed->setValue(item->value().toUInt());
    QModelIndex outputOptionsIdx = model->index("Output");
    idx = model->index("storage_interval",0,outputOptionsIdx);
    item = model->getItem(idx);
    sbUpdInterval->setValue(item->value().toUInt());
}

void RunView::start(bool b)
{
    McDriverObj* D = ionsui->ions_driver;
    McDriverObj::DriverStatus st = D->status();
    if (b == (st == McDriverObj::mcRunning)) return;
    ionsui->ions_driver->start(b);
    if (b) {
        runTimer->start(100);
        box1->setEnabled(false);
        resetButton->setEnabled(false);
    } else runTimer->stop();
}

void RunView::reset()
{
    int ret = QMessageBox::warning(ionsui, "Reset",
                         "Reset simulation?\n"
                         "This will discard all data.",
                         QMessageBox::Ok | QMessageBox::Cancel);

    if (ret != QMessageBox::Ok) return;

    ionsui->ions_driver->reset();

    ionsui->statusLabel->setText("");
    ionsui->progressBar->setValue(0);
    for(auto edt : simInfoItems) edt->setText("");
    for(auto edt : simTotals) edt->setText("");

    startButton->setChecked(false);
    resetButton->setEnabled(false);

    box1->setEnabled(true);
    ionsui->optionsView->setEnabled(true);
}

QString mytimefmt(double t)
{
    unsigned long ti = t;
    unsigned long s = ti % 60;
    ti = (ti-s)/60;
    unsigned long m = ti % 60;
    ti = (ti-m)/60;
    return QString("%1:%2:%3")
        .arg(ti,2,10,QChar('0'))
        .arg(m,2,10,QChar('0'))
        .arg(s,2,10,QChar('0'));
}

void RunView::onUpdateView()
{
    //QString cool_chars = "⣾⣽⣻⢿⡿⣟⣯⣷";
    QString cool_chars = "⣾⣷⣯⣟⡿⢿⣻⣽";
    static int k = 0;

    const McDriverObj* D = ionsui->ions_driver;
    McDriverObj::DriverStatus st = D->status();

    bool isRunning = st == McDriverObj::mcRunning;

    ionsui->progressBar->setValue(D->progress());
    simInfoItems[0]->setText(QString::number(D->nions()));
    simInfoItems[1]->setText(QString::number(D->ips()));
    simInfoItems[2]->setText(mytimefmt(D->elapsed()));
    simInfoItems[3]->setText(mytimefmt(D->eta()));


    if (isRunning) {

        ionsui->statusLabel->setText(
            QString("Running %1").arg(cool_chars[k++ & 7])
            );

    } else {

        ionsui->statusLabel->setText("Stopped");

        box1->setEnabled(true);
        if (startButton->isChecked())
            stopButton->setChecked(true);

        runTimer->stop();

        resetButton->setEnabled(true);
    }
}

void RunView::onRunTimer()
{
    ionsui->ions_driver->update_run_data();
    onUpdateView();
}

void RunView::onSimulationCreated()
{
    ionsui->optionsView->setEnabled(false);
}

void RunView::onTallyUpdate()
{
    auto T = ionsui->ions_driver->totals();
    if (!T.isNull()) {
        for(int i=1; i<T.size(); ++i)
            simTotals[i-1]->setText(QString::number(T[i]));
    }
}


