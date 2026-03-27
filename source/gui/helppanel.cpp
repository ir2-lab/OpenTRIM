#include "helppanel.h"
#include "optionsmodel.h"

#include <QTextBrowser>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QEvent>
#include <QRegularExpression>
#include <QCheckBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QLineEdit>

/*Help Database*/

void HelpPanel::buildHelpDatabase(){
    //General simulation options

    helpDb_["simulation_type"] = {
        "Defines which particles are explicitly transported.",
        "Use <b>FullCascade</b> for radiation damage studies. "
        "Use <b>IonsOnly</b> for fast ion range/energy-loss profiles.",
        ""
    };

    helpDb_["screening_type"] = {
        "The screened Coulomb potential used for calculating nuclear scattering angles. "
        "This determines the ion-atom interaction physics.",
        "<b>ZBL</b> is the common default and is compatible with standard SRIM-style screened nuclear scattering assumptions. ",
        ""
    };

    helpDb_["electronic_stopping"] = {
        "Model for continuous energy loss of moving particles to target electrons. "
        "This is the dominant energy loss mechanism at high velocities.",
        "Use <b>SRIM13</b> for best agreement with experimental data. ",
        ""
    };

    helpDb_["electronic_straggling"] = {
        "Statistical fluctuations (straggling) in electronic energy loss. "
        "Adds realistic broadening to ion range distributions.",
        "Enable for realistic range distributions. "
        "Disable for faster runs when only mean ranges are needed.",
        ""
    };

    helpDb_["nrt_calculation"] = {
        "Method for estimating the number of displacements produced in a collision cascade. ",
        "The NRT model is the standard for radiation damage estimation. "
        "Different sub-models handle multi-element targets differently.",
        ""
    };

    helpDb_["intra_cascade_recombination"] = {
        "Controls whether Frenkel pair recombination within a single collision cascade is modeled. "
        "In reality, many displaced atoms recombine with nearby vacancies during cascade evolution",
        "Enable for more realistic defect counts. "
        "Enabling this option can reduce the number of surviving defects relative to pure NRT-style estimates",
        ""
    };

    //Ion transport options

    helpDb_["min_energy"] = {
        "Minimum kinetic energy (in eV) below which a particle is no longer tracked. "
        "Lowering this value tracks particles to lower energies — more accurate but slower.",
        "For sputtering: use 1 eV or lower. "
        "For deep implant profiles: 5–10 eV is usually sufficient. ",
        "eV"
    };

    helpDb_["flight_path_type"] = {
        "How the free flight path (distance between collisions) is determined. "
        "This determines how collision distances are sampled. ",
        "<b>Constant</b> uses a fixed mean free path. "
        "<b>Variable</b> uses energy-dependent mean-free-path tables and may skip low impact scattering events ,below energy or angle thresholds.",
        ""
    };

    helpDb_["flight_path_const"] = {
        "The constant free flight path expressed in units of the atomic radius. "
        "A value of 1.0 corresponds to roughly one collision per atomic spacing.",
        "Values around 1.0 are standard. Smaller values increase accuracy but slow down the simulation. "
        "Values much larger than 1 may miss important close collisions.",
        "× atomic radius"
    };

    helpDb_["max_rel_loss"] = {
        "Maximum fractional energy loss (ΔE/E) allowed per collision step. "
        "If a step would lose more than this fraction, the step is shortened.",
        "Smaller values give more accurate energy loss tracking at the cost of speed. "
        "The default 0.05 (5%) is a good balance.",
        ""
    };

    helpDb_["min_recoil_energy"] = {
        "Minimum energy (in eV) for a recoil atom to be followed as a new cascade particle. "
        "Recoils below this energy are not tracked and contribute only to local energy deposition.",
        "Lower values track more recoils (more accurate damage profiles, slower). "
        "Should be ≤ the displacement energy for accurate defect counting.",
        "eV"
    };

    helpDb_["min_scattering_angle"] = {
        "Minimum lab-frame scattering angle considered in variable-flight-path sampling. "
        "Very small-angle events below this threshold may be ignored for efficiency.",
        "The default 2° skips very small angle deflections. "
        "Reduce to 0.5° only if you need extreme accuracy in angular distributions.",
        "degrees"
    };


    //Ion Source

    helpDb_["atomic_number"] = {
        "Atomic number of the incident ion. Determines the nuclear charge, affecting "
        " scattering cross-sections and electronic stopping power.",
        "Common ions: H(1), He(2), C(6), N(7), O(8), Ne(10), Si(14), Ar(18), Kr(36), Xe(54), Au(79).",
        ""
    };

    helpDb_["atomic_mass"] = {
        "Atomic mass of the incident ion (amu). "
        "If set to 0, the natural isotopic average mass is used automatically.",
        "Set explicitly for specific isotopes (D=2.014, ³He=3.016). "
        "Leave at 0 for the natural average.",
        "amu"
    };

     helpDb_["center"] = {
        "Mean value of this distribution. "
        "For energy: the mean ion beam energy in eV. "
        "For spatial: the beam center position [x, y, z] in nm. "
        "For angular: the beam center direction vector.",
        "Combine with a non-SingleValue distribution type and FWHM to model beam spread.",
        ""
    };


    helpDb_["type"] = {
        "Type of probability distribution used to sample this parameter. "
        "<b>SingleValue</b> uses a fixed value (monoenergetic or single direction).",
        "Use <b>SingleValue</b> for idealized beam conditions. "
        "Other types add spread around the central value, controlled by the FWHM.",
        ""
    };


    helpDb_["fwhm"] = {
        "Full Width at Half Maximum of the distribution. "
        "Controls the spread around the mean value. "
        "Only active when the distribution type is not SingleValue.",
        "Typical beam spot sizes depend on teh system and focusing optics.",
        ""
    };

    //Target

    helpDb_["cell_count"] = {
        "Number of simulation cells along each axis [nx, ny, nz]. "
        "The z-axis is the depth direction. Defines the spatial resolution of tallied profiles.",
        "More cells = finer spatial resolution but more memory. "
        "200–500 depth cells is typical. For 1D simulations, spacial resolution is usually controlled through nz.",
        "cells"
    };

    helpDb_["origin"] = {
        "Origin of the simulation grid in nm [x₀, y₀, z₀]. "
        "Usually [0, 0, 0], the target surface.",
        "Adjust if your target does not start at the origin.",
        "nm"
    };

    helpDb_["size"] = {
        "Size of the simulation box along each axis [Lx, Ly, Lz] in nm. "
        "Lz is the target depth. Must be large enough that ions stop inside.",
        "Use roughly 3× the projected ion range for Lz. "
        "For 1D simulations, Lx and Ly are irrelevant.",
        "nm"
    };

    helpDb_["periodic_bc"] = {
        "Periodic boundary conditions along each axis [x, y, z]. "
        "A value of 1 enables periodic BC, 0 disables it. "
        "Periodic BCs wrap particles that exit one side back through the opposite side.",
        "",
        ""
    };

    //Output options

    helpDb_["store_damage_events"] = {
        "Whether to store individual damage events (displacements, replacements, vacancies). "
        "Provides detailed information about each defect-producing event.",
        "Enable for detailed damage analysis. "
        "Can produce large output for high-energy cascades.",
        ""
    };

    helpDb_["store_dedx"] = {
        "Whether to store the depth-resolved energy deposition profile "
        "(nuclear and electronic components).",
        "Essential for understanding where energy is dissipated in the target. "
        "The nuclear component corresponds to lattice damage, the electronic "
        "component to ionization and excitation.",
        ""
    };

    helpDb_["store_exit_events"] = {
        "Whether to record ions and atoms that exit the target (both transmitted and sputtered). "
        "Required for computing sputtering yields and transmission data.",
        "Enable for sputtering studies or thin-film transmission experiments. "
        "Disable to save memory for bulk targets where only depth profiles are needed.",
        ""
    };

    helpDb_["store_pka_events"] = {
        "Whether to store information about Primary Knock-on Atom (PKA) events. "
        "PKAs are target atoms initially displaced by the incoming ion, "
        "initiating a recoil cascade.",
        "Useful for analyzing PKA energy spectra and damage initiation.",
        ""
    };


}

