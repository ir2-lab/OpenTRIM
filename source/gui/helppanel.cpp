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
#include <QSplitter>

QString HelpPanel::sectionHeader(const QString &title) const{
    return QString("<tr><td style='padding: 8px 0 2px 0;'>"
        "  <span style='font-size: 8pt; font-weight: bold; "
        "  text-transform: uppercase; color: #888; "
        "  letter-spacing: 1px;'>%1</span>"
        "</td></tr>"
    ).arg(title);
}


QString HelpPanel::detectCurrentValue(OptionsItem *item) const{
    if(!item) return QString();

    QVariant val = item->value();
    return val.isValid() ? val.toString() : QString();
}


QString HelpPanel::BooleanHelp(OptionsItem *item)const{
    QString html;
    QString toolTip = item->toolTip();
    QString currentVal = detectCurrentValue(item);
    bool isTrue = (currentVal.toLower() == "true" || currentVal == "1");

    html += sectionHeader("Effect");

    QString trueEffect, falseEffect;
    if(!toolTip.isEmpty()){
        if(toolTip.startsWith("Enable", Qt::CaseInsensitive)){
            trueEffect = toolTip;
            falseEffect = toolTip;
            falseEffect.replace(QRegularExpression("^Enable[s]?", 
                QRegularExpression::CaseInsensitiveOption), "Disable");
        }else if(toolTip.startsWith("Store", Qt::CaseInsensitive)){
            trueEffect = toolTip;
            falseEffect = QString("Do not %1").arg(toolTip.left(1).toLower() + toolTip.mid(1));
        }else{
            trueEffect = toolTip;
            falseEffect = QString("Disabled");
        }
    }else{
        trueEffect = "Enabled";
        falseEffect = "Disabled";
    }

    QString trueMarker = "";
    QString falseMarker = "";
    QString trueStyle = "color: #2e7d32; font-weight: bold;";
    QString falseStyle = "color: #c62828; font-weight: bold;";

    html += QString("<tr><td style='padding: 2px 0;'>"
        "  <span style='%1'>• True: %2%3</span><br/>"
        "  <span style='%4'>• False: %5%6</span>"
        "</td></tr>"
    ).arg(trueStyle, trueEffect, trueMarker,
          falseStyle, falseEffect, falseMarker);

    html += sectionHeader("Current Value");
    QString valStr = isTrue ? "true" : "false";
    QString bg = isTrue ? "#e8f5e9" : "#fce4ec";
    QString color = isTrue ? "#2e7d32" : "#c62828";
    html += QString("<tr><td style='padding: 2px 0;'>"
        "  <code style='background: %1; color: %2; padding: 2px 8px; "
        "  border-radius: 3px; font-size: 10pt; font-weight: bold;'>%3</code>"
        "</td></tr>").arg(bg, color, valStr);

    return html;
}

QString HelpPanel::EnumHelp(OptionsItem *item)const{
    QString html;
    html += sectionHeader("Options");

    EnumOptionsItem *enumItem = dynamic_cast<EnumOptionsItem *>(item);
    if(!enumItem) return html;

    const QStringList &values = enumItem->values();
    const QStringList &labels = enumItem->valueLabels();
    QString currentVal = detectCurrentValue(item);

    QString whats = item->whatsThis();

    QMap<QString, QString> valueDescriptions;
    QStringList tipLines;

    if(!whats.isEmpty()){
        QStringList lines = whats.split('\n');
        for(const QString &line : lines){
            QString trimmed = line.trimmed();
            if(trimmed.isEmpty()) continue;

            if(trimmed.startsWith("- ") || trimmed.startsWith("• ")){
                QString entry = trimmed.mid(2).trimmed();
                int colonPos = entry.indexOf(':');
                if(colonPos > 0){
                    QString key = entry.left(colonPos).trimmed();
                    QString desc = entry.mid(colonPos + 1).trimmed();
                    for(int i = 0; i < values.size(); ++i){
                        if(key == values[i] || (i < labels.size() && key == labels[i])){
                            valueDescriptions[values[i]] = desc;
                            break;
                        }
                    }
                }
            }else{
                QString tip = item->toolTip();
                if(trimmed != tip && !trimmed.isEmpty()) tipLines.append(trimmed);
            }
        }
    }

    QString optionsBox;
    optionsBox += "<div style='background: #f4f4f4; border: 1px solid #ddd; "
                  "border-radius: 4px; padding: 6px 10px; margin: 2px 0;'>";

    for(int i = 0; i < values.size(); ++i){
        bool isCurrent = (values[i] == currentVal);
        QString label = (i < labels.size() && !labels[i].isEmpty()) ? labels[i] : values[i];
        QString desc = valueDescriptions.value(values[i], "");

        QString nameStyle;
        if(isCurrent)
            nameStyle = "color: #1565c0; font-weight: bold;";
        else
            nameStyle = "font-weight: bold; color: #333;";

        optionsBox += "<div style='padding: 3px 0;'>";

        if(!desc.isEmpty()){
            optionsBox += QString("<span style='%1'>%2</span>"
                " &mdash; <span style='color: #666;'>%3</span>")
                .arg(nameStyle, label, desc);
        } else {
            optionsBox += QString("<span style='%1'>%2</span>").arg(nameStyle, label);
        }

        optionsBox += "</div>";
    }
    optionsBox += "</div>";

    html += QString("<tr><td style='padding: 2px 0 10px 0;'>%1</td></tr>").arg(optionsBox);

    if(!tipLines.isEmpty()){
        html += sectionHeader("Note");
        QString noteText = tipLines.join("<br/>");
        html += QString("<tr><td style='padding: 2px 0 10px 0;'>"
            "<div style='background: #c8d1f6; border: 1px solid #aed5f3; "
            "border-radius: 4px; padding: 6px 10px; margin: 2px 0;'>"
            "<span style='font-size: 9pt; color: #555; font-style: italic;'>%1</span>"
            "</div>"
            "</td></tr>").arg(noteText);
    }

    if(!currentVal.isEmpty()){
        html += sectionHeader("Current Value");
        html += QString("<tr><td style='padding: 2px 0;'>"
            "<span style='font-size: 9pt; font-weight: bold; color: #1565c0;'>\"%1\"</span>"
            "</td></tr>").arg(currentVal);
    }

    return html;
}

