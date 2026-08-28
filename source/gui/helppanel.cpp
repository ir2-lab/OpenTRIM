#include "helppanel.h"
#include "optionsmodel.h"
#include "optionwidgetmapper.h"
#include "utallyview.h"
#include "json_defs_p.h"

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
#include <QTimer>
#include <QAbstractItemView>
#include <QDebug>
#include <QTableView>
#include <QHeaderView>
#include <QTabBar>
#include <QMouseEvent>
#include <QEnterEvent>

struct HelpPanel::HelpData
{
    enum widget_hint_t { label, tabbar, header, table };
    QWidget *parentView{ nullptr };
    QStringList path;
    ojson::array_t j;
    widget_hint_t widget_hint{ label };
    int get_section(QWidget *w, const QPoint &p)
    {
        if (p.isNull())
            return 0;
        int col = 0;
        switch (widget_hint) {
        case tabbar: {
            QTabBar *b = qobject_cast<QTabBar *>(w);
            col = b->tabAt(p);
        } break;
        case header: {
            col = ((QHeaderView *)parentView)->logicalIndexAt(p);
            // check if this is the UserTally bins table header
            // section number is written in the "section" property
            if (parentView->objectName() == "binsTable") {
                auto V = parentView->property("section");
                if (V.isValid())
                    col = V.toInt();
            }
        } break;
        case table: {
            QModelIndex idx = ((QTableView *)parentView)->indexAt(p);
            col = idx.column();
            // check if this is the UserTally bins table
            // in this case the section is the id of the binning variable
            if (parentView->objectName() == "binsTable") {
                // section = bin variable
                auto binsModel = qobject_cast<const UserTallyBinsModel *>(idx.model());
                if (binsModel) {
                    col = binsModel->rowVar()[idx.row()];
                }
            }
        } break;
        default:
            break;
        }
        return col;
    }
    // the default options template
    static ojson json_templ;
};

ojson HelpPanel::HelpData::json_templ = ojson::parse(mcconfig::config_template().toJSON());

HelpPanel::HelpPanel(QWidget *parent) : QTextBrowser(parent)
{
    setOpenExternalLinks(true);
    setMinimumWidth(200);

    hideTimer_ = new QTimer(this);
    hideTimer_->setSingleShot(true);
    hideTimer_->setInterval(1000);
    connect(hideTimer_, &QTimer::timeout, this, [this]() {
        clearCurrent();
        clear();
    });

    setPlaceholderText("Hover over any config option to display help");
}

HelpPanel::~HelpPanel()
{
    for (auto p : static_help_map) {
        delete p;
    }
}

void HelpPanel::setWidgetMapper(OptionWidgetMapper *m)
{
    if (mapper_) {
        for (QWidget *w : mapper_->widgets())
            w->removeEventFilter(this);
        mapper_ = nullptr;
    }
    if (m) {
        mapper_ = m;
        for (QWidget *w : mapper_->widgets())
            w->installEventFilter(this);
    }
}

void HelpPanel::addStaticHelp(QWidget *w, const QString &path)
{
    QStringList lst;
    lst << path;
    addStaticHelp(w, lst);
}

void HelpPanel::addStaticHelp(QWidget *w, const QStringList &paths)
{
    HelpData *h = new HelpData;
    h->path = paths;
    ojson::array_t j;
    for (const QString &path : paths) {
        j.push_back(spec_for_path(HelpData::json_templ, ojson::json_pointer(path.toStdString())));
    }
    h->j = j;
    const QMetaObject *mobj = w->metaObject();
    if (mobj->inherits(&QTableView::staticMetaObject)) {
        h->widget_hint = HelpData::table;
        h->parentView = qobject_cast<QTableView *>(w);
        assert(h->parentView);
        w = ((QTableView *)h->parentView)->viewport();

        // add another entry for the header
        QHeaderView *hv = ((QTableView *)h->parentView)->horizontalHeader();
        HelpData *h2 = new HelpData;
        *h2 = *h;
        h2->widget_hint = HelpData::header;
        h2->parentView = hv;
        static_help_map.insert(hv->viewport(), h2);
        hv->viewport()->setMouseTracking(true);
        hv->viewport()->installEventFilter(this);
    } else if (mobj->inherits(&QTabBar::staticMetaObject)) {
        h->widget_hint = HelpData::tabbar;
    } else if (mobj->inherits(&QTabWidget::staticMetaObject)) {
        h->widget_hint = HelpData::tabbar;
        QTabWidget *tw = qobject_cast<QTabWidget *>(w);
        w = tw->tabBar();
    }
    static_help_map.insert(w, h);
    w->setMouseTracking(true);
    w->installEventFilter(this);
}