/*Style helpers*/

QString HelpPanel::sectionHeader(const QString &title) const{
    return QString(
        "<tr>"
        "  <td style='padding: 6px 0 2px 0; color: #888; font-size: 9pt; "
        "     text-transform: uppercase; letter-spacing: 1px;'>"
        "    %1"
        "  </td>"
        "</tr>"
    ).arg(title);
}

QString HelpPanel::codeBlock(const QString &text) const{
    return QString(
        "<code style='background: #f4f4f4; padding: 2px 8px; "
        "border-radius: 3px; font-size: 10pt;'>%1</code>"
    ).arg(text);
}

QString HelpPanel::infoBox(const QString &text, const QString &color) const{
    return QString(
        "<div style='background: %2; border-radius: 4px; "
        "padding: 8px 10px; margin: 4px 0;'>%1</div>"
    ).arg(text, color);
}

QString HelpPanel::typeBadge(const QString &typeName) const{
    return QString(
        "<span style='background: #e0e7ff; color: #3b5998; font-size: 8pt; "
        "padding: 2px 8px; border-radius: 8px; font-weight: bold; "
        "text-transform: uppercase; letter-spacing: 0.5px;'>%1</span>"
    ).arg(typeName);
}

QString HelpPanel::effectRow(const QString &value, const QString &desc, const QString &color) const{
    return QString(
        "<div style='background: %3; border-radius: 4px; "
        "padding: 6px 10px; margin-bottom: 3px;'>"
        "  <code style='font-weight: bold;'>%1</code>"
        "  <span style='color: #555;'> — %2</span>"
        "</div>"
    ).arg(value, desc, color);
}

