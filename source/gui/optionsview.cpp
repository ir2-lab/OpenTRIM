#include "optionsview.h"
#include "floatlineedit.h"
#include "periodic_table.h"
#include "periodictablewidget.h"
#include "materialsdefview.h"
#include "regionsview.h"
#include "optionsmodel.h"
#include "optionwidgetmapper.h"
#include "mainui.h"
#include "mcdriverobj.h"
#include "simboxview.h"
#include "helppanel.h"

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
#include <QSplitter>
#include <QToolButton>
#include <QSignalBlocker>

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

OptionsView::OptionsView(MainUI *iui, QWidget *parent)
    : QWidget{ parent }, mainui(iui)
{
    tabWidget = new QTabWidget;

    OptionsModel *model = mainui->optionsModel;
    mapper = new OptionWidgetMapper(model, this);

    // need the helpPanel before the option widgets
    // are created, so that helpPanel->addStaticHelp()
    // can be called
    helpPanel = new HelpPanel;

    for (int i = 0; i < model->rowCount(); ++i) {
        QModelIndex idx = model->index(i, 0);
        OptionsItem *item = model->getItem(idx);

        QString category = item->key();
        if (category == "Run")
            continue;

        QWidget *widget = nullptr;
        if (category == "IonBeam")
            widget = createIonBeamTab(idx);
        else if (category == "Target")
            widget = createTargetTab(idx);
        else if (category == "UserTally") {
            // TODO !!
        } else
            widget = createTab(idx);

        if (widget)
            tabWidget->addTab(widget, item->name());
    }

    helpPanel->addStaticHelp(tabWidget,
                             { "/Simulation", "/Transport", "/IonBeam", "/Target", "/Output" });

    // main title widget
    QLabel *simTitleLabel = new QLabel("Simulation title:");
    {
        QModelIndex idxOut = model->index("Output", 0);
        QModelIndex idxTitle = model->index("title", 0, idxOut);
        OptionsItem *item = model->getItem(idxTitle);
        simTitle = (QLineEdit *)item->createEditor(nullptr);
        simTitleLabel->setToolTip(simTitle->toolTip());
        simTitleLabel->setWhatsThis(simTitle->whatsThis());
        mapper->addMapping(simTitle, idxTitle, true, item->editorSignal());
        mapper->addMapping(simTitleLabel, idxTitle, false);
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

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Apply | QDialogButtonBox::Cancel
                                             | QDialogButtonBox::Help,
                                     Qt::Horizontal);

    btValidate = new QPushButton(QIcon(":/assets/ionicons/checkmark-done-outline.svg"), "Validate");
    buttonBox->addButton(btValidate, QDialogButtonBox::ActionRole);
    connect(btValidate, &QPushButton::clicked, this, &OptionsView::validateOptions);

    // pass the mapper to the helpPanel to display help
    // for all editor widgets
    helpPanel->setWidgetMapper(mapper);

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
    {
        splitter = new QSplitter;
        QSizePolicy sizePolicy = splitter->sizePolicy();
        sizePolicy.setVerticalPolicy(QSizePolicy::Expanding);
        splitter->setSizePolicy(sizePolicy);
        splitter->addWidget(tabWidget);
        {
            QWidget *rightPanel = new QWidget;
            QVBoxLayout *vbox = new QVBoxLayout;
            rightPanel->setLayout(vbox);
            vbox->setContentsMargins(0, 0, 0, 0);
            vbox->setSpacing(0);
            QLabel *lbl = new QLabel("Configuration help");
            lbl->setStyleSheet("font-weight: bold;");
            int h = tabWidget->tabBar()->sizeHint().height();
            lbl->setMinimumHeight(h - 2);
            lbl->setFrameShape(QFrame::StyledPanel);
            lbl->setFrameShadow(QFrame::Raised);
            vbox->addWidget(lbl);
            vbox->addWidget(helpPanel);
            splitter->addWidget(rightPanel);

            helpToggle = new QToolButton;
            helpToggle->setIcon(QIcon(":/assets/ionicons/help-circle-outline.svg"));
            // helpToggle->setIconSize(tabWidget->tabBar()->iconSize());
            helpToggle->setCheckable(true);
            helpToggle->setChecked(true); // help panel starts visible
            helpToggle->setAutoRaise(true);
            helpToggle->setToolTip(tr("Show/Hide Help Panel"));
            tabWidget->setCornerWidget(helpToggle, Qt::TopRightCorner);

            // Button drives the splitter: open/close the right pane by resizing it
            connect(helpToggle, &QToolButton::toggled, this,
                    &OptionsView::toggleHelpPanel);

            // Dragging the splitter handle closed/open must keep the button in sync
            connect(splitter, &QSplitter::splitterMoved, this,
                    &OptionsView::onSplitterMoved);
        }
        splitter->setStretchFactor(0, 2);
        splitter->setStretchFactor(1, 1);
        splitter->setCollapsible(0, false);
        splitter->setCollapsible(1, true);
        splitter->setSizes({ 600, 300 });
        vbox->addWidget(splitter);
    }

    vbox->addWidget(buttonBox);
    setLayout(vbox);
    vbox->setContentsMargins(0, 0, 0, 0);

    QPushButton *ba = buttonBox->button(QDialogButtonBox::Apply);
    QPushButton *br = buttonBox->button(QDialogButtonBox::Cancel);
    helpButton = buttonBox->button(QDialogButtonBox::Help);
    ba->setEnabled(false);
    br->setEnabled(false);
    helpButton->setCheckable(true);
    helpButton->setChecked(true);
    helpButton->setToolTip("Show/Hide Help Panel");

    connect(ba, &QPushButton::clicked, this, &OptionsView::submit);
    connect(br, &QPushButton::clicked, this, &OptionsView::revert);
    connect(helpButton, &QPushButton::toggled, this, &OptionsView::toggleHelpPanel);

    connect(this, &OptionsView::modifiedChanged, ba,
            &QPushButton::setEnabled); //, Qt::QueuedConnection);
    connect(this, &OptionsView::modifiedChanged, br, &QPushButton::setEnabled);

    connect(model, &OptionsModel::dataChanged, this, &OptionsView::setModified2);

    connect(materialsView, &MaterialsDefView::materialsChanged, this,
            &OptionsView::drawSimBox);
    connect(regionsView, &RegionsView::regionsChanged, this, &OptionsView::drawSimBox);
    {
        VectorLineEdit *w = findChild<VectorLineEdit *>("/Target/size");
        if (w)
            connect(w, &VectorLineEdit::editingFinished, this, &OptionsView::drawSimBox);
        w = findChild<VectorLineEdit *>("/Target/origin");
        if (w)
            connect(w, &VectorLineEdit::editingFinished, this, &OptionsView::drawSimBox);
    }
    connect(mainui->driverObj(), &McDriverObj::configChanged, this, &OptionsView::revert);

    bool ret = connect(mainui->driverObj(), &McDriverObj::statusChanged, this,
                       &OptionsView::onDriverStatusChanged, Qt::QueuedConnection);
    assert(ret);
}

