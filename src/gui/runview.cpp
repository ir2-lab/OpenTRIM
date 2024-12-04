#include "runview.h"
#include "ionsui.h"
#include "mydatawidgetmapper.h"
#include "optionsmodel.h"

#include "tally.h"

#include "error_fmt.h"

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

RunView::RunView(IonsUI *iui, QWidget *parent)
    : QWidget{parent}, ionsui(iui)
{
    /* Create & Map widgets to OptionsModel */

    OptionsModel* model = ionsui->optionsModel;
    mapper = new MyDataWidgetMapper(model,this);

    // main title widget
    QLabel* simTitleLabel = new QLabel("Simulation title:");
    {
        QModelIndex idxOut = model->index("Output",0);
        QModelIndex idxTitle = model->index("title",0,idxOut);
        OptionsItem* item = model->getItem(idxTitle);
        simTitle = (QLineEdit*)item->createEditor(this);
        simTitle->setReadOnly(true);
        simTitleLabel->setToolTip(simTitle->toolTip());
        simTitleLabel->setWhatsThis(simTitle->whatsThis());
        mapper->addMapping(simTitle,idxTitle,item->editorSignal());
        simTitleLabel->setStyleSheet("font-size : 14pt; font-weight : bold;");
        simTitle->setStyleSheet("font-size : 14pt");
    }

    // These /Driver/xxx items are NOT connected to mapper
    // because they can be overriden every time we run the sim
    QModelIndex driverOptionsIdx = model->index("Driver");
    QModelIndex idx = model->index("max_no_ions",0,driverOptionsIdx);
    OptionsItem* item = model->getItem(idx);
    sbIons = (QSpinBox *)item->createEditor(this);

    idx = model->index("threads",0,driverOptionsIdx);
    item = model->getItem(idx);
    sbNThreads = (QSpinBox *)item->createEditor(this);

    idx = model->index("seed",0,driverOptionsIdx);
    item = model->getItem(idx);
    sbSeed = (QSpinBox *)item->createEditor(this);

    QModelIndex outputOptionsIdx = model->index("Output");
    idx = model->index("storage_interval",0,outputOptionsIdx);
    item = model->getItem(idx);
    sbUpdInterval = (QSpinBox *)item->createEditor(this);

    /* Create Info items */

    int ntotals = tally::std_tallies - 1;
    QStringList totalLabels;
    QColor clr = palette().color(QPalette::Window);
    QString styleSheet = QString("background: %1").arg(clr.name());
    for(int i=0; i<ntotals; ++i) {
        QLineEdit* edt = new QLineEdit;
        edt->setReadOnly(true);
        edt->setStyleSheet(styleSheet);
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

    box3 = new QGroupBox("Simulation Results");
    {
        int k=0;
        int Nrow[] = {5, 7, 6};
        const char* titles[] = {"Point Defects", "Energy Loss", "Damage"};
        const char* units[] = {"(counts/ion)", "(eV/ion)", "(eV/ion)"};
        QHBoxLayout* hbox = new QHBoxLayout;
        for(int i=0; i<3; ++i) {
            QFormLayout* flayout = new QFormLayout;
            flayout->addRow(new QLabel(QString("%1 %2").arg(titles[i]).arg(units[i])));
            for(int j=0; j<Nrow[i]; ++j) {
                flayout->addRow(totalLabels.at(k),simTotals[k]);
                k++;
            }
            hbox->addLayout(flayout);
            if (i!=2) hbox->addSpacing(10);
        }
        box3->setLayout(hbox);
        box3->setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);
    }


    QVBoxLayout* vbox = new QVBoxLayout;
    {
        QHBoxLayout* hbox = new QHBoxLayout;
        hbox->addWidget(simTitleLabel);
        hbox->addWidget(simTitle);
        hbox->addStretch();
        vbox->addLayout(hbox);
    }
    vbox->addWidget(box1);
    //vbox->addWidget(box2);
    vbox->addWidget(box3);
    vbox->addStretch();
    setLayout(vbox);
    vbox->setContentsMargins(0,0,0,0);

    /* Connect Signals */

    bool ret = connect(ionsui->driverObj(), &McDriverObj::statusChanged,
            this, &RunView::onDriverStatusChanged, Qt::QueuedConnection);
    assert(ret);

    connect(ionsui->driverObj(), &McDriverObj::tallyUpdate,
            this, &RunView::onTallyUpdate, Qt::QueuedConnection);

//    connect(ionsui->driverObj(), &McDriverObj::simulationCreated,
//            this, &RunView::onTallyUpdate);

    connect(ionsui->driverObj(), &McDriverObj::configChanged,
            this, &RunView::revert);

    connect(sbIons,QOverload<int>::of(&QSpinBox::valueChanged),
            ionsui->driverObj(), &McDriverObj::setMaxIons);
    connect(sbNThreads,QOverload<int>::of(&QSpinBox::valueChanged),
            ionsui->driverObj(), &McDriverObj::setNThreads);
    connect(sbSeed,QOverload<int>::of(&QSpinBox::valueChanged),
            ionsui->driverObj(), &McDriverObj::setSeed);
    connect(sbUpdInterval,QOverload<int>::of(&QSpinBox::valueChanged),
            ionsui->driverObj(), &McDriverObj::setUpdInterval);
}

void RunView::revert()
{
    mapper->revert();

    McDriverObj* D = ionsui->driverObj();
    sbIons->setValue(D->maxIons());
    sbNThreads->setValue(D->nThreads());
    sbSeed->setValue(D->seed());
    sbUpdInterval->setValue(D->updInterval());
}

void RunView::onSimulationCreated()
{
    ionsui->optionsView->setEnabled(false);
}

void RunView::onTallyUpdate()
{
    auto T = ionsui->driverObj()->totals();
    auto dT = ionsui->driverObj()->dtotals();
    if (!T.isNull()) {
        for(int i=1; i<T.size(); ++i)
            simTotals[i-1]->setText(
                QString::fromStdString(print_with_err(T[i],dT[i],'g',1))
                );
    }
}

void RunView::onDriverStatusChanged()
{
    const McDriverObj* D = ionsui->driverObj();
    McDriverObj::DriverStatus st = D->status();

    box1->setEnabled(st != McDriverObj::mcRunning);

    sbSeed->setEnabled(st == McDriverObj::mcReset);

    if (st == McDriverObj::mcReset)
        for(auto edt : simTotals) edt->setText("");
}