QString HelpPanel::NumericHelp(OptionsItem *item) const{
    QString html;
    html += sectionHeader("Range");

    double minVal = 0, maxVal = 0;
    if(auto *fi = dynamic_cast<FloatOptionsItem *>(item)){
        minVal = fi->min();
        maxVal = fi->max();
    }else if(auto *ii = dynamic_cast<IntOptionsItem *>(item)){
        minVal = ii->min();
        maxVal = ii->max();
    }

    QString currentVal = detectCurrentValue(item);

    html += QString("<tr><td style='padding: 2px 0;'>"
        "  <span style='font-size: 10pt;'>%1 — %2</span>"
        "</td></tr>").arg(minVal).arg(maxVal);

    if(!currentVal.isEmpty()) {
        html += sectionHeader("Current Value");
        html += QString("<tr><td style='padding: 2px 0;'>"
            "  <span style='font-size: 12pt; font-weight: bold; color: #1565c0;'>%1</span>"
            "</td></tr>").arg(currentVal);
    }
    return html;
}

QString HelpPanel::VectorHelp(OptionsItem *item)const{
    QString html;
    QString currentVal = detectCurrentValue(item);

    double minVal = 0, maxVal = 0;
    if(auto *vi = dynamic_cast<VectorOptionsItem *>(item)){
        minVal = vi->min();
        maxVal = vi->max();
    }else if(auto *ivi = dynamic_cast<IVectorOptionsItem *>(item)){
        minVal = ivi->min();
        maxVal = ivi->max();
    }

    html += sectionHeader("Element Range");
    html += QString("<tr><td style='padding: 2px 0;'>"
        "  <span style='font-size: 9pt;'>%1 — %2</span>"
        "</td></tr>").arg(minVal).arg(maxVal);

    if(!currentVal.isEmpty()) {
        html += sectionHeader("Current Value");
        html += QString("<tr><td style='padding: 2px 0;'>"
            "  <span style='font-size: 9pt; font-weight: bold; color: #1565c0;'>%1</span>"
            "</td></tr>").arg(currentVal);
    }
    return html;
}

HelpPanel::HelpPanel(OptionsModel *model, QWidget *parent): QWidget(parent), model_(model){

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(4, 4, 4, 4);

    auto *header = new QHBoxLayout;
    auto *header_label = new QLabel(tr("    Configuration Help"));

    header_label->setStyleSheet("font-weight: bold; font-size: 12pt;");
    header->addWidget(header_label);
    header->addStretch();

    layout->addLayout(header);

    browser_ = new QTextBrowser;
    browser_->setOpenExternalLinks(true);
    browser_->setFrameStyle(QFrame::NoFrame);
    layout->addWidget(browser_);

    setMinimumWidth(220);
    setMaximumWidth(350);
}

void HelpPanel::registerWidget(QWidget *editor, const QModelIndex &index){
    if(editor && index.isValid()){
        widgetIndexMap_[editor] = QPersistentModelIndex(index);
        editor->installEventFilter(this);
    }
}


bool HelpPanel::eventFilter(QObject *watched, QEvent *event){
    if(event->type() == QEvent::FocusIn ||
        event->type() == QEvent::Enter){
        QWidget *w = qobject_cast<QWidget *>(watched);
        if(w && widgetIndexMap_.contains(w)){
            showHelpFor(widgetIndexMap_.value(w));
            return false;
        }
    }
    return QWidget::eventFilter(watched, event);
}


