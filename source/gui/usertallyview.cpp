#include "usertallyview.h"

#include "json_defs_p.h"
#include "optionsmodel.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFile>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QColor>
#include <QPushButton>
#include <QRegularExpression>
#include <QSpinBox>
#include <QStyledItemDelegate>
#include <QTableView>
#include <QTextBrowser>
#include <QToolButton>
#include <QVBoxLayout>
#include <cmath>

namespace {
const char *kTemplatePath = ":/assets/data/tally_templates.json";

struct VariableInfo {
    const char *name;
    const char *description;
};

const VariableInfo kVars[] = {
    { "x", "Position x" },          { "y", "Position y" },
    { "z", "Position z" },          { "r", "Radius" },
    { "rho", "Cylindrical rho" },   { "cosTheta", "Direction cosine theta" },
    { "nx", "Direction x" },        { "ny", "Direction y" },
    { "nz", "Direction z" },        { "E", "Ion energy" },
    { "Tdam", "Damage energy" },    { "V", "Vacancies" },
    { "atom_id", "Atomic species" }, { "recoil_id", "Recoil generation" }
};

Event parseEvent(const QString &name)
{
    if (name == "IonExit")
        return Event::IonExit;
    if (name == "Vacancy")
        return Event::Vacancy;
    if (name == "Replacement")
        return Event::Replacement;
    if (name == "CascadeComplete")
        return Event::CascadeComplete;
    if (name == "BoundaryCrossing")
        return Event::BoundaryCrossing;
    return Event::IonStop;
}

QString eventToString(Event ev)
{
    return event_name(ev);
}

void setSpin3(QDoubleSpinBox *sb[3], const vector3 &v)
{
    sb[0]->setValue(v.x());
    sb[1]->setValue(v.y());
    sb[2]->setValue(v.z());
}

vector3 spin3Value(QDoubleSpinBox *sb[3])
{
    return vector3(sb[0]->value(), sb[1]->value(), sb[2]->value());
}

bool isLabFrame(const coord_sys &cs)
{
    const vector3 O0(0.f, 0.f, 0.f);
    const vector3 Z0(0.f, 0.f, 1.f);
    const vector3 XZ0(1.f, 0.f, 1.f);
    return cs.origin == O0 && cs.zaxis == Z0 && cs.xzvector == XZ0;
}

std::vector<BinVariableModel::Row> binsToRows(const user_tally::bin_var_t &bins)
{
    std::vector<BinVariableModel::Row> rows;
    auto pushIf = [&rows](const char *name, const std::vector<float> &v) {
        if (v.size() >= 2)
            rows.push_back({ name, BinVariableModel::edgesToString(v) });
    };

    pushIf("x", bins.x);
    pushIf("y", bins.y);
    pushIf("z", bins.z);
    pushIf("r", bins.r);
    pushIf("rho", bins.rho);
    pushIf("cosTheta", bins.cosTheta);
    pushIf("nx", bins.nx);
    pushIf("ny", bins.ny);
    pushIf("nz", bins.nz);
    pushIf("E", bins.E);
    pushIf("Tdam", bins.Tdam);
    pushIf("V", bins.V);
    pushIf("atom_id", bins.atom_id);
    pushIf("recoil_id", bins.recoil_id);

    return rows;
}

void rowsToBins(const std::vector<BinVariableModel::Row> &rows, user_tally::bin_var_t &bins)
{
    bins = user_tally::bin_var_t();

    for (const auto &r : rows) {
        std::vector<float> edges = BinVariableModel::parseEdges(r.edges);
        if (edges.size() < 2)
            continue;

        if (r.variable == "x")
            bins.x = edges;
        else if (r.variable == "y")
            bins.y = edges;
        else if (r.variable == "z")
            bins.z = edges;
        else if (r.variable == "r")
            bins.r = edges;
        else if (r.variable == "rho")
            bins.rho = edges;
        else if (r.variable == "cosTheta")
            bins.cosTheta = edges;
        else if (r.variable == "nx")
            bins.nx = edges;
        else if (r.variable == "ny")
            bins.ny = edges;
        else if (r.variable == "nz")
            bins.nz = edges;
        else if (r.variable == "E")
            bins.E = edges;
        else if (r.variable == "Tdam")
            bins.Tdam = edges;
        else if (r.variable == "V")
            bins.V = edges;
        else if (r.variable == "atom_id")
            bins.atom_id = edges;
        else if (r.variable == "recoil_id")
            bins.recoil_id = edges;
    }
}

class VariableComboDelegate : public QStyledItemDelegate
{
public:
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &, const QModelIndex &) const override
    {
        QComboBox *cb = new QComboBox(parent);
        const QStringList vars = BinVariableModel::variableNames();
        for (const QString &v : vars)
            cb->addItem(v + " - " + BinVariableModel::variableDescription(v), v);
        return cb;
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const override
    {
        QComboBox *cb = static_cast<QComboBox *>(editor);
        const QString var = index.model()->data(index, Qt::EditRole).toString();
        int i = cb->findData(var);
        if (i >= 0)
            cb->setCurrentIndex(i);
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override
    {
        QComboBox *cb = static_cast<QComboBox *>(editor);
        model->setData(index, cb->currentData(), Qt::EditRole);
    }
};

} // namespace

BinVariableModel::BinVariableModel(QObject *parent) : QAbstractTableModel(parent) { }

int BinVariableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return static_cast<int>(rows_.size());
}

int BinVariableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 5;
}

QVariant BinVariableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= rowCount())
        return QVariant();

    const Row &r = rows_[index.row()];

    if (role == Qt::BackgroundRole && index.column() == 1) {
        const std::vector<float> edges = parseEdges(r.edges);
        if (!isStrictlyMonotonic(edges))
            return QColor(255, 220, 220);
    }

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    switch (index.column()) {
    case 0:
        return r.variable;
    case 1:
        return r.edges;
    case 2: {
        const std::vector<float> edges = parseEdges(r.edges);
        if (edges.size() < 2)
            return 0;
        return static_cast<int>(edges.size() - 1);
    }
    case 3:
        return "~";
    case 4:
        return "x";
    default:
        break;
    }

    return QVariant();
}

QVariant BinVariableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole || orientation != Qt::Horizontal)
        return QVariant();

    switch (section) {
    case 0:
        return "Variable";
    case 1:
        return "Bin edges";
    case 2:
        return "N bins";
    case 3:
        return "[~]";
    case 4:
        return "[x]";
    default:
        return QVariant();
    }
}

Qt::ItemFlags BinVariableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    Qt::ItemFlags f = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    if (index.column() == 0 || index.column() == 1)
        f |= Qt::ItemIsEditable;
    return f;
}

bool BinVariableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    Row &r = rows_[index.row()];
    if (index.column() == 0)
        r.variable = value.toString();
    else if (index.column() == 1)
        r.edges = value.toString();
    else
        return false;

    emit dataChanged(index, index);
    emit dataChanged(this->index(index.row(), 2), this->index(index.row(), 2));
    emit rowsChanged();
    return true;
}

bool BinVariableModel::insertRows(int row, int count, const QModelIndex &parent)
{
    if (count <= 0)
        return false;

    beginInsertRows(parent, row, row + count - 1);
    for (int i = 0; i < count; ++i)
        rows_.insert(rows_.begin() + row, { "x", "0 1" });
    endInsertRows();
    emit rowsChanged();
    return true;
}

bool BinVariableModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (count <= 0 || row < 0 || row + count > rowCount())
        return false;

    beginRemoveRows(parent, row, row + count - 1);
    rows_.erase(rows_.begin() + row, rows_.begin() + row + count);
    endRemoveRows();
    emit rowsChanged();
    return true;
}

void BinVariableModel::setRows(const std::vector<Row> &rows)
{
    beginResetModel();
    rows_ = rows;
    endResetModel();
    emit rowsChanged();
}

std::vector<BinVariableModel::Row> BinVariableModel::rows() const
{
    return rows_;
}

void BinVariableModel::clear()
{
    beginResetModel();
    rows_.clear();
    endResetModel();
    emit rowsChanged();
}

QStringList BinVariableModel::variableNames()
{
    QStringList vars;
    for (const auto &v : kVars)
        vars << v.name;
    return vars;
}

QString BinVariableModel::variableDescription(const QString &name)
{
    for (const auto &v : kVars) {
        if (name == v.name)
            return v.description;
    }
    return QString();
}

