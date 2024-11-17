#ifndef RUNVIEW_H
#define RUNVIEW_H

#include <QWidget>


class QLineEdit;
class QLabel;
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

    explicit RunView(IonsUI* iui, QWidget *parent = nullptr);

public slots:
    void revert();
    void onSimulationCreated();
    void onTallyUpdate();
    void onDriverStatusChanged();

signals:

private:

    IonsUI* ionsui;
    MyDataWidgetMapper* mapper;
    QLineEdit* simTitle;
    QGroupBox* box1;
    QGroupBox* box2;
    QGroupBox* box3;
    QSpinBox* sbIons;
    QSpinBox* sbNThreads;
    QSpinBox* sbSeed;
    QSpinBox* sbUpdInterval;
    std::vector<QLineEdit*> simTotals;

};



#endif // RUNVIEW_H