/*Text parsing*/

QString HelpPanel::extractMeaning(const QString &text) const{
    if(text.isEmpty()) return QString();

    int listStart = -1;

    QRegularExpression listPattern(R"([\n\r]\s*-\s+\w)");
    auto match = listPattern.match(text);
    if(match.hasMatch()) listStart = match.capturedStart();

    QString meaning;
    if(listStart >0){
        meaning = text.left(listStart).trimmed();
    }else{
        QRegularExpression sentenceEnd(R"(\.\s)");
        auto m = sentenceEnd.match(text);
        if(m.hasMatch()){
            meaning = text.left(m.capturedStart() +1).trimmed();
        }else{
            meaning = text.trimmed();
        }
    }

    return meaning;
}

QStringList HelpPanel::extractOptions(const QString &text) const{
    QStringList options;
    if(text.isEmpty())return options;

    QRegularExpression optPattern(R"(-\s+(\w[\w\s]*?)[\s]*[:\-\x{2013}\x{2014}]\s*(.+))");
    auto it = optPattern.globalMatch(text);
    while(it.hasNext()){
        auto match =it.next();
        QString name = match.captured(1).trimmed();
        QString desc = match.captured(2).trimmed();
        options << name + "|" +desc;
    }

    return options;
}

QString HelpPanel::parseOptionsListHtml(const QString &text) const{
    QStringList options = extractOptions(text);
    if(options.isEmpty()) return QString();

    QString html;
    html += "<table cellspacing='0' cellpadding='0' style='margin: 4px 0;'>";

    for(const QString &opt : options){
        QStringList parts = opt.split('|',Qt::KeepEmptyParts);
        QString name = parts.value(0);
        QString desc = parts.value(1);

        html += QString(
            "<tr>"
            "  <td style='padding: 4px 0;'>"
            "    <div style='background: #f0f4f8; border-radius: 4px; "
            "         padding: 6px 10px; margin-bottom: 2px;'>"
            "      <span style='font-weight: bold; color: #2060a0;'>%1</span>"
            "      <span style='color: #555;'> — %2</span>"
            "    </div>"
            "  </td>"
            "</tr>"
        ).arg(name, desc);
    }

    html += "</table>";
    return html;
}

/*Type-specific info*/

