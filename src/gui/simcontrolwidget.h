#ifndef SIMCONTROLWIDGET_H
#define SIMCONTROLWIDGET_H

#include <QWidget>

class QLineEdit;
class QLabel;
class QSpinBox;
class QProgressBar;
class QToolButton;
class QTimer;

class McDriverObj;

class SimControlWidget : public QWidget
{
    Q_OBJECT
public:
    explicit SimControlWidget(McDriverObj* d, QWidget *parent = nullptr);

signals:

private slots:
    void onStart(bool b);
    void onReset();
    void onDriverStatusChanged();
    void onSimTimer();
    void onSimulationStarted(bool b);

private:
    McDriverObj* driver_;
    QToolButton* btStart;
    QToolButton* btReset;
    std::vector<QLineEdit*> simIndicators;
    QProgressBar* progressBar;
    QLabel* runIndicator;
    QTimer* simTimer;

};

#endif // SIMCONTROLWIDGET_H
