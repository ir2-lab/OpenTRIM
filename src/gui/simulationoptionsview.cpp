#include "simulationoptionsview.h"
#include "periodictable.h"
#include "periodictablewidget.h"
#include "materialsdefview.h"
#include "targetgeometryview.h"
#include "optionsmodel.h"
#include "mydatawidgetmapper.h"
#include "ionsui.h"
#include "runview.h"

#include "mccore.h"

#include <QJsonObject>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QComboBox>
#include <QLabel>
#include <QTabWidget>
#include <QTreeView>
#include <QTextBrowser>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QSpinBox>
#include <QWhatsThis>
#include <QAction>
#include <QDataWidgetMapper>

SimulationOptionsView::SimulationOptionsView(IonsUI *iui, QWidget *parent)
    : QWidget{parent}, ionsui(iui)
{
    tabWidget = new QTabWidget;

    QObjectList opts;
    QWidget* widget;
    QFormLayout* flayout;

    OptionsModel* model = ionsui->optionsModel;
    mapper = new MyDataWidgetMapper(model,this);

//    treeView = new QTreeView;
//    treeView->setModel(model);
//    treeView->setItemDelegate(new OptionsItemDelegate(this));
//    treeView->setAlternatingRowColors(true);
//    treeView->setEditTriggers(QAbstractItemView::AllEditTriggers);
//    tabWidget->addTab(treeView,"Tree");

    QStringList categories;
    categories << "General"
               << "Ion Transport"
               << "Ion Source"
               << "Output";

    for(int i=0; i<categories.count(); ++i)
    {
        QString category = categories.at(i);

        QModelIndex idx1 = model->index(i,0);
        assert(idx1.isValid());
        widget = new QWidget;
        flayout = new QFormLayout;
        for(int j=0; j<model->rowCount(idx1); ++j) {
            QModelIndex idx2 = model->index(j,0,idx1);
            OptionsItem* item = model->getItem(idx2);
            QWidget* w = item->createEditor(widget);
            QLabel* lbl = new QLabel(item->name(),widget);
            lbl->setToolTip(w->toolTip());
            lbl->setWhatsThis(w->whatsThis());
            mapper->addMapping(w,idx2,item->editorSignal());
            flayout->addRow(lbl,w);
        }
        QHBoxLayout* hbox = new QHBoxLayout;
        hbox->addLayout(flayout);
        hbox->addStretch();
        widget->setLayout(hbox);


        tabWidget->addTab(widget,category);

        if (category == "Ion Source") {
            QWidget* w = mapper->findWidget("ionZ");
            int row;
            QFormLayout::ItemRole role;
            flayout->getWidgetPosition(w,
                                       &row,
                                       &role);
            QSpinBox* sb = qobject_cast<QSpinBox*>(w);
            if (sb)
                sb->setReadOnly(true);

            QPushButton* button = new QPushButton("Select Ion");
            const Element::Isotope& H1 = PeriodicTable::at(1).isotopes()[0];
            ionLabel = new QLabel(H1.symbol);
            flayout->insertRow(row,(QWidget*)button,ionLabel);
            connect(button,&QPushButton::clicked,
                    this, &SimulationOptionsView::selectIonZ);
        }
    }

    materialsView = new MaterialsDefView(model);
    tabWidget->addTab(materialsView,"Materials");

    targetView = new TargetGeometryView(ionsui);
    tabWidget->addTab(targetView,"Geometry");

    jsonView = new QTextBrowser;
    tabWidget->addTab(jsonView, "JSON");

    QDialogButtonBox* buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Apply | QDialogButtonBox::Cancel | QDialogButtonBox::Help,
        Qt::Horizontal);

    QPushButton* btValidate = new QPushButton(QIcon(":/icons/assets/ionicons/checkmark-done-outline.svg"),
                                              "Validate");
    buttonBox->addButton(btValidate, QDialogButtonBox::ActionRole);
    connect(btValidate, &QPushButton::clicked,
            this, &SimulationOptionsView::validateOptions);


    helpButton = buttonBox->button(QDialogButtonBox::Help);
    whatsThisAction = QWhatsThis::createAction(this);

    helpButton->setText(whatsThisAction->text());
    helpButton->setStatusTip(whatsThisAction->statusTip());
    helpButton->setToolTip(whatsThisAction->toolTip());
    helpButton->setIcon(whatsThisAction->icon());
    helpButton->setCheckable(whatsThisAction->isCheckable());

    // connect the button to the slot that forwards the
    // signal to the action
    connect(helpButton, &QPushButton::clicked,
            whatsThisAction, &QAction::trigger);

    // connect the action and the button
    // so that when the action is changed the
    // button is changed too!
    connect(whatsThisAction, &QAction::changed,
            this, &SimulationOptionsView::help);


    QPushButton* ba = buttonBox->button(QDialogButtonBox::Apply);
    QPushButton* br = buttonBox->button(QDialogButtonBox::Cancel);
    ba->setEnabled(false); br->setEnabled(false);

    connect(ba, &QPushButton::clicked, this, &SimulationOptionsView::submit);
    connect(br, &QPushButton::clicked, this, &SimulationOptionsView::revert);


    connect(this, &SimulationOptionsView::modifiedChanged,
            ba, &QPushButton::setEnabled); //, Qt::QueuedConnection);
    connect(this, &SimulationOptionsView::modifiedChanged,
            br, &QPushButton::setEnabled);

    connect(model, &OptionsModel::dataChanged,
            this, &SimulationOptionsView::setModified2);

    QVBoxLayout* vlayout = new QVBoxLayout;
    vlayout->addWidget(tabWidget);
    vlayout->addWidget(buttonBox);
    setLayout(vlayout);
    vlayout->setContentsMargins(0,0,0,0);

}