bool HelpPanel::eventFilter(QObject *watched, QEvent *event)
{
    QWidget *w = qobject_cast<QWidget *>(watched);
    if (!w)
        return QWidget::eventFilter(watched, event);

    switch (event->type()) {

    case QEvent::FocusIn:
    case QEvent::Enter: {

        // check dynamic widgets
        QModelIndex index = mapper_->widgetToIndex(w);
        if (index.isValid()) {
            OptionsItem *item = static_cast<OptionsItem *>(index.internalPointer());
            if (item) {
                hideTimer_->stop();
                clearCurrent();
                setHtml(generateHelpHtml(item));
                currentWidget = w;
            }
        } else if (static_help_map.contains(w)) {
            HelpData *h = static_help_map.value(w);
            if (h) {
                hideTimer_->stop();
                clearCurrent();
                currentSection = h->get_section(w, eventPos(event));
                setHtml(generateHelpHtml(h, currentSection));
                currentWidget = w;
                currentHelpData = h;
            }
        }
    } break;

    case QEvent::FocusOut:
    case QEvent::Leave:
    case QEvent::Hide: {

        if (w == currentWidget) {
            // check for combo drop-down
            if (auto *combo = qobject_cast<QComboBox *>(watched)) {
                // if (event->type() == QEvent::Leave) {
                qDebug() << "Combo + ev=" << event->type();
                if (combo->view() && combo->view()->isVisible()) {
                    qDebug() << "view+vis = true, no timer";
                    comboPopup = combo->view();
                    comboPopup->installEventFilter(this);
                    return false; // popup open, ignore this Leave
                } else {
                    qDebug() << "view+vis = false, clear timer";
                }
            }

            qDebug() << "timer after ev=" << event->type();
            hideTimer_->start();
        }

        if (w == comboPopup) {
            qDebug() << "comboPopup timer after ev=" << event->type();
            hideTimer_->start();
        }
    } break;

    case QEvent::MouseMove: {
        if (w == currentWidget && currentHelpData) {
            int section = currentHelpData->get_section(w, eventPos(event));
            if (section != currentSection) {
                if (section < currentHelpData->path.size() && section >= 0) {
                    setHtml(generateHelpHtml(currentHelpData, section));
                    currentSection = section;
                    qDebug() << "section change to " << currentSection
                             << " after ev=" << event->type();
                } else {
                    qDebug() << "out of scope section " << section << " after ev=" << event->type();
                    hideTimer_->start();
                }
            }
        }
    }

    default:
        break;
    }
    return QWidget::eventFilter(watched, event);
}

void HelpPanel::enterEvent(QEvent *ev)
{
    hideTimer_->stop();
}

void HelpPanel::leaveEvent(QEvent *ev)
{
    hideTimer_->start();
}

QPoint HelpPanel::eventPos(QEvent *event)
{
    if (!event)
        return QPoint(); // invalid

    switch (event->type()) {
    case QEvent::Enter: {
        auto *enterEvent = static_cast<QEnterEvent *>(event);
        return enterEvent->pos();
    }
    case QEvent::MouseMove: {
        auto *mouseEvent = static_cast<QMouseEvent *>(event);
        return mouseEvent->pos();
    }
    case QEvent::FocusIn:
        // QFocusEvent carries no position info (focus can come from
        // keyboard navigation, programmatic setFocus(), etc.)
        return QPoint(); // invalid
    default:
        return QPoint(); // invalid
    }
}

