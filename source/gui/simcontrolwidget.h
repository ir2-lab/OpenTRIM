#ifndef SIMCONTROLWIDGET_H
#define SIMCONTROLWIDGET_H

#include <QWidget>

class QLineEdit;
class QLabel;
class QDoubleSpinBox;
class QSpinBox;
class QProgressBar;
class QToolButton;
class QTimer;

class McDriverObj;
class MainUI;

class SimControlWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SimControlWidget(MainUI *ui, QWidget *parent = nullptr);

signals:

private slots:
    void onStart(bool b);
    void onReset();
    void onDriverStatusChanged();
    void onSimTimer();
    void onSimulationStarted(bool b);
    void onSimulationCreated();
    void revert();

private:
    MainUI *mainui_;
    McDriverObj *driver_;
    QToolButton *btStart;
    QToolButton *btReset;
    QDoubleSpinBox *sbIons;
    QSpinBox *sbNThreads;
    QSpinBox *sbSeed;
    QSpinBox *sbUpdInterval;
    QSpinBox *sbMaxTime;
    std::vector<QWidget *> simCtrls;
    std::vector<QLineEdit *> simIndicators;
    QProgressBar *progressBar;
    QLabel *runIndicator;
    QTimer *simTimer;
};

#endif // SIMCONTROLWIDGET_H