std::vector<float> BinVariableModel::parseEdges(const QString &text)
{
    std::vector<float> out;
    const QStringList parts = text.split(QRegularExpression("[\\s,;]+"), Qt::SkipEmptyParts);
    for (const QString &p : parts) {
        bool ok = false;
        float v = p.toFloat(&ok);
        if (ok)
            out.push_back(v);
    }
    return out;
}

QString BinVariableModel::edgesToString(const std::vector<float> &edges)
{
    QStringList s;
    for (float v : edges)
        s << QString::number(v, 'g', 8);
    return s.join(' ');
}

bool BinVariableModel::isStrictlyMonotonic(const std::vector<float> &edges)
{
    if (edges.size() < 2)
        return false;
    for (size_t i = 1; i < edges.size(); ++i) {
        if (!(edges[i] > edges[i - 1]))
            return false;
    }
    return true;
}

LinspaceDialog::LinspaceDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Generate bin edges");
    QVBoxLayout *v = new QVBoxLayout(this);

    QFormLayout *f = new QFormLayout;
    min_ = new QDoubleSpinBox;
    max_ = new QDoubleSpinBox;
    bins_ = new QSpinBox;

    min_->setRange(-1e9, 1e9);
    max_->setRange(-1e9, 1e9);
    min_->setDecimals(6);
    max_->setDecimals(6);
    bins_->setRange(1, 1000000);

    min_->setValue(0.0);
    max_->setValue(1.0);
    bins_->setValue(10);

    f->addRow("Min value:", min_);
    f->addRow("Max value:", max_);
    f->addRow("N bins:", bins_);
    v->addLayout(f);

    preview_ = new QLabel;
    v->addWidget(preview_);

    QDialogButtonBox *box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(box, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(box, &QDialogButtonBox::rejected, this, &QDialog::reject);
    v->addWidget(box);

    connect(min_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            &LinspaceDialog::updatePreview);
    connect(max_, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
            &LinspaceDialog::updatePreview);
    connect(bins_, QOverload<int>::of(&QSpinBox::valueChanged), this,
            &LinspaceDialog::updatePreview);
    updatePreview();
}

std::vector<float> LinspaceDialog::edges() const
{
    const int n = bins_->value();
    const double a = min_->value();
    const double b = max_->value();
    std::vector<float> e;
    e.reserve(static_cast<size_t>(n) + 1);

    if (n <= 0)
        return e;

    const double step = (b - a) / static_cast<double>(n);
    for (int i = 0; i <= n; ++i)
        e.push_back(static_cast<float>(a + step * i));

    return e;
}

void LinspaceDialog::updatePreview()
{
    const std::vector<float> e = edges();
    if (e.size() < 2) {
        preview_->setText("No edges generated");
        return;
    }

    QString txt = QString("%1 edges, %2 bins\n").arg(e.size()).arg(e.size() - 1);
    txt += QString("%1, %2, ... , %3, %4")
                   .arg(e.front(), 0, 'g', 6)
                   .arg(e[1], 0, 'g', 6)
                   .arg(e[e.size() - 2], 0, 'g', 6)
                   .arg(e.back(), 0, 'g', 6);
    preview_->setText(txt);
}

TallyTemplateDialog::TallyTemplateDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Tally Templates");
    resize(760, 460);

    QVBoxLayout *root = new QVBoxLayout(this);
    QHBoxLayout *body = new QHBoxLayout;

    list_ = new QListWidget;
    preview_ = new QTextBrowser;
    preview_->setReadOnly(true);

    body->addWidget(list_, 1);
    body->addWidget(preview_, 2);
    root->addLayout(body, 1);

    QDialogButtonBox *box = new QDialogButtonBox(QDialogButtonBox::Cancel);
    accept_ = box->addButton("Load into current tally", QDialogButtonBox::AcceptRole);
    accept_->setEnabled(false);
    connect(box, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(box, &QDialogButtonBox::rejected, this, &QDialog::reject);
    root->addWidget(box);

    connect(list_, &QListWidget::currentRowChanged, this, &TallyTemplateDialog::onCurrentTemplateChanged);

    const bool ok = loadTemplates();
    if (ok && list_->count() > 0)
        list_->setCurrentRow(0);
    else {
        preview_->setPlainText("No templates available. Check resource file tally_templates.json.");
        accept_->setEnabled(false);
    }
}