void OptionsView::submit()
{
    const mcconfig *opt = mapper->model()->options();
    mainui->driverObj()->setOptions(*opt, false);
    jsonView->setPlainText(QString::fromStdString(mainui->driverObj()->json()));
    setModified(false);
    // emit optionsChanged();
}

void OptionsView::revert()
{
    const mcconfig &opt = mainui->driverObj()->options();
    mapper->model()->setOptions(opt);
    mapper->revert();
    materialsView->setWidgetData();
    regionsView->revert();
    jsonView->setPlainText(QString::fromStdString(mainui->driverObj()->json()));

    // fix the isotope label
    ionLabel->setText(
            closestIsotopeSymbol(opt.IonBeam.ion.atomic_number, opt.IonBeam.ion.atomic_mass));

    applyRules();
    drawSimBox();

    setModified(false);
}

void OptionsView::applyRules()
{
    // Apply option combination rules

    // current unsaved mcconfig
    const mcconfig *opt = mapper->model()->options();

    // Using a lambda expression for a short-lived function
    auto enable_if = [this](const QString &key, bool b) {
        QWidget *w = mapper->findWidget(key);
        if (w)
            w->setEnabled(b);
    };

    enable_if("/Simulation/electronic_straggling",
              opt->Simulation.electronic_stopping != dedx_calc::electronic_stopping_t::Off);

    enable_if("/Transport/flight_path_const", false);
    enable_if("/Transport/mfp_range", false);
    enable_if("/Transport/max_rel_eloss", false);
    enable_if("/Transport/min_recoil_energy", false);
    enable_if("/Transport/min_scattering_angle", false);

    switch (opt->Transport.flight_path_type) {
    case flight_path_calc::Constant:
        enable_if("/Transport/flight_path_const", true);
        break;
    case flight_path_calc::Variable:
        enable_if("/Transport/max_rel_eloss", true);
        enable_if("/Transport/min_recoil_energy", true);
        enable_if("/Transport/min_scattering_angle", true);
        enable_if("/Transport/mfp_range", true);
        break;
    default:
        break;
    }

    // enable fwhm in all ion beam distributions
    enable_if("/IonBeam/energy_distribution/fwhm",
              opt->IonBeam.energy_distribution.type != ion_beam::SingleValue);
    enable_if("/IonBeam/angular_distribution/fwhm",
              opt->IonBeam.angular_distribution.type != ion_beam::SingleValue);
    enable_if("/IonBeam/spatial_distribution/fwhm",
              opt->IonBeam.spatial_distribution.type != ion_beam::SingleValue);
}