QString HelpPanel::booleanEffectHtml(OptionsItem *item) const{
    QString name = item->name().toLower();
    QString key = item->key().toLower();
    QString whats = item->whatsThis();
    QString desc = whats.isEmpty() ? item->toolTip() : whats;

    //check helpDb for richer description
    if(helpDb_.contains(item->key())) desc =helpDb_[item->key()].description;

    QString trueEffect, falseEffect;

    if(name.contains("enable") || key.contains("enable")){
        QString feature = name;
        feature.remove(QRegularExpression("^enable[sd]?\\s*", QRegularExpression::CaseInsensitiveOption));

        if (feature.isEmpty()) feature = "this feature";
        
        trueEffect =QString("Enables %1").arg(feature);
        falseEffect = QString("Disables %1").arg(feature);
    }else if(!desc.isEmpty()){
        QString cleanDesc = desc.trimmed();
        
        cleanDesc.remove(QRegularExpression("<[^>]*>")); // Strip HTML tags for the effect row
        if (cleanDesc.endsWith('.')) cleanDesc.chop(1);

        QRegularExpression firstSentence(R"(^(.+?\.)\s)");  // Take just the first sentence for the effect
        auto m = firstSentence.match(cleanDesc);

        if(m.hasMatch()){
            cleanDesc = m.captured(1);
            if(cleanDesc.endsWith('.'))cleanDesc.chop(1);
        }

        if(cleanDesc.startsWith("Enable", Qt::CaseInsensitive) ||
            cleanDesc.startsWith("Control", Qt::CaseInsensitive)){
            trueEffect = cleanDesc;
            falseEffect = cleanDesc;
            if(cleanDesc.startsWith("Enable", Qt::CaseInsensitive))
                falseEffect.replace(QRegularExpression("^Enable[s]?", QRegularExpression::CaseInsensitiveOption), "Disable");
            else if(cleanDesc.startsWith("Control", Qt::CaseInsensitive))
                falseEffect.replace(QRegularExpression("^Controls", QRegularExpression::CaseInsensitiveOption), "Disables");
        }else if(cleanDesc.startsWith("Whether",Qt::CaseInsensitive)){
            QString action = cleanDesc;

            action.remove(QRegularExpression("^Whether\\s+(to\\s+)?", QRegularExpression::CaseInsensitiveOption));

            QString capitalized = action.left(1).toUpper() +action.mid(1);
            trueEffect = capitalized;
            falseEffect = QString("Does NOT %1").arg(action);
        }else{
            trueEffect = cleanDesc;
            falseEffect = QString("Disables %1").arg(cleanDesc.left(1).toLower() + cleanDesc.mid(1));
        }
    }else{
        QString label = item->name().isEmpty() ? item->key() : item->name();
        trueEffect = QString("Enables %1").arg(label.toLower());
        falseEffect = QString("Disables %1").arg(label.toLower());
    }

    QString html;
    html += effectRow("true", trueEffect, "#e8f5e9");
    html += effectRow("false", falseEffect, "#fce4ec");
    return html;
}