bool TallyTemplateDialog::loadTemplates()
{
    entries_.clear();

    static bool cacheInitialized = false;
    static ojson cache;

    if (!cacheInitialized) {
        QFile f(kTemplatePath);
        if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
            return false;

        try {
            cache = ojson::parse(f.readAll().toStdString(), nullptr, true, true);
            cacheInitialized = true;
        } catch (...) {
            return false;
        }
    }

    const ojson &j = cache;

    if (!j.is_array())
        return false;

    for (const auto &node : j) {
        if (!node.is_object())
            continue;

        TemplateEntry e;
        e.parameters.id = node.value("id", std::string("Template"));
        e.parameters.description = node.value("description", std::string());
        e.parameters.event = parseEvent(QString::fromStdString(node.value("event", std::string("IonStop"))));

        if (node.contains("bins") && node["bins"].is_object()) {
            for (auto it = node["bins"].begin(); it != node["bins"].end(); ++it) {
                std::vector<float> edges;
                if (it.value().is_array())
                    edges = it.value().get<std::vector<float>>();

                const std::string k = it.key();
                if (k == "x")
                    e.parameters.bins.x = edges;
                else if (k == "y")
                    e.parameters.bins.y = edges;
                else if (k == "z")
                    e.parameters.bins.z = edges;
                else if (k == "r")
                    e.parameters.bins.r = edges;
                else if (k == "rho")
                    e.parameters.bins.rho = edges;
                else if (k == "cosTheta")
                    e.parameters.bins.cosTheta = edges;
                else if (k == "nx")
                    e.parameters.bins.nx = edges;
                else if (k == "ny")
                    e.parameters.bins.ny = edges;
                else if (k == "nz")
                    e.parameters.bins.nz = edges;
                else if (k == "E")
                    e.parameters.bins.E = edges;
                else if (k == "Tdam")
                    e.parameters.bins.Tdam = edges;
                else if (k == "V")
                    e.parameters.bins.V = edges;
                else if (k == "atom_id")
                    e.parameters.bins.atom_id = edges;
                else if (k == "recoil_id")
                    e.parameters.bins.recoil_id = edges;
            }
        }

        entries_.push_back(e);
        list_->addItem(QString::fromStdString(e.parameters.id));
    }

    return !entries_.empty();
}

user_tally::parameters TallyTemplateDialog::selectedTemplate() const
{
    int i = list_->currentRow();
    if (i < 0 || i >= static_cast<int>(entries_.size()))
        return user_tally::parameters();
    return entries_[i].parameters;
}

void TallyTemplateDialog::refreshPreview(int row)
{
    if (row < 0 || row >= static_cast<int>(entries_.size())) {
        preview_->clear();
        accept_->setEnabled(false);
        return;
    }

    const auto &p = entries_[row].parameters;
    std::vector<BinVariableModel::Row> rows = binsToRows(p.bins);

    QString html;
    html += QString("<b>Event:</b> %1<br>").arg(eventToString(p.event));
    html += QString("<b>Description:</b> %1<br><br>")
                    .arg(QString::fromStdString(p.description).toHtmlEscaped());
    html += "<b>Bins:</b><br>";
    for (const auto &r : rows) {
        const std::vector<float> edges = BinVariableModel::parseEdges(r.edges);
        if (edges.size() >= 2) {
            html += QString("%1 : %2 &rarr; %3 &nbsp; (%4 bins)<br>")
                            .arg(r.variable.toHtmlEscaped())
                            .arg(edges.front(), 0, 'g', 4)
                            .arg(edges.back(), 0, 'g', 4)
                            .arg(edges.size() - 1);
        } else {
            html += QString("%1 : %2<br>")
                            .arg(r.variable.toHtmlEscaped())
                            .arg(r.edges.toHtmlEscaped());
        }
    }

    preview_->setHtml(html);
    accept_->setEnabled(true);
}

void TallyTemplateDialog::onCurrentTemplateChanged(int row)
{
    refreshPreview(row);
}

