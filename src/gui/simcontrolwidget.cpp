#include "simcontrolwidget.h"

#include "mcdriverobj.h"

#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QLineEdit>
#include <QToolButton>
#include <QProgressBar>
#include <QLabel>
#include <QStyle>
#include <QMessageBox>

SimControlWidget::SimControlWidget(McDriverObj *d, QWidget *parent)
    : QWidget{parent}, driver_(d)
{
    /* Create Controls */
    QSize bsz(32,32);
    btStart = new QToolButton;
    btStart->setIcon(QIcon(":/icons/assets/ionicons/play-circle-outline.svg"));
    btStart->setIconSize(bsz);
    btStart->setCheckable(true);
    btStart->setToolTip("Run");

    btReset = new QToolButton;
    btReset->setIcon(QIcon(":/icons/assets/ionicons/refresh-circle-outline.svg"));
    btReset->setIconSize(bsz);
    btReset->setToolTip("Reset");
    btReset->setEnabled(false);

    progressBar = new QProgressBar;
    progressBar->setFormat("%p%");
    progressBar->setMinimum(0);
    progressBar->setMaximum(1000);

    runIndicator = new QLabel(QString("⣾"));
    runIndicator->setMinimumSize(QSize(40,40));
    runIndicator->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    runIndicator->setStyleSheet("font-size: 24px");

    /* Create Info items */

    QStringList indicatorLabels{ "Ions", "Ions/s", "Elapsed", "ETC"};
    QStringList typContent{ "10000000000000", "100000000", "00000:00:00", "00000:00:00"};

    QColor clr = palette().color(QPalette::Window);
    QString styleSheet = QString("background: %1").arg(clr.name());

    for(int i=0; i<indicatorLabels.size(); ++i) {
        QLineEdit* edt = new QLineEdit;
        edt->setReadOnly(true);
        //edt->setFrame(false);
        edt->setStyleSheet(styleSheet);
        edt->setMinimumWidth(fontMetrics().horizontalAdvance(typContent.at(i)));
        edt->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
        edt->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        simIndicators.push_back(edt);
    }

    /* Layout Widgets */
    QVBoxLayout* masterLayout = new QVBoxLayout;
    masterLayout->setContentsMargins(0,0,0,0);
    setLayout(masterLayout);
    { // hline
        QFrame* frm = new QFrame;
        frm->setFrameStyle(QFrame::Raised);
        frm->setFrameShape(QFrame::HLine);
        masterLayout->addWidget(frm);
    }
    {
        QHBoxLayout* hbox = new QHBoxLayout;
        hbox->setContentsMargins(9,0,9,9);
        { // buttons
            QHBoxLayout* hbox2 = new QHBoxLayout;
            hbox2->setContentsMargins(0,0,9,0);
            hbox2->addWidget(btStart);
            hbox2->addWidget(btReset);
            hbox2->addWidget(runIndicator);

            hbox2->setSpacing(0);
            hbox->addLayout(hbox2);
        }
        {
            QVBoxLayout* vbox = new QVBoxLayout;
            { // Indicators
                QHBoxLayout* hbox2 = new QHBoxLayout;
                for(int i=0; i<indicatorLabels.count(); ++i) {
                    hbox2->addWidget(new QLabel(indicatorLabels.at(i)));
                    hbox2->addWidget(simIndicators[i]);
                }
                hbox2->addStretch();
                vbox->addLayout(hbox2);
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

    bool ret = connect(driver_, &McDriverObj::statusChanged,
                       this, &SimControlWidget::onDriverStatusChanged, Qt::QueuedConnection);
    assert(ret);

}

void SimControlWidget::onStart(bool b)
{
    McDriverObj* D = driver_;
    McDriverObj::DriverStatus st = D->status();
     if (b) {
        // already running ?
        if (st == McDriverObj::mcRunning) return;
        // validate
        QString msg;
        bool ret = D->validateOptions(&msg);
        if (!ret) {
            QMessageBox::warning(window(),"Run Simulation",msg);
            btStart->setChecked(false);
            return;
        }
        D->start(b);
        simTimer->start(100);
        btStart->setIcon(QIcon(":/icons/assets/ionicons/pause-circle-outline.svg"));
    } else {
        D->start(b);
        simTimer->stop();
        btStart->setIcon(QIcon(":/icons/assets/ionicons/play-circle-outline.svg"));
        //runIndicator->setText("");
    }
}

void SimControlWidget::onReset()
{
    int ret = QMessageBox::warning(window(), "Reset",
                                   "Reset simulation?\n"
                                   "This will discard all data.",
                                   QMessageBox::Ok | QMessageBox::Cancel);

    if (ret != QMessageBox::Ok) return;

    McDriverObj* D = driver_;
    if (D->status() == McDriverObj::mcRunning)
        onStart(false);

    D->reset();

    progressBar->setValue(0);
    for(auto edt : simIndicators) edt->setText("");
}

void SimControlWidget::onDriverStatusChanged()
{
    const McDriverObj* D = driver_;
    McDriverObj::DriverStatus st = D->status();
    btReset->setEnabled(st != McDriverObj::mcReset);
    if (st != McDriverObj::mcRunning) {
        simTimer->stop();
        btStart->setIcon(QIcon(":/icons/assets/ionicons/play-circle-outline.svg"));
        if (btStart->isChecked()) btStart->setChecked(false);
        //runIndicator->setText("");
    }
    if (st == McDriverObj::mcReset) {
        for(auto edt : simIndicators) edt->setText("");
        progressBar->setValue(0);
    }
}

QString mytimefmt_(double t, bool ceil = false)
{
    unsigned long ti = ceil ?  std::ceil(t) : std::floor(t);
    unsigned long s = ti % 60;
    ti = (ti-s)/60;
    unsigned long m = ti % 60;
    ti = (ti-m)/60;
    return QString("%1:%2:%3")
        .arg(ti,2,10,QChar('0'))
        .arg(m,2,10,QChar('0'))
        .arg(s,2,10,QChar('0'));
}

void SimControlWidget::onSimTimer()
{
    driver_->update_run_data();

    QString cool_chars = "⣾⣷⣯⣟⡿⢿⣻⣽";
    static int k = 0;
    runIndicator->setText(QString(cool_chars[k++ & 7]));

    const McDriverObj* D = driver_;

    progressBar->setValue(D->progress());
    simIndicators[0]->setText(QString::number(D->nions()));
    simIndicators[1]->setText(QString::number(D->ips()));
    simIndicators[2]->setText(mytimefmt_(D->elapsed()));
    simIndicators[3]->setText(mytimefmt_(D->eta(),true));
}
