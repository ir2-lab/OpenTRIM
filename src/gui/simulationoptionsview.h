#ifndef SIMULATIONOPTIONSVIEW_H
#define SIMULATIONOPTIONSVIEW_H

#include <QWidget>

class QPushButton;
class QLabel;
class JSEdit;
class QTabWidget;
class QLineEdit;

class MyDataWidgetMapper;
class MaterialsDefView;
class RegionsView;
class OptionsModel;
class IonsUI;

class SimulationOptionsView : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(bool modified READ modified WRITE setModified NOTIFY modifiedChanged FINAL)

public:

    QAction* whatsThisAction;
    QPushButton* helpButton;
    QPushButton* btSelectIon;
    QLabel* ionLabel;
    JSEdit* jsonView;
    QTabWidget* tabWidget;
    QLineEdit* simTitle;
    MyDataWidgetMapper* mapper;
    MaterialsDefView* materialsView;
    RegionsView* regionsView;

    SimulationOptionsView(IonsUI *iui, QWidget *parent = nullptr);

    bool modified() const { return modified_; }

public slots:
    void setModified(bool b = true) {
        modified_ = b;
        applyRules();
        emit modifiedChanged(b);
    }
    void setModified2(const QModelIndex&, const QModelIndex&, const QVector<int>&) {
        setModified();
    }

signals:
    void modifiedChanged(bool);
    void optionsChanged();

public slots:
    void revert();
    void submit();
    void help();
    void selectIonZ();
    void validateOptions();
    void onDriverStatusChanged();

private:
    bool modified_{false};
    void applyRules();


    IonsUI* ionsui;
};

#endif // SIMULATIONOPTIONSVIEW_H