UserTallyView::UserTallyView(OptionsModel *m, QWidget *parent)
    : QWidget(parent),
      model_(m),
      binModel_(new BinVariableModel(this))
{
    tallyIndex_ = model_->index("UserTally");

    QVBoxLayout *root = new QVBoxLayout(this);

    QHBoxLayout *top = new QHBoxLayout;
    top->addWidget(new QLabel("Tally:"));
    cbTallyID_ = new QComboBox;
    cbTallyID_->setMinimumContentsLength(16);
    top->addWidget(cbTallyID_, 1);

    btDbTally_ = new QToolButton;
    btDbTally_->setIcon(QIcon(":/assets/lucide/database-search.svg"));
    btDbTally_->setToolTip("Load tally template");
    top->addWidget(btDbTally_);

    btAddTally_ = new QToolButton;
    btAddTally_->setIcon(QIcon(":/assets/ionicons/add-outline.svg"));
    btAddTally_->setToolTip("Add new tally");
    top->addWidget(btAddTally_);

    btDupTally_ = new QToolButton;
    btDupTally_->setIcon(QIcon(":/assets/ionicons/create-outline.svg"));
    btDupTally_->setToolTip("Duplicate tally");
    top->addWidget(btDupTally_);

    btDelTally_ = new QToolButton;
    btDelTally_->setIcon(QIcon(":/assets/ionicons/remove-outline.svg"));
    btDelTally_->setToolTip("Remove tally");
    top->addWidget(btDelTally_);

    btEdtTally_ = new QToolButton;
    btEdtTally_->setIcon(QIcon(":/assets/ionicons/create-outline.svg"));
    btEdtTally_->setToolTip("Rename tally");
    top->addWidget(btEdtTally_);

    root->addLayout(top);

    summaryLabel_ = new QLabel("No bins defined - tally is inactive");
    memoryLabel_ = new QLabel("Memory: 0 B per tally flush");
    warningLabel_ = new QLabel;
    warningLabel_->setStyleSheet("color: #b22222;");
    root->addWidget(summaryLabel_);
    root->addWidget(memoryLabel_);
    root->addWidget(warningLabel_);

    QFormLayout *form = new QFormLayout;
    leDescription_ = new QLineEdit;
    cbEvent_ = new QComboBox;
    cbEvent_->addItems({ "IonStop", "IonExit", "Vacancy", "Replacement", "CascadeComplete", "BoundaryCrossing" });
    form->addRow("Description:", leDescription_);
    form->addRow("Event:", cbEvent_);
    root->addLayout(form);

    QGroupBox *csBox = new QGroupBox("Coordinate system");
    QVBoxLayout *csV = new QVBoxLayout(csBox);
    cbUseLabFrame_ = new QCheckBox("Use lab frame");
    csV->addWidget(cbUseLabFrame_);

    coordWidget_ = new QWidget;
    QGridLayout *cg = new QGridLayout(coordWidget_);

    for (int i = 0; i < 3; ++i) {
        sbOrigin_[i] = new QDoubleSpinBox;
        sbZaxis_[i] = new QDoubleSpinBox;
        sbXZvec_[i] = new QDoubleSpinBox;

        sbOrigin_[i]->setRange(-1e6, 1e6);
        sbZaxis_[i]->setRange(-1e3, 1e3);
        sbXZvec_[i]->setRange(-1e3, 1e3);

        sbOrigin_[i]->setDecimals(6);
        sbZaxis_[i]->setDecimals(6);
        sbXZvec_[i]->setDecimals(6);
    }

    cg->addWidget(new QLabel("Origin:"), 0, 0);
    cg->addWidget(sbOrigin_[0], 0, 1);
    cg->addWidget(sbOrigin_[1], 0, 2);
    cg->addWidget(sbOrigin_[2], 0, 3);

    cg->addWidget(new QLabel("Z-axis:"), 1, 0);
    cg->addWidget(sbZaxis_[0], 1, 1);
    cg->addWidget(sbZaxis_[1], 1, 2);
    cg->addWidget(sbZaxis_[2], 1, 3);

    cg->addWidget(new QLabel("XZ-plane vec:"), 2, 0);
    cg->addWidget(sbXZvec_[0], 2, 1);
    cg->addWidget(sbXZvec_[1], 2, 2);
    cg->addWidget(sbXZvec_[2], 2, 3);

    csV->addWidget(coordWidget_);
    root->addWidget(csBox);

    QHBoxLayout *binsHdr = new QHBoxLayout;
    binsHdr->addWidget(new QLabel("Bin variables"));
    binsHdr->addStretch();
    btAddBin_ = new QToolButton;
    btAddBin_->setIcon(QIcon(":/assets/ionicons/add-outline.svg"));
    btAddBin_->setToolTip("Add variable");
    binsHdr->addWidget(btAddBin_);
    root->addLayout(binsHdr);

    binTableView_ = new QTableView;
    binTableView_->setModel(binModel_);
    binTableView_->setItemDelegateForColumn(0, new VariableComboDelegate);
    binTableView_->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    binTableView_->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    binTableView_->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    binTableView_->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    binTableView_->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    root->addWidget(binTableView_, 1);

    connect(btAddTally_, &QToolButton::clicked, this, &UserTallyView::addTally);
    connect(btDupTally_, &QToolButton::clicked, this, &UserTallyView::duplicateTally);
    connect(btDelTally_, &QToolButton::clicked, this, &UserTallyView::removeTally);
    connect(btEdtTally_, &QToolButton::clicked, this, &UserTallyView::renameTally);
    connect(btDbTally_, &QToolButton::clicked, this, &UserTallyView::loadFromTemplate);
    connect(btAddBin_, &QToolButton::clicked, this, &UserTallyView::addBinVariable);

    connect(cbTallyID_, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &UserTallyView::updateSelectedTally);
    connect(leDescription_, &QLineEdit::editingFinished, this, &UserTallyView::setValueData);
    connect(cbEvent_, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &UserTallyView::setValueData);
    connect(cbUseLabFrame_, &QCheckBox::stateChanged, this, &UserTallyView::onUseLabFrameChanged);
    connect(binModel_, &BinVariableModel::rowsChanged, this, &UserTallyView::setValueData);
    connect(binModel_, &BinVariableModel::rowsChanged, this, &UserTallyView::updateSummaryLabel);
    connect(binTableView_, &QTableView::clicked, this, &UserTallyView::onBinTableClicked);

    for (int i = 0; i < 3; ++i) {
        connect(sbOrigin_[i], QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
                &UserTallyView::setValueData);
        connect(sbZaxis_[i], QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
                &UserTallyView::setValueData);
        connect(sbXZvec_[i], QOverload<double>::of(&QDoubleSpinBox::valueChanged), this,
                &UserTallyView::setValueData);
    }

    setWidgetData();
}

