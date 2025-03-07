#include "simulationoptionsview.h"
#include "periodic_table.h"
#include "periodictablewidget.h"
#include "materialsdefview.h"
#include "regionsview.h"
#include "optionsmodel.h"
#include "mydatawidgetmapper.h"
#include "mainui.h"
#include "mcdriverobj.h"

#include <QJsonObject>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QScrollArea>
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
#include <QMessageBox>

#include "jsedit/jsedit.h"

const char *closestIsotopeSymbol(int Z, double M)
{
    auto &elmnt = periodic_table::at(Z);

    if (elmnt.isotopes.empty())
        return elmnt.symbol.c_str();

    int i = 0;
    double diff = std::abs(elmnt.isotopes[i].mass - M);
    for (int j = 1; j < elmnt.isotopes.size(); ++j) {
        double d = std::abs(elmnt.isotopes[j].mass - M);
        if (d < diff) {
            i = j;
            diff = d;
        }
    }
    return elmnt.isotopes[i].symbol.c_str();
}

SimulationOptionsView::SimulationOptionsView(MainUI *iui, QWidget *parent)
    : QWidget{ parent }, ionsui(iui)
{
    tabWidget = new QTabWidget;

    OptionsModel *model = ionsui->optionsModel;
    mapper = new MyDataWidgetMapper(model, this);

    for (int i = 0; i < model->rowCount(); ++i) {
        QModelIndex idx = model->index(i, 0);
        OptionsItem *item = model->getItem(idx);

        QString category = item->key();
        if (category == "Driver")
            continue;

        QWidget *widget;
        if (category == "IonBeam")
            widget = createIonBeamTab(idx);
        else if (category == "Target")
            widget = createTargetTab(idx);
        else
            widget = createTab(idx);

        tabWidget->addTab(widget, item->name());
    }

    // main title widget
    QLabel *simTitleLabel = new QLabel("Simulation title:");
    {
        QModelIndex idxOut = model->index("Output", 0);
        QModelIndex idxTitle = model->index("title", 0, idxOut);
        OptionsItem *item = model->getItem(idxTitle);
        simTitle = (QLineEdit *)item->createEditor(nullptr);
        simTitleLabel->setToolTip(simTitle->toolTip());
        simTitleLabel->setWhatsThis(simTitle->whatsThis());
        mapper->addMapping(simTitle, idxTitle, item->editorSignal());
        simTitleLabel->setStyleSheet("font-size : 14pt; font-weight : bold;");
        simTitle->setStyleSheet("font-size : 14pt");
    }

    jsonView = new JSEdit;
    jsonView->setReadOnly(true);
    const char *hlpmsg_json[] = { "Read-only view of current JSON configuration",
                                  "It is updated after clicking the Apply button" };
    jsonView->setToolTip(hlpmsg_json[0]);
    jsonView->setWhatsThis(QString("%1\n\n%2").arg(hlpmsg_json[0]).arg(hlpmsg_json[1]));
    tabWidget->addTab(jsonView, "JSON");

    QDialogButtonBox *buttonBox = new QDialogButtonBox(
            QDialogButtonBox::Apply | QDialogButtonBox::Cancel | QDialogButtonBox::Help,
            Qt::Horizontal);

    QPushButton *btValidate = new QPushButton(
            QIcon(":/assets/ionicons/checkmark-done-outline.svg"), "Validate");
    buttonBox->addButton(btValidate, QDialogButtonBox::ActionRole);
    connect(btValidate, &QPushButton::clicked, this, &SimulationOptionsView::validateOptions);

    helpButton = buttonBox->button(QDialogButtonBox::Help);
    whatsThisAction = QWhatsThis::createAction(this);

    helpButton->setText(whatsThisAction->text());
    helpButton->setStatusTip(whatsThisAction->statusTip());
    helpButton->setToolTip(whatsThisAction->toolTip());
    helpButton->setIcon(whatsThisAction->icon());
    helpButton->setCheckable(whatsThisAction->isCheckable());

    /* Layout config page */
    QVBoxLayout *vbox = new QVBoxLayout;
    {
        QHBoxLayout *hbox = new QHBoxLayout;
        hbox->addWidget(simTitleLabel);
        hbox->addWidget(simTitle);
        hbox->addStretch();
        vbox->addLayout(hbox);
    }
    vbox->addSpacing(V_SPACING);
    vbox->addWidget(tabWidget);
    vbox->addWidget(buttonBox);
    setLayout(vbox);
    vbox->setContentsMargins(0, 0, 0, 0);

    // connect the button to the slot that forwards the
    // signal to the action
    connect(helpButton, &QPushButton::clicked, whatsThisAction, &QAction::trigger);

    // connect the action and the button
    // so that when the action is changed the
    // button is changed too!
    connect(whatsThisAction, &QAction::changed, this, &SimulationOptionsView::help);

    QPushButton *ba = buttonBox->button(QDialogButtonBox::Apply);
    QPushButton *br = buttonBox->button(QDialogButtonBox::Cancel);
    ba->setEnabled(false);
    br->setEnabled(false);

    connect(ba, &QPushButton::clicked, this, &SimulationOptionsView::submit);
    connect(br, &QPushButton::clicked, this, &SimulationOptionsView::revert);

    connect(this, &SimulationOptionsView::modifiedChanged, ba,
            &QPushButton::setEnabled); //, Qt::QueuedConnection);
    connect(this, &SimulationOptionsView::modifiedChanged, br, &QPushButton::setEnabled);

    connect(model, &OptionsModel::dataChanged, this, &SimulationOptionsView::setModified2);

    connect(ionsui->driverObj(), &McDriverObj::configChanged, this, &SimulationOptionsView::revert);

    bool ret = connect(ionsui->driverObj(), &McDriverObj::statusChanged, this,
                       &SimulationOptionsView::onDriverStatusChanged, Qt::QueuedConnection);
    assert(ret);
}