void HelpPanel::showHelpFor(const QModelIndex &index){
    if(!index.isValid()) return;

    OptionsItem *item = static_cast<OptionsItem *>(index.internalPointer());
    if(!item) return;

    browser_->setHtml(generateHelpHtml(item));
}

void HelpPanel::togglePanel(){
    QSplitter *splitter = qobject_cast<QSplitter *>(parentWidget());
    if(splitter){
        QList<int> sizes = splitter->sizes();
        if(sizes[1] == 0){
            sizes[1] = 200;
            splitter->setSizes(sizes);
        } else {
            sizes[1] = 0;
            splitter->setSizes(sizes);
        }
    }else{
        setVisible(!isVisible());
    }
}

//Main HTML generation

QString HelpPanel::generateHelpHtml(OptionsItem *item) const{
    if(!item) return QString();

    QString type;
    if(dynamic_cast<BoolOptionsItem *>(item))
        type = "bool";
    else if(dynamic_cast<EnumOptionsItem *>(item))
        type = "selection";
    else if(dynamic_cast<VectorOptionsItem *>(item))
        type = "vector";
    else if(dynamic_cast<IVectorOptionsItem *>(item))
        type = "ivector";
    else if(dynamic_cast<FloatOptionsItem *>(item))
        type = "float";
    else if(dynamic_cast<IntOptionsItem *>(item))
        type = "int";
    else if(dynamic_cast<StringOptionsItem *>(item))
        type = "string";
    else if(item->childCount() > 0)
        type = "group";
    else
        type = "unknown";

    QString html = "<html><head><style>"
        "body { font-family: sans-serif; margin: 6px; }"
        "table { width: 100%; border-collapse: collapse; }"
        "</style></head><body><table>";

    QString label = item->name().isEmpty() ? item->key() : item->name();
    html += QString("<tr><td style='padding-bottom: 6px;'>"
        "  <span style='font-size: 13pt; font-weight: bold; color: #0d47a1;'>%1</span>"
        "</td></tr>").arg(label);


    html += sectionHeader("Internal Name");
    html += QString("<tr><td style='padding: 2px 0 6px 0;'>"
        "  <code style='font-size: 10pt; background: #f5f5f5; padding: 2px 4px;'>%1</code>"
        "</td></tr>").arg(item->key());

    QString toolTip = item->toolTip();
    if(!toolTip.isEmpty()){
        html += sectionHeader("Description");
        html += QString("<tr><td style='padding: 2px 0 10px 0;'>"
            "  <span style='font-size: 10pt;'>%1</span>"
            "</td></tr>").arg(toolTip);
    }

    QString whatsThis = item->whatsThis();

    html += sectionHeader("Type");
    QString typeDisplay = type.toUpper();
    html += QString("<tr><td style='padding: 2px 0 6px 0;'>"
        "  <span style='font-size: 10pt; font-weight: bold; color: #1565c0;'>%1</span>"
        "</td></tr>").arg(typeDisplay);

    if(type == "bool"){
        html += BooleanHelp(item);
    }else if(type == "selection") {
        html += EnumHelp(item);
    }else if(type == "int" || type == "float") {
        html += NumericHelp(item);
    }else if(type == "vector" || type == "ivector") {
        html += VectorHelp(item);
    }else if(type == "string") {
        QString currentVal = detectCurrentValue(item);
        if(!currentVal.isEmpty()){
            html += sectionHeader("Current Value");
            html += QString("<tr><td style='padding: 2px 0;'>"
                "  <span style='font-size: 11pt; font-weight: bold; color: #1565c0;'>%1</span>"
                "</td></tr>").arg(currentVal);
        }
    }
    if(!whatsThis.isEmpty() && type != "selection"){
    
        QString cleanWhats = whatsThis;
    
        if(cleanWhats.startsWith(toolTip + "\n\n"))
            cleanWhats = cleanWhats.mid(toolTip.length() + 2);
        else if(cleanWhats.startsWith(toolTip + "\n"))
            cleanWhats = cleanWhats.mid(toolTip.length() + 1);
        else if(cleanWhats == toolTip)
            cleanWhats.clear();
    
        if(!cleanWhats.isEmpty()){
            html += sectionHeader("Note");
            QString formatted = cleanWhats;
            formatted.replace("\n", "<br/>");
            html += QString("<tr><td style='padding: 2px 0 10px 0;'>"
                "  <div style='background: #c8d1f6; border: 1px solid #aed5f3; "
                "  border-radius: 4px; padding: 6px 10px; margin: 2px 0;'>"
                "  <span style='font-size: 9pt; color: #555; font-style: italic;'>%1</span>"
                "  </div>"
                "</td></tr>").arg(formatted);
        }
    }
    html += "</table></body></html>";
    return html;
}