void UserTallyView::setWidgetData()
{
    cbTallyID_->blockSignals(true);
    cbTallyID_->clear();

    auto &ut = model_->options()->UserTally;
    for (const auto &t : ut)
        cbTallyID_->addItem(QString::fromStdString(t.id));

    cbTallyID_->blockSignals(false);

    if (!ut.empty()) {
        cbTallyID_->setCurrentIndex(0);
        updateSelectedTally();
    } else {
        leDescription_->clear();
        binModel_->clear();
        warningLabel_->clear();
        updateSummaryLabel();
    }

    const bool has = !ut.empty();
    btDupTally_->setEnabled(has);
    btDelTally_->setEnabled(has);
    btEdtTally_->setEnabled(has);
}

void UserTallyView::setValueData()
{
    if (syncingUi_)
        return;

    user_tally::parameters *t = currentTally();
    if (!t)
        return;

    t->description = leDescription_->text().toStdString();
    t->event = currentEvent();

    if (cbUseLabFrame_->isChecked()) {
        t->coordinate_system.reset();
    } else {
        t->coordinate_system.origin = spin3Value(sbOrigin_);
        t->coordinate_system.zaxis = spin3Value(sbZaxis_);
        t->coordinate_system.xzvector = spin3Value(sbXZvec_);
    }

    rowsToBins(binModel_->rows(), t->bins);

    model_->notifyDataChanged(tallyIndex_);
    updateSummaryLabel();
}