void SimulationOptionsView::submit()
{
    const mcdriver::options *opt = mapper->model()->options();
    ionsui->driverObj()->setOptions(*opt, false);
    jsonView->setPlainText(QString::fromStdString(ionsui->driverObj()->json()));
    setModified(false);
    // emit optionsChanged();
}

void SimulationOptionsView::help()
{
    helpButton->setEnabled(whatsThisAction->isEnabled());
    helpButton->setChecked(whatsThisAction->isChecked());
}

void SimulationOptionsView::revert()
{
    const mcdriver::options &opt = ionsui->driverObj()->options();
    mapper->model()->setOptions(opt);
    mapper->revert();
    materialsView->setWidgetData();
    regionsView->revert();
    jsonView->setPlainText(QString::fromStdString(ionsui->driverObj()->json()));

    // fix the isotope label
    ionLabel->setText(
            closestIsotopeSymbol(opt.IonBeam.ion.atomic_number, opt.IonBeam.ion.atomic_mass));

    applyRules();
    setModified(false);
}

void SimulationOptionsView::applyRules()
{
    // Apply option combination rules
    OptionsModel *model = mapper->model();

    QModelIndex parent, idx1, idx2;
    int i;
    bool b;
    QWidget *w;

    // Using a lambda expression for a short-lived function
    auto enable_if = [this](const QString &key, bool b) {
        QWidget *w = mapper->findWidget(key);
        if (w)
            w->setEnabled(b);
    };

    parent = model->index("Simulation");
    idx1 = model->index("eloss_calculation", 1, parent);
    i = model->data(idx1, Qt::EditRole).toInt();
    b = i == dedx_calc::EnergyLossAndStraggling;
    enable_if("/Simulation/straggling_model", b);

    parent = model->index("Transport");
    enable_if("/Transport/flight_path_const", false);
    enable_if("/Transport/max_mfp", false);
    enable_if("/Transport/allow_sub_ml_scattering", false);
    enable_if("/Transport/max_rel_eloss", false);
    enable_if("/Transport/min_recoil_energy", false);

    idx1 = model->index("flight_path_type", 1, parent);
    i = model->data(idx1, Qt::EditRole).toInt();
    auto fp_t = flight_path_calc::flight_path_type_t(i);
    switch (fp_t) {
    case flight_path_calc::AtomicSpacing:
        break;
    case flight_path_calc::Constant:
        enable_if("/Transport/flight_path_const", true);
        break;
    case flight_path_calc::MendenhallWeller:
        enable_if("/Transport/max_rel_eloss", true);
        enable_if("/Transport/min_recoil_energy", true);
        break;
    case flight_path_calc::IPP:
        enable_if("/Transport/max_rel_eloss", true);
        enable_if("/Transport/min_recoil_energy", true);
        enable_if("/Transport/max_mfp", true);
        enable_if("/Transport/allow_sub_ml_scattering", true);
        break;
    default:
        break;
    }

    // enable fwhm in all ion beam distributions
    parent = model->index("IonBeam");
    for (const char *key :
         { "energy_distribution", "angular_distribution", "spatial_distribution" }) {
        QModelIndex idx = model->index(key, 0, parent);
        idx = model->index("type", 1, idx);
        int i = model->data(idx, Qt::EditRole).toInt();
        enable_if(QString("/IonBeam/%1/fwhm").arg(key),
                  ion_beam::distribution_t(i) != ion_beam::SingleValue);
    }
}