QString HelpPanel::typeInfoHtml(OptionsItem *item) const{
    QVariant val = item->value();
    QString html;

    for(auto it = widgetIndexMap_.constBegin(); it !=widgetIndexMap_.constEnd(); ++it){
        if(!it.value().isValid()) continue;
        OptionsItem *mappedItem = static_cast<OptionsItem *>(it.value().internalPointer());
        if(mappedItem != item)continue;

        QWidget *widget = it.key();

        /*boolean box*/
        QComboBox *combo = qobject_cast<QComboBox *>(widget);
        if (combo && combo->count() == 2) {
            QString item0 = combo->itemText(0).toLower();
            QString item1 = combo->itemText(1).toLower();
            if ((item0 =="false" && item1 == "true") ||
                (item0== "true" && item1 == "false")) {
                html += sectionHeader("Type");
                html += QString("<tr><td style='padding: 2px 0 10px 0;'>%1</td></tr>")
                            .arg(typeBadge("Boolean"));
                html += sectionHeader("Effect");
                html += QString("<tr><td style='padding: 2px 0 10px 0;'>%1</td></tr>")
                            .arg(booleanEffectHtml(item));
                return html;
            }
        }

        /*enum combo box*/
        if(combo && combo->count() > 0){
            html += sectionHeader("Type");
            html += QString("<tr><td style='padding: 2px 0 10px 0;'>%1</td></tr>")
                        .arg(typeBadge("Selection"));

            QString whats = item->whatsThis();
            QStringList parsed = extractOptions(whats.isEmpty() ? item->toolTip() : whats);
            if(parsed.isEmpty()){
                html += sectionHeader("Options");
                QString optsHtml;
                for(int i = 0; i < combo->count(); ++i){
                    QString tipText = combo->itemData(i, Qt::ToolTipRole).toString();
                    if (tipText.isEmpty()) tipText = combo->itemText(i);
                    optsHtml += effectRow(combo->itemText(i), tipText, (i % 2 == 0) ? "#f0f4f8" : "#e8ecf0");
                }
                html += QString("<tr><td style='padding: 2px 0 10px 0;'>%1</td></tr>").arg(optsHtml);
            }
            return html;
        }

        /*checkbox*/
        QCheckBox *check = qobject_cast<QCheckBox *>(widget);
        if(check){
            html += sectionHeader("Type");
            html += QString("<tr><td style='padding: 2px 0 10px 0;'>%1</td></tr>")
                        .arg(typeBadge("Boolean"));
            html += sectionHeader("Effect");
            html += QString("<tr><td style='padding: 2px 0 10px 0;'>%1</td></tr>")
                        .arg(booleanEffectHtml(item));
            return html;
        }

        /*spinbox*/
        QSpinBox *spin = qobject_cast<QSpinBox *>(widget);
        if (spin) {
            html += sectionHeader("Type");
            html += QString("<tr><td style='padding: 2px 0 10px 0;'>%1</td></tr>")
                        .arg(typeBadge("Integer"));
            QString rangeStr = QString("%1 – %2")
                .arg(spin->minimum()).arg(spin->maximum());
            html += sectionHeader("Range");
            html += QString("<tr><td style='padding: 2px 0 10px 0;'>%1</td></tr>")
                        .arg(codeBlock(rangeStr));
            return html;
        }

        QDoubleSpinBox *dspin = qobject_cast<QDoubleSpinBox *>(widget);
        if(dspin){
            html += sectionHeader("Type");
            html += QString("<tr><td style='padding: 2px 0 10px 0;'>%1</td></tr>")
                        .arg(typeBadge("Float"));
            QString rangeStr = QString("%1 – %2")
                .arg(dspin->minimum(), 0, 'g', 6)
                .arg(dspin->maximum(), 0, 'g', 6);
            html += sectionHeader("Range");
            html += QString("<tr><td style='padding: 2px 0 10px 0;'>%1</td></tr>")
                        .arg(codeBlock(rangeStr));
            if (!qFuzzyIsNull(dspin->singleStep())) {
                html += sectionHeader("Step size");
                html += QString("<tr><td style='padding: 2px 0 10px 0;'>%1</td></tr>")
                            .arg(codeBlock(QString::number(dspin->singleStep(), 'g', 6)));
            }
            return html;
        }

        //detect numeric vs text
        QLineEdit *le = qobject_cast<QLineEdit *>(widget);
        if(le){
            bool isNumeric = false;
            QString valStr = val.toString().trimmed();
            if(!valStr.isEmpty()){
                valStr.toDouble(&isNumeric);
                if(!isNumeric && valStr.startsWith('[') && valStr.endsWith(']')){
                    html += sectionHeader("Type");
                    html += QString("<tr><td style='padding: 2px 0 10px 0;'>%1</td></tr>")
                                .arg(typeBadge("Array"));
                    html += sectionHeader("Format");
                    html += QString("<tr><td style='padding: 2px 0 10px 0;'>%1</td></tr>")
                                .arg(codeBlock("[x, y, z]"));
                    return html;
                }
            }
            html += sectionHeader("Type");
            html += QString("<tr><td style='padding: 2px 0 10px 0;'>%1</td></tr>")
                        .arg(typeBadge(isNumeric ? "Float" : "Text"));
            return html;
        }

        break;
    }

    //fallback
    switch (static_cast<QMetaType::Type>(val.type())){
    case QMetaType::Bool:
        html += sectionHeader("Type");
        html += QString("<tr><td style='padding: 2px 0 10px 0;'>%1</td></tr>")
                    .arg(typeBadge("Boolean"));
        html += sectionHeader("Effect");
        html += QString("<tr><td style='padding: 2px 0 10px 0;'>%1</td></tr>")
                    .arg(booleanEffectHtml(item));
        break;
    case QMetaType::Int:
    case QMetaType::LongLong:
        html += sectionHeader("Type");
        html += QString("<tr><td style='padding: 2px 0 10px 0;'>%1</td></tr>")
                    .arg(typeBadge("Integer"));
        break;
    case QMetaType::Double:
    case QMetaType::Float:
        html += sectionHeader("Type");
        html += QString("<tr><td style='padding: 2px 0 10px 0;'>%1</td></tr>")
                    .arg(typeBadge("Float"));
        break;
    default:
        break;
    }

    return html;
}