void UserTallyView::addTally()
{
    user_tally::parameters p;
    p.id = QString("Tally%1").arg(model_->options()->UserTally.size() + 1).toStdString();
    p.description = "User tally";
    p.event = Event::IonStop;

    model_->options()->UserTally.push_back(p);
    model_->notifyDataChanged(tallyIndex_);

    setWidgetData();
    cbTallyID_->setCurrentIndex(cbTallyID_->count() - 1);
}

void UserTallyView::removeTally()
{
    int i = cbTallyID_->currentIndex();
    auto &ut = model_->options()->UserTally;
    if (i < 0 || i >= static_cast<int>(ut.size()))
        return;

    QMessageBox::StandardButton ret = QMessageBox::warning(
            this, "Remove tally",
            QString("%1 is being removed.\nClick OK to proceed.").arg(cbTallyID_->currentText()),
            QMessageBox::Ok | QMessageBox::Cancel);
    if (ret != QMessageBox::Ok)
        return;

    ut.erase(ut.begin() + i);
    model_->notifyDataChanged(tallyIndex_);
    setWidgetData();
}

void UserTallyView::duplicateTally()
{
    int i = cbTallyID_->currentIndex();
    auto &ut = model_->options()->UserTally;
    if (i < 0 || i >= static_cast<int>(ut.size()))
        return;

    user_tally::parameters copy = ut[i];
    QString baseId = QString::fromStdString(copy.id);
    if (baseId.isEmpty())
        baseId = "Tally";

    QString newId = baseId + "_copy";
    int suffix = 2;
    auto exists = [&ut](const QString &id) {
        for (const auto &t : ut) {
            if (QString::fromStdString(t.id) == id)
                return true;
        }
        return false;
    };
    while (exists(newId)) {
        newId = QString("%1_copy%2").arg(baseId).arg(suffix++);
    }

    copy.id = newId.toStdString();
    ut.push_back(copy);
    model_->notifyDataChanged(tallyIndex_);

    setWidgetData();
    cbTallyID_->setCurrentText(newId);
}

void UserTallyView::renameTally()
{
    int i = cbTallyID_->currentIndex();
    auto &ut = model_->options()->UserTally;
    if (i < 0 || i >= static_cast<int>(ut.size()))
        return;

    bool ok = false;
    QString s = QInputDialog::getText(this, "Rename tally", "New tally id", QLineEdit::Normal,
                                      cbTallyID_->currentText(), &ok);
    if (!ok || s.trimmed().isEmpty())
        return;

    ut[i].id = s.toStdString();
    model_->notifyDataChanged(tallyIndex_);

    setWidgetData();
    cbTallyID_->setCurrentText(s);
}

void UserTallyView::addBinVariable()
{
    binModel_->insertRows(binModel_->rowCount(), 1);
}

void UserTallyView::loadFromTemplate()
{
    user_tally::parameters *t = currentTally();
    if (!t) {
        addTally();
        t = currentTally();
        if (!t)
            return;
    }

    TallyTemplateDialog dlg(this);

    if (dlg.exec() != QDialog::Accepted)
        return;

    user_tally::parameters p = dlg.selectedTemplate();
    t->description = p.description;
    t->event = p.event;
    t->bins = p.bins;

    updateSelectedTally();
    model_->notifyDataChanged(tallyIndex_);
}

void UserTallyView::updateSelectedTally()
{
    const user_tally::parameters *t = currentTally();
    if (!t)
        return;

    syncingUi_ = true;

    leDescription_->blockSignals(true);
    cbEvent_->blockSignals(true);

    leDescription_->setText(QString::fromStdString(t->description));
    setCurrentEvent(t->event);

    setSpin3(sbOrigin_, t->coordinate_system.origin);
    setSpin3(sbZaxis_, t->coordinate_system.zaxis);
    setSpin3(sbXZvec_, t->coordinate_system.xzvector);

    cbUseLabFrame_->setChecked(isLabFrame(t->coordinate_system));
    onUseLabFrameChanged();

    binModel_->setRows(binsToRows(t->bins));

    leDescription_->blockSignals(false);
    cbEvent_->blockSignals(false);
    syncingUi_ = false;

    btDelTally_->setEnabled(true);
    btEdtTally_->setEnabled(true);
    btDupTally_->setEnabled(true);

    updateSummaryLabel();
}

