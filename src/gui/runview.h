#ifndef RUNVIEW_H
#define RUNVIEW_H

#include <QWidget>


class QLineEdit;
class QSpinBox;
class QProgressBar;
class QToolButton;
class QLabel;
class QGroupBox;
class QTimer;

class MyDataWidgetMapper;
class IonsUI;

class RunView : public QWidget
{
    Q_OBJECT

public:

    QGroupBox* box1;
    QGroupBox* box2;
    QGroupBox* box3;
    QSpinBox* sbIons;
    QSpinBox* sbNThreads;
    QSpinBox* sbSeed;
    QSpinBox* sbUpdInterval;
    QToolButton* startButton;
    QToolButton* stopButton;
    QToolButton* resetButton;
    std::vector<QLineEdit*> simInfoItems;
    std::vector<QLineEdit*> simTotals;
    QTimer* runTimer;

    explicit RunView(IonsUI* iui, QWidget *parent = nullptr);

    size_t max_ions() const;
    int nthreads() const;
    unsigned int seed() const;
    size_t updInterval() const;


public slots:
    void revert();
    void start(bool b);
    void reset();
    void onUpdateView();
    void onRunTimer();
    void onSimulationCreated();
    void onTallyUpdate();

signals:

private:

    IonsUI* ionsui;
    MyDataWidgetMapper* mapper;

};



#endif // RUNVIEW_H