inline QVector3D qV3(vector3 v)
{
    return QVector3D(v.x(), v.y(), v.z());
}

void OptionsView::drawSimBox()
{
    // current unsaved mcconfig
    const mcconfig *opt = mapper->model()->options();
    const auto &t = opt->Target;

    std::unordered_map<std::string, int> mmap; // map material_id->index

    for (int i = 0; i < t.materials.size(); ++i) {
        auto md = t.materials[i];
        mmap[md.id] = i;
    }

    simBoxView->clear();
    simBoxView->setBox(qV3(t.origin), qV3(t.origin + t.size));
    for (const auto &r : t.regions) {
        auto it = mmap.find(r.material_id);
        QColor clr = (it == mmap.end()) ? Qt::transparent
                                        : QColor(t.materials[it->second].color.c_str());
        QString s = QString("Region: %1|Material: %2").arg(r.id.c_str()).arg(r.material_id.c_str());
        simBoxView->addRegion(qV3(r.origin), qV3(r.origin + r.size), clr.rgba(), s);
    }
}

QWidget *OptionsView::createIonBeamTab(const QModelIndex &parent)
{
    QWidget *widget = new QWidget;
    OptionsModel *model = mainui->optionsModel;
    QVBoxLayout *vbox = new QVBoxLayout;
    widget->setLayout(vbox);

    for (int i = 0; i < model->rowCount(parent); ++i) {
        QModelIndex idx = model->index(i, 0, parent); // model->index("ion");
        OptionsItem *item = model->getItem(idx);
        QGroupBox *box = new QGroupBox(item->name());
        helpPanel->addStaticHelp(box, item->path());
        QStringList excludeKeys;
        if (item->key() == "ion")
            excludeKeys << "symbol";
        QFormLayout *flayout = createForm(idx, box, excludeKeys);
        box->setLayout(flayout);
        vbox->addWidget(box); //, i >> 1, i % 2);

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
            connect(btSelectIon, &QPushButton::clicked, this, &OptionsView::selectIonZ);
            QModelIndex idx1 = model->index(0, 0, idx);
            mapper->addMapping(btSelectIon, idx1, false);
            mapper->addMapping(ionLabel, idx1, false);
        }
    }

    vbox->addStretch();

    QScrollArea *sa = new QScrollArea;
    sa->setWidget(widget);

    return sa;
}