void UserTallyView::updateSummaryLabel()
{
    const std::vector<BinVariableModel::Row> rows = binModel_->rows();

    QStringList names;
    QStringList dims;
    long long totalBins = 1;
    bool hasBins = false;
    bool hasInvalid = false;

    for (const auto &r : rows) {
        const std::vector<float> edges = BinVariableModel::parseEdges(r.edges);
        if (edges.size() < 2) {
            hasInvalid = true;
            continue;
        }
        if (!BinVariableModel::isStrictlyMonotonic(edges)) {
            hasInvalid = true;
            continue;
        }

        const int n = static_cast<int>(edges.size() - 1);
        names << r.variable;
        dims << QString::number(n);
        totalBins *= n;
        hasBins = true;
    }

    if (!hasBins) {
        summaryLabel_->setText("No bins defined - tally is inactive");
        summaryLabel_->setStyleSheet("");
        memoryLabel_->setText("Memory: 0 B per tally flush");
    } else if (names.size() == 1) {
        summaryLabel_->setText(
                QString("1D tally: %1  (%2 bins)").arg(names.join(" ")).arg(totalBins));
    } else {
        summaryLabel_->setText(QString("%1D tally: %2  (%3 = %4 bins)")
                                       .arg(names.size())
                                       .arg(names.join(" x "))
                                       .arg(dims.join(" x "))
                                       .arg(totalBins));
    }

    if (hasBins) {
        if (totalBins >= 200000)
            summaryLabel_->setStyleSheet("color: #b22222; font-weight: bold;");
        else if (totalBins >= 10000)
            summaryLabel_->setStyleSheet("color: #e07000; font-weight: bold;");
        else
            summaryLabel_->setStyleSheet("");
    }

    const double bytes = static_cast<double>(totalBins) * sizeof(double);
    QString memStr;
    if (bytes < 1024.0)
        memStr = QString("%1 B").arg(bytes, 0, 'f', 0);
    else if (bytes < 1024.0 * 1024.0)
        memStr = QString("%1 KB").arg(bytes / 1024.0, 0, 'f', 1);
    else
        memStr = QString("%1 MB").arg(bytes / (1024.0 * 1024.0), 0, 'f', 2);
    memoryLabel_->setText(QString("Memory: ~%1 per tally flush").arg(memStr));

    QStringList warnings;
    if (hasInvalid)
        warnings << "Bin edges must be strictly monotonic for all rows.";
    if (totalBins > 500000)
        warnings << "Large tally warning: total bins exceed 500000.";
    warningLabel_->setText(warnings.join(" "));
}

void UserTallyView::applyLinspaceToCurrentRow()
{
    const QModelIndex idx = binTableView_->currentIndex();
    if (!idx.isValid())
        return;

    LinspaceDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted)
        return;

    const std::vector<float> edges = dlg.edges();
    if (edges.size() < 2)
        return;

    binModel_->setData(binModel_->index(idx.row(), 1), BinVariableModel::edgesToString(edges));
}

void UserTallyView::onBinTableClicked(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    if (index.column() == 3) {
        binTableView_->setCurrentIndex(index);
        applyLinspaceToCurrentRow();
    } else if (index.column() == 4) {
        binModel_->removeRows(index.row(), 1);
    }
}

void UserTallyView::onUseLabFrameChanged(int)
{
    const bool useLab = cbUseLabFrame_->isChecked();
    coordWidget_->setVisible(!useLab);
    setValueData();
}

Event UserTallyView::currentEvent() const
{
    return parseEvent(cbEvent_->currentText());
}

void UserTallyView::setCurrentEvent(Event ev)
{
    int i = cbEvent_->findText(eventToString(ev));
    if (i < 0)
        i = 0;
    cbEvent_->setCurrentIndex(i);
}

user_tally::parameters *UserTallyView::currentTally()
{
    int i = cbTallyID_->currentIndex();
    auto &ut = model_->options()->UserTally;
    if (i < 0 || i >= static_cast<int>(ut.size()))
        return nullptr;
    return &ut[i];
}

const user_tally::parameters *UserTallyView::currentTally() const
{
    int i = cbTallyID_->currentIndex();
    const auto &ut = model_->options()->UserTally;
    if (i < 0 || i >= static_cast<int>(ut.size()))
        return nullptr;
    return &ut[i];
}
