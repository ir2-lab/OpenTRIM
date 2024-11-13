#include "simulationoptionsview.h"
#include "periodictable.h"
#include "periodictablewidget.h"
#include "materialsdefview.h"
#include "regionsview.h"
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
#include <QDialogButtonBox>
#include <QPushButton>
#include <QSpinBox>
#include <QWhatsThis>
#include <QAction>
#include <QDataWidgetMapper>
#include <QLineEdit>

#include "jsedit/jsedit.h"

SimulationOptionsView::SimulationOptionsView(IonsUI *iui, QWidget *parent)
    : QWidget{parent}, ionsui(iui)
{
    tabWidget = new QTabWidget;

    QObjectList opts;
    QWidget* widget;
    QFormLayout* flayout;

    OptionsModel* model = ionsui->optionsModel;
    mapper = new MyDataWidgetMapper(model,this);

    QStringList categories, categoryNames;
    categories << "Simulation"
               << "Transport"
               << "IonBeam"
               << "Target"
               << "Output";

    categoryNames << "General"
               << "Ion Transport"
               << "Ion Source"
               << "Target"
               << "Output";

    for(int i=0; i<categories.count(); ++i)
    {
        QString category = categories.at(i);
        QString categoryName = categoryNames.at(i);

        QModelIndex idx1 = model->index(category);
        assert(idx1.isValid());
        widget = new QWidget;
        flayout = new QFormLayout;
        for(int j=0; j<model->rowCount(idx1); ++j) {
            QModelIndex idx2 = model->index(j,0,idx1);
            OptionsItem* item = model->getItem(idx2);
            QWidget* w = item->createEditor(widget);
            if (w) {
                QLabel* lbl = new QLabel(item->name(),widget);
                lbl->setToolTip(w->toolTip());
                lbl->setWhatsThis(w->whatsThis());
                mapper->addMapping(w,idx2,item->editorSignal());
                flayout->addRow(lbl,w);
            }
        }

        if (category == "IonBeam") {
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

            QHBoxLayout* hbox = new QHBoxLayout;
            hbox->addLayout(flayout);
            hbox->addStretch();
            widget->setLayout(hbox);
        }
        else if (category == "Target") {

            QTabWidget* innerTab = new QTabWidget;

            materialsView = new MaterialsDefView(model);
            innerTab->addTab(materialsView,"Materials");

            regionsView = new RegionsView(model);
            innerTab->addTab(regionsView,"Regions");

            //targetView = new TargetGeometryView(ionsui);
            //innerTab->addTab(targetView,"Regions");

            QHBoxLayout* hbox = new QHBoxLayout;
            hbox->addLayout(flayout);
            hbox->addStretch();

            QVBoxLayout* vbox = new QVBoxLayout;
            vbox->addLayout(hbox);
            vbox->addSpacing(20);
            vbox->addWidget(innerTab);
            widget->setLayout(vbox);

        } else {
            QHBoxLayout* hbox = new QHBoxLayout;
            hbox->addLayout(flayout);
            hbox->addStretch();
            widget->setLayout(hbox);

        }

        tabWidget->addTab(widget,categoryName);

    }

    // main title widget
    QLabel* simTitleLabel = new QLabel("Simulation title:");
    {
        QModelIndex idxOut = model->index("Output",0);
        QModelIndex idxTitle = model->index("title",0,idxOut);
        OptionsItem* item = model->getItem(idxTitle);
        simTitle = (QLineEdit*)item->createEditor(widget);
        simTitleLabel->setToolTip(simTitle->toolTip());
        simTitleLabel->setWhatsThis(simTitle->whatsThis());
        mapper->addMapping(simTitle,idxTitle,item->editorSignal());
        simTitleLabel->setStyleSheet("font-size : 14pt; font-weight : bold;");
        simTitle->setStyleSheet("font-size : 14pt");
    }



    jsonView = new JSEdit;
    jsonView->setReadOnly(true);
    const char* hlpmsg_json[] = {
        "Read-only view of current JSON configuration",
        "It is updated after clicking the Apply button"
    };
    jsonView->setToolTip(hlpmsg_json[0]);
    jsonView->setWhatsThis(QString("%1\n\n%2")
                               .arg(hlpmsg_json[0])
                               .arg(hlpmsg_json[1])
                           );
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

    /* Layout config page */
    QVBoxLayout* vbox = new QVBoxLayout;
    {
        QHBoxLayout* hbox = new QHBoxLayout;
        hbox->addWidget(simTitleLabel);
        hbox->addWidget(simTitle);
        hbox->addStretch();
        vbox->addLayout(hbox);
    }
    vbox->addWidget(tabWidget);
    vbox->addWidget(buttonBox);
    setLayout(vbox);
    vbox->setContentsMargins(0,0,0,0);

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



}

void SimulationOptionsView::submit()
{
    QJsonDocument& jsonOptions = ionsui->ions_driver->jsonOptions;
    jsonOptions = mapper->model()->jsonOptions();
    jsonView->setPlainText(jsonOptions.toJson(QJsonDocument::Indented));
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
    //treeView->setWidgetData();
    regionsView->revert();
    jsonView->setPlainText(jsonOptions.toJson(QJsonDocument::Indented));

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