/*Constructor and Widget Registration*/

HelpPanel::HelpPanel(OptionsModel *model, QWidget *parent): QWidget(parent), model_(model){
    setObjectName("HelpPanel");
    setMinimumWidth(250);
    setMaximumWidth(380);

    buildHelpDatabase();

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);

    QHBoxLayout *header = new QHBoxLayout;

    QLabel *title = new QLabel("<b>Config Help</b>");

    QPushButton *closeBtn = new QPushButton(QString::fromUtf8("\xc3\x97"));
    closeBtn->setFixedSize(24, 24);
    closeBtn->setFlat(true);
    connect(closeBtn, &QPushButton::clicked, this, &HelpPanel::toggleVisibility);
    
    header->addWidget(title);
    header->addStretch();
    header->addWidget(closeBtn);
    
    layout->addLayout(header);

    browser_ = new QTextBrowser;
    browser_->setOpenExternalLinks(false);
    browser_->setReadOnly(true);
    browser_->setStyleSheet(
        "QTextBrowser {"
        "  font-size: 10pt;"
        "  padding: 4px;"
        "  border: 1px solid #d0d0d0;"
        "  border-radius: 4px;"
        "  background: #fafafa;"
        "}"
    );
    layout->addWidget(browser_);

    clear();
}

void HelpPanel::registerWidget(QWidget *editor, const QModelIndex &index){
    if(!editor || !index.isValid()) return;

    widgetIndexMap_.insert(editor, QPersistentModelIndex(index));
    editor->setMouseTracking(true);
    editor->setAttribute(Qt::WA_Hover, true);
    editor->installEventFilter(this);
}