// HTML generation

QString HelpPanel::preample(const QString &label, const QString &path, const QString &desc,
                            const QString &type)
{
    QString html = "<html><head><style>"
                   "body { font-family: sans-serif; margin: 6px; }"
                   "table { width: 100%; border-collapse: collapse; }"
                   "</style></head><body><table>";

    html += QString("<tr><td style='padding-bottom: 6px;'>"
                    "  <span style='font-size: 13pt; font-weight: bold; color: #0d47a1;'>%1</span>"
                    "</td></tr>")
                    .arg(label.toHtmlEscaped());

    html += sectionHeader("JSON Path");
    html += QString("<tr><td style='padding: 2px 0 6px 0;'>"
                    "  <code style='font-size: 10pt; background: #f5f5f5; padding: 2px "
                    "4px;'>%1</code>"
                    "</td></tr>")
                    .arg(path);

    if (!desc.isEmpty()) {
        html += sectionHeader("Description");
        html += QString("<tr><td style='padding: 2px 0 10px 0;'>"
                        "  <span style='font-size: 10pt;'>%1</span>"
                        "</td></tr>")
                        .arg(desc.toHtmlEscaped());
    }

    html += sectionHeader("Type");
    html += QString("<tr><td style='padding: 2px 0 6px 0;'>"
                    "  <span style='font-size: 10pt; font-weight: bold; color: #1565c0;'>%1</span>"
                    "</td></tr>")
                    .arg(type);

    return html;
}

QString HelpPanel::sectionHeader(const QString &title)
{
    return QString("<tr><td style='padding: 8px 0 2px 0;'>"
                   "  <span style='font-size: 8pt; font-weight: bold; "
                   "  text-transform: uppercase; color: #888; "
                   "  letter-spacing: 1px;'>%1</span>"
                   "</td></tr>")
            .arg(title.toHtmlEscaped());
}

QString HelpPanel::enumHelp(const QStringList &values, const QStringList &labels,
                            const QStringList &valueDescriptions)
{
    QString html;
    html += sectionHeader("Options");

    QString optionsBox;
    optionsBox += "<div style='background: #f4f4f4; border: 1px solid #ddd; "
                  "border-radius: 4px; padding: 6px 10px; margin: 2px 0;'>";

    for (int i = 0; i < values.size(); ++i) {
        QString value = values[i].toHtmlEscaped();
        QString label = labels[i].toHtmlEscaped();
        QString desc = valueDescriptions[i].toHtmlEscaped();

        QString nameStyle = "font-weight: bold; color: #333;";

        optionsBox += "<div style='padding: 3px 0;'>";

        if (value != label) {
            optionsBox += QString("<span style='%1'>%2 [%3]</span>"
                                  " &mdash; <span style='color: #666;'>%4</span>")
                                  .arg(nameStyle, value, label, desc);
        } else {
            optionsBox += QString("<span style='%1'>%2</span>"
                                  " &mdash; <span style='color: #666;'>%3</span>")
                                  .arg(nameStyle, value, desc);
        }

        optionsBox += "</div>";
    }
    optionsBox += "</div>";

    html += QString("<tr><td style='padding: 2px 0 10px 0;'>%1</td></tr>").arg(optionsBox);

    return html;
}

QString HelpPanel::notesHelp(const QStringList &notes)
{
    QString html = sectionHeader("Notes");

    QString formatted;
    for (const QString &s : notes)
        formatted +=
                QString("<p style=\"margin: 0 0 0 12px; text-indent: -10px; padding-left: 12px;\">"
                        "<span style='font-size: 9pt; color: #555; font-style: "
                        "italic;'>&bull;&nbsp;%1</span></p>")
                        .arg(s.toHtmlEscaped());

    html += QString("<tr><td style='padding: 2px 0 10px 0;'>"
                    "  <div style='background: #c8d1f6; border: 1px solid #aed5f3; "
                    "  border-radius: 4px; padding: 10px 0; margin: 2px 0;'>"
                    "  %1"
                    "  </div>"
                    "</td></tr>")
                    .arg(formatted);

    return html;
}