QWidget *OptionsView::createTargetTab(const QModelIndex &idx)
{
    OptionsModel *model = mainui->optionsModel;
    QTabWidget *innerTab = new QTabWidget;

    materialsView = new MaterialsDefView(this);
    innerTab->addTab(materialsView, "Materials");

    regionsView = new RegionsView(this);
    innerTab->addTab(regionsView, "Regions");

    helpPanel->addStaticHelp(innerTab, { "/Target/materials", "/Target/regions" });

    simBoxView = new SimBoxView;
    innerTab->addTab(simBoxView, "Geometry Viewer");

    QWidget *widget = new QWidget;
    QHBoxLayout *hbox1 = new QHBoxLayout;
    hbox1->addLayout(createForm(idx, widget), 3);
    hbox1->addStretch(1);
    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addLayout(hbox1);
    vbox->addSpacing(12);
    vbox->addWidget(innerTab);
    widget->setLayout(vbox);

    return widget;
}

QWidget *OptionsView::createTab(const QModelIndex &idx)
{
    // exclude experimental stuff
    QStringList excludeKeys;
    excludeKeys << "time_ordered_cascades"
                << "correlated_recombination"
                << "move_recoil"
                << "recoil_sub_ed";
    QWidget *widget = new QWidget;
    QHBoxLayout *hbox = new QHBoxLayout;
    widget->setLayout(hbox);
    hbox->addLayout(createForm(idx, widget, excludeKeys));
    hbox->addStretch();
    return widget;
}

QFormLayout *OptionsView::createForm(const QModelIndex &parent, QWidget *widgetParent,
                                               const QStringList &excludeKeys)
{
    QFormLayout *flayout = new QFormLayout;
    OptionsModel *model = mainui->optionsModel;
    for (int row = 0; row < model->rowCount(parent); ++row) {
        QModelIndex i = model->index(row, 0, parent);
        OptionsItem *item = model->getItem(i);

        if (excludeKeys.contains(item->key()))
            continue;

        QWidget *w = item->createEditor(widgetParent);
        if (w) {            
            flayout->addRow(item->name(), w);
            mapper->addMapping(w, i, true, item->editorSignal());
            mapper->addMapping(flayout->labelForField(w), i, false);
        }
    }
    return flayout;
}

void OptionsView::toggleHelpPanel(bool checked)
{
    QList<int> sizes = splitter->sizes();
    int total = sizes[0] + sizes[1];
    if (checked) {
        if (sizes[1] == 0) {
            sizes[1] = qMax(200, total / 3);
            sizes[0] = total - sizes[1];
        }
    } else {
        sizes[0] = total;
        sizes[1] = 0;
    }
    splitter->setSizes(sizes);
    if (checked != helpToggle->isChecked()) {
        QSignalBlocker blocker(helpToggle);
        helpToggle->setChecked(checked);
    }
    if (checked != helpButton->isChecked()) {
        QSignalBlocker blocker(helpButton);
        helpButton->setChecked(checked);
    }
}

void OptionsView::onSplitterMoved(int a, int b)
{
    bool open = splitter->sizes()[1] > 0;
    if (open != helpToggle->isChecked()) {
        QSignalBlocker blocker(helpToggle);
        helpToggle->setChecked(open);
    }
    if (open != helpButton->isChecked()) {
        QSignalBlocker blocker(helpButton);
        helpButton->setChecked(open);
    }
}

void OptionsView::selectIonZ()
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

void OptionsView::validateOptions()
{
    QString msg;
    bool ret = mainui->driverObj()->validateOptions(&msg);
    if (!ret)
        QMessageBox::warning(mainui, "Options validation", msg);
    else
        QMessageBox::information(mainui, "Options validation", "Options are OK!");
}

void OptionsView::onDriverStatusChanged()
{
    McDriverObj *D = mainui->driverObj();
    auto s = D->status();
    // activate/deactivate widgets depending on sim status
    bool isreset = s == McDriverObj::mcReset;
    mapper->setEnabled(isreset);
    btSelectIon->setEnabled(isreset);
    materialsView->setEnabled(isreset);
    regionsView->setEnabled(isreset);
    if (isreset)
        applyRules();
    btValidate->setEnabled(isreset);
    buttonBox->setEnabled(isreset);

    if (isreset) {
        setToolTip("");
        mapper->setToolTip();
    } else {
        QString msg("Config cannot be changed while the simulation is active.\n"
                    "Reset the simulation to change the configuration.");
        setToolTip(msg);
        mapper->setToolTip(msg);
    }
}