bool HelpPanel::eventFilter(QObject *watched, QEvent *event){
    if(event->type() == QEvent::FocusIn ||
        event->type() == QEvent::Enter ||
        event->type() == QEvent::HoverEnter) {
        QWidget *w = qobject_cast<QWidget *>(watched);
        if(w && widgetIndexMap_.contains(w)){
            showHelpFor(widgetIndexMap_.value(w));
            return false;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void HelpPanel::toggleVisibility(){
    setVisible(!isVisible());
}

void HelpPanel::showHelpFor(const QModelIndex &index){
    if(!index.isValid()){
        clear();
        return;
    }

    OptionsItem *item = static_cast<OptionsItem *>(index.internalPointer());
    if(!item){
        clear();
        return;
    }

    browser_->setHtml(generateHelpHtml(item));
}

void HelpPanel::clear(){
    browser_->setHtml("<html><body style='color: #999; padding: 20px; text-align: center;'>"
        "<p style='font-size: 14pt;'></p>"
        "<p><i>Hover over a configuration option<br/>to see its description.</i></p>"
        "</body></html>"
    );
}

// Main HTML generation

QString HelpPanel::generateHelpHtml(OptionsItem *item) const{
    QString key = item->key();

    bool hasDbEntry = helpDb_.contains(key);

    HelpEntry entry;
    if(hasDbEntry) entry = helpDb_.value(key);

    QString html;
    html += "<html><body style='padding: 6px; font-family: sans-serif; "
            "line-height: 1.4; color: #333;'>";

    /*Title*/
    QString name = item->name().isEmpty() ? key : item->name();
    html += QString("<div style='border-bottom: 2px solid #2060a0; padding-bottom: 6px; "
        "margin-bottom: 8px;'>"
        "  <span style='font-size: 14pt; font-weight: bold; color: #2060a0;'>%1</span>"
        "</div>"
    ).arg(name);

    html += "<table width='100%' cellspacing='0' cellpadding='0'>";

    /*Internal name*/
    if(!key.isEmpty()){
        html += sectionHeader("Internal name");
        html += QString("<tr><td style='padding: 2px 0 10px 0;'>%1</td></tr>").arg(codeBlock(key));
    }

    /*Description, prefer helpDb, fall back to whatsThis/toolTip*/
    QString whats = item->whatsThis();
    QString tip = item->toolTip();
    QString descText;

    if(hasDbEntry && !entry.description.isEmpty()){
        descText = entry.description;
    }else{
        descText = whats.isEmpty() ? tip : whats;
    }

    QString meaning = extractMeaning(descText);
    if(!meaning.isEmpty()){
        html += sectionHeader("Description");
        html += QString("<tr><td style='padding: 2px 0 10px 0;'>"
            "  <span style='font-size: 10pt;'>%1</span>"
            "</td></tr>").arg(hasDbEntry ? descText : meaning);
    }

    if(hasDbEntry && !entry.units.isEmpty()){
        html += sectionHeader("Units");
        html += QString("<tr><td style='padding: 2px 0 10px 0;'>%1</td></tr>").arg(typeBadge(entry.units));
    }

    QString typeHtml = typeInfoHtml(item);
    bool isSelectionType = typeHtml.contains("SELECTION", Qt::CaseInsensitive);
    html += typeHtml;

    if(isSelectionType){
        QString meaningSource = whats.isEmpty() ? tip : whats;
        QString optionsHtml = parseOptionsListHtml(meaningSource);

        if(!optionsHtml.isEmpty()){
            html += sectionHeader("Options");
            html += QString("<tr><td style='padding: 2px 0 10px 0;'>%1</td></tr>").arg(optionsHtml);
        }
    }

    if(hasDbEntry && !entry.tip.isEmpty()){
        html += sectionHeader("Tip");
        html += QString("<tr><td style='padding: 2px 0 10px 0;'>%1</td></tr>").arg(infoBox(entry.tip,"#fff8e1"));
    }

    if(!hasDbEntry && !tip.isEmpty() && tip != whats && tip != meaning){
        QString tipMeaning = extractMeaning(tip);
        if(tipMeaning != meaning && !tipMeaning.isEmpty()){
            html += sectionHeader("Note");
            html += QString("<tr><td style='padding: 2px 0 10px 0;'>%1</td></tr>"
            ).arg(infoBox(tipMeaning, "#fff8e1"));
        }
    }

    QVariant val = item->value();
    if(val.isValid() && !val.isNull()){
        html += sectionHeader("Current value");

        QString valStr = val.toString();
        QString bg = "#f4f4f4";

        if(static_cast<QMetaType::Type>(val.type()) == QMetaType::Bool){
            bg = val.toBool() ? "#e8f5e9" : "#fce4ec";
            valStr = val.toBool() ? "true" : "false";
        }else if (valStr.toLower() == "true"){
            bg = "#e8f5e9";
        }else if (valStr.toLower() == "false"){
            bg = "#fce4ec";
        }

        html += QString("<tr><td style='padding: 2px 0 10px 0;'>"
            "  <code style='background: %2; padding: 2px 8px; "
            "  border-radius: 3px; font-size: 10pt; font-weight: bold;'>%1</code>"
            "</td></tr>"
        ).arg(valStr, bg);
    }

    html += "</table>";
    html += "</body></html>";
    return html;
}