QString HelpPanel::numericHelp(const QString &sectionTitle, double minVal, double maxVal)
{
    QString html;
    html += sectionHeader(sectionTitle.toHtmlEscaped());
    html += QString("<tr><td style='padding: 2px 0;'>"
                    "  <span style='font-size: 10pt;'>%1 — %2</span>"
                    "</td></tr>")
                    .arg(minVal)
                    .arg(maxVal);

    return html;
}

QString HelpPanel::vectorHelp(int size, double minVal, double maxVal)
{
    QString html = sectionHeader("Element Count");
    QString elmnt;
    if (size)
        elmnt = QString::number(size);
    else
        elmnt = "Variable";

    html += QString("<tr><td style='padding: 2px 0;'>"
                    "  <span style='font-size: 10pt;'>%1</span>"
                    "</td></tr>")
                    .arg(elmnt);

    html += numericHelp("Element range", minVal, maxVal);
    return html;
}

QString HelpPanel::detectCurrentValue(OptionsItem *item)
{
    if (!item)
        return QString();

    QVariant val = item->value();
    return val.isValid() ? val.toString() : QString();
}

QString HelpPanel::currentValue(OptionsItem *item)
{
    if (!item)
        return QString();

    mcconfig::option_type_t type = item->type();

    QString html = sectionHeader("Current Value");

    QString currentVal = detectCurrentValue(item);

    switch (type) {
    case mcconfig::tBool: {
        bool isTrue = (currentVal.toLower() == "true" || currentVal == "1");
        QString valStr = isTrue ? "true" : "false";
        QString bg = isTrue ? "#e8f5e9" : "#fce4ec";
        QString color = isTrue ? "#2e7d32" : "#c62828";
        html += QString("<tr><td style='padding: 2px 0;'>"
                        "  <code style='background: %1; color: %2; padding: 2px 8px; "
                        "  border-radius: 3px; font-size: 10pt; font-weight: bold;'>%3</code>"
                        "</td></tr>")
                        .arg(bg, color, valStr);
    } break;
    case mcconfig::tEnum:
    case mcconfig::tInt:
    case mcconfig::tFloat:
    case mcconfig::tVector:
    case mcconfig::tIntVector:
    case mcconfig::tString: {
        if (!currentVal.isEmpty()) {
            html += QString("<tr><td style='padding: 2px 0;'>"
                            "  <span style='font-size: 11pt; font-weight: bold; color: "
                            "#1565c0;'>%1</span>"
                            "</td></tr>")
                            .arg(currentVal);
        }
    } break;
    default:
        break;
    }
    return html;
}

QString HelpPanel::generateHelpHtml(OptionsItem *item)
{
    if (!item)
        return QString();

    mcconfig::option_type_t type = item->type();
    QString label = item->name().isEmpty() ? item->key() : item->name();
    QString html =
            preample(label, item->path(), item->toolTip(), QString(toString(type)).toUpper());

    switch (type) {
    case mcconfig::tBool:
        break;
    case mcconfig::tEnum:
        html += enumHelp(item);
        break;
    case mcconfig::tInt:
    case mcconfig::tFloat:
        html += numericHelp(item);
        break;
    case mcconfig::tVector:
    case mcconfig::tIntVector:
        html += vectorHelp(item);
        break;
    case mcconfig::tString:
        break;
    default:
        break;
    }

    html += currentValue(item);

    QString notes = item->notes();
    if (!notes.isEmpty()) {
        html += notesHelp(notes.split(QChar('\n')));
    }

    html += "</table></body></html>";
    return html;
}