QWidget *SimulationOptionsView::createIonBeamTab(const QModelIndex &parent)
{
    QWidget *widget = new QWidget;
    OptionsModel *model = ionsui->optionsModel;
    // QVBoxLayout* vbox = new QVBoxLayout;
    QGridLayout *grid = new QGridLayout;

    for (int i = 0; i < model->rowCount(parent); ++i) {
        QModelIndex idx = model->index(i, 0, parent); // model->index("ion");
        OptionsItem *item = model->getItem(idx);
        QGroupBox *box = new QGroupBox(item->name());
        QFormLayout *flayout = createForm(idx, box);
        box->setLayout(flayout);
        grid->addWidget(box, i >> 1, i % 2);

        if (item->key() == "ion") {
            // set atomic_number to read only
            QWidget *w = mapper->findWidget("/IonBeam/ion/atomic_number");
            QSpinBox *sb = qobject_cast<QSpinBox *>(w);
            if (sb)
                sb->setReadOnly(true);

            // add button to select ion
            btSelectIon = new QPushButton("Select Ion");
            auto &H1 = periodic_table::at(1).isotopes[0];
            ionLabel = new QLabel(H1.symbol.c_str());
            flayout->insertRow(0, (QWidget *)btSelectIon, ionLabel);
            connect(btSelectIon, &QPushButton::clicked, this, &SimulationOptionsView::selectIonZ);
        }
    }

    QVBoxLayout *vbox = new QVBoxLayout;
    widget->setLayout(vbox);
    vbox->addLayout(grid);
    vbox->addStretch();

    //    QScrollArea* scrollArea = new QScrollArea;
    //    scrollArea->setWidget(widget);

    //    return scrollArea;
    //    widget->setLayout(grid);
    return widget;
}

QWidget *SimulationOptionsView::createTargetTab(const QModelIndex &idx)
{
    OptionsModel *model = ionsui->optionsModel;
    QTabWidget *innerTab = new QTabWidget;

    materialsView = new MaterialsDefView(model);
    innerTab->addTab(materialsView, "Materials");

    regionsView = new RegionsView(model);
    innerTab->addTab(regionsView, "Regions");

    QWidget *widget = new QWidget;
    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->addLayout(createForm(idx, widget));
    hbox->addStretch();
    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addLayout(hbox);
    vbox->addSpacing(20);
    vbox->addWidget(innerTab);
    widget->setLayout(vbox);
    return widget;
}

QWidget *SimulationOptionsView::createTab(const QModelIndex &idx)
{
    QWidget *widget = new QWidget;
    QHBoxLayout *hbox = new QHBoxLayout;
    widget->setLayout(hbox);
    hbox->addLayout(createForm(idx, widget));
    hbox->addStretch();
    return widget;
}

QFormLayout *SimulationOptionsView::createForm(const QModelIndex &parent, QWidget *widgetParent)
{
    QFormLayout *flayout = new QFormLayout;
    OptionsModel *model = ionsui->optionsModel;
    for (int row = 0; row < model->rowCount(parent); ++row) {
        QModelIndex i = model->index(row, 0, parent);
        OptionsItem *item = model->getItem(i);
        QWidget *w = item->createEditor(widgetParent);
        if (w) {
            // QLabel* lbl = new QLabel(item->name());
            // lbl->setToolTip(w->toolTip());
            // lbl->setWhatsThis(w->whatsThis());
            mapper->addMapping(w, i, item->editorSignal());
            flayout->addRow(item->name(), w);
        }
    }
    return flayout;
}

void SimulationOptionsView::selectIonZ()
{
    PeriodicTableDialog dlg(true);
    if (dlg.exec() == QDialog::Accepted) {
        OptionsModel *model = mapper->model();
        QModelIndex i = model->index("IonBeam");
        i = model->index("ion", 0, i);
        OptionsItem *item = static_cast<OptionsItem *>(i.internalPointer());

        std::ostringstream os;
        os << "{ \"symbol\" : \"" << periodic_table::at(dlg.selectedZ()).symbol << "\", "
           << " \"atomic_number\" : " << dlg.selectedZ() << ", "
           << " \"atomic_mass\" : " << dlg.selectedMass() << "}";

        item->direct_set("/IonBeam/ion", os.str().c_str());

        QModelIndex j = model->index("atomic_number", 0, i);
        // model->setData(j,dlg.selectedZ());
        model->dataChanged(j, j, { Qt::EditRole });
        j = model->index("atomic_mass", 0, i);
        ;
        // model->setData(j,dlg.selectedMass());
        model->dataChanged(j, j, { Qt::EditRole });
        ionLabel->setText(dlg.selectedIonSymbol());
    }
}

void SimulationOptionsView::validateOptions()
{
    QString msg;
    bool ret = ionsui->driverObj()->validateOptions(&msg);
    if (!ret)
        QMessageBox::warning(ionsui, "Options validation", msg);
    else
        QMessageBox::information(ionsui, "Options validation", "Options are OK!");
}

void SimulationOptionsView::onDriverStatusChanged()
{
    McDriverObj *D = ionsui->driverObj();
    auto s = D->status();
    // activate/deactivate widgets depending on sim status
    bool isreset = s == McDriverObj::mcReset;
    mapper->setEnabled(isreset);
    btSelectIon->setEnabled(isreset);
    materialsView->setEnabled(isreset);
    regionsView->setEnabled(isreset);
    if (isreset)
        applyRules();
}