void SimulationOptionsView::submit()
{
    QJsonDocument& jsonOptions = ionsui->ions_driver->jsonOptions;
    jsonOptions = mapper->model()->jsonOptions();
    jsonView->setText(jsonOptions.toJson(QJsonDocument::Indented));
    setModified(false);
    emit optionsChanged();
}

void SimulationOptionsView::help()
{
    helpButton->setEnabled(whatsThisAction->isEnabled());
    helpButton->setChecked(whatsThisAction->isChecked());
}

void SimulationOptionsView::revert()
{
    const QJsonDocument& jsonOptions = ionsui->ions_driver->jsonOptions;
    mapper->model()->setJsonOptions(jsonOptions);
    mapper->revert();
    ionsui->runView->revert();
    materialsView->setWidgetData();
    targetView->setWidgetData();
    jsonView->setText(jsonOptions.toJson(QJsonDocument::Indented));

//    dynamic_cast<OptionsModel*>(treeView->model())->setJsonOptions(json);
//    treeView->expandAll();
//    treeView->resizeColumnToContents(0);
//    treeView->resizeColumnToContents(1);
//    treeView->collapseAll();

    applyRules();
    setModified(false);
}

void SimulationOptionsView::applyRules()
{
    OptionsModel* model = mapper->model();
    QModelIndex idx1 = model->index("Simulation");
    QModelIndex idx2 = model->index("eloss_calculation",1,idx1);
    int i = model->data(idx2, Qt::EditRole).toInt();
    bool b = i == mccore::EnergyLossAndStraggling;
    QWidget* w = mapper->findWidget("straggling_model");
    if (w) w->setEnabled(b);
}

void SimulationOptionsView::selectIonZ()
{
    PeriodicTableDialog dlg(true);
    if(dlg.exec()==QDialog::Accepted) {
        OptionsModel* model = mapper->model();
        QModelIndex idx1 = model->index("IonBeam");
        QModelIndex idx2 = model->index("ionZ",0,idx1);
        model->setData(idx2,dlg.selectedZ());
        idx2 = model->index("ionM",0,idx1);
        model->setData(idx2,dlg.selectedMass());
        ionLabel->setText(dlg.selectedIonSymbol());
    }
}

void SimulationOptionsView::validateOptions()
{
    ionsui->ions_driver->validateOptions();
}