QString HelpPanel::enumHelp(OptionsItem *item)
{
    EnumOptionsItem *enumItem = dynamic_cast<EnumOptionsItem *>(item);
    if (!enumItem)
        return QString();

    return enumHelp(enumItem->values(), enumItem->valueLabels(),
                    enumItem->valueValueDescriptions());
}

QString HelpPanel::numericHelp(OptionsItem *item)
{
    double minVal = 0, maxVal = 0;
    if (auto *fi = dynamic_cast<FloatOptionsItem *>(item)) {
        minVal = fi->min();
        maxVal = fi->max();
    } else if (auto *ii = dynamic_cast<IntOptionsItem *>(item)) {
        minVal = ii->min();
        maxVal = ii->max();
    }

    return numericHelp("Range", minVal, maxVal);
}

QString HelpPanel::vectorHelp(OptionsItem *item)
{
    double minVal = 0, maxVal = 0;
    int size;
    if (auto *vi = dynamic_cast<VectorOptionsItem *>(item)) {
        minVal = vi->min();
        maxVal = vi->max();
        size = vi->size();
    } else if (auto *ivi = dynamic_cast<IVectorOptionsItem *>(item)) {
        minVal = ivi->min();
        maxVal = ivi->max();
        size = ivi->size();
    }
    return vectorHelp(size, minVal, maxVal);
}

namespace {
QString toString(const ojson &j)
{
    return QString::fromStdString(j.template get<std::string>());
}
QStringList toStringList(const ojson &j)
{
    std::vector<std::string> s;
    QStringList S;
    j.get_to(s);
    for (auto &i : s)
        S << QString::fromStdString(i);
    return S;
}
} // namespace
QString HelpPanel::generateHelpHtml(HelpData *item, int section)
{
    if (!item || (section < 0) || (section >= item->path.size()))
        return QString();

    const ojson &j = item->j[section];

    mcconfig::option_type_t type;
    j["type"].get_to(type);

    QString label = j.contains("label") ? toString(j["label"]) : toString(j["name"]);
    QString toolTip = j.contains("toolTip") ? toString(j["toolTip"]) : QString();
    QString html =
            preample(label, item->path.at(section), toolTip, QString(toString(type)).toUpper());

    switch (type) {
    case mcconfig::tBool:
        break;
    case mcconfig::tEnum:
        html += enumHelp(item, section);
        break;
    case mcconfig::tInt:
    case mcconfig::tFloat:
        html += numericHelp(item, section);
        break;
    case mcconfig::tVector:
    case mcconfig::tIntVector:
        html += vectorHelp(item, section);
        break;
    case mcconfig::tString:
        break;
    default:
        break;
    }

    if (j.contains("whatsThis")) {
        std::vector<std::string> s;
        if (j["whatsThis"].is_array())
            j["whatsThis"].get_to(s);
        else {
            std::string s1;
            j["whatsThis"].get_to(s1);
            if (!s1.empty())
                s.push_back(s1);
        }
        if (!s.empty()) {
            QStringList S;
            for (auto &i : s)
                S << QString::fromStdString(i);
            html += notesHelp(S);
        }
    }

    html += "</table></body></html>";
    return html;
}

QString HelpPanel::enumHelp(HelpData *item, int section)
{
    const ojson &j = item->j[section];
    return enumHelp(toStringList(j["values"]), toStringList(j["valueLabels"]),
                    toStringList(j["valueDescriptions"]));
}

QString HelpPanel::numericHelp(HelpData *item, int section)
{
    const ojson &j = item->j[section];
    double min = j["min"].template get<double>();
    double max = j["max"].template get<double>();
    return numericHelp("Range", min, max);
}

QString HelpPanel::vectorHelp(HelpData *item, int section)
{
    const ojson &j = item->j[section];
    double min = j["min"].template get<double>();
    double max = j["max"].template get<double>();
    int size = j["size"].template get<int>();
    return vectorHelp(size, min, max);
}
