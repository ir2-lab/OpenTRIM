#include "utallyview.h"
#include "materialsdefview.h" // reuse MyComboBox
#include "qstring_vector_serialize.h"
#include "optionsmodel.h"
#include "optionsview.h"
#include "mainui.h"
#include "helppanel.h"

#include <QtVectorEdit/qvectoredit.h>

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QToolButton>
#include <QComboBox>
#include <QStandardItemModel>
#include <QTableView>
#include <QHeaderView>
#include <QInputDialog>
#include <QMessageBox>
#include <QLineEdit>
#include <QFontMetrics>
#include <QItemSelectionModel>
#include <QDebug>

#include <sstream>

#include "json_defs_p.h"

namespace {

struct FieldSpec
{
    float min{ 0 };
    float max{ 0 };
    int digits{ 6 };
    QString label;
    QString toolTip;
};

// Obtain per-subfield min/max/digits/label/toolTip specs. Used for both
// "coordinate_system" (3 entries) and "bins" (14 entries)
QMap<QString, FieldSpec> loadUserTallyFieldSpecs(const QString &path)
{
    QMap<QString, FieldSpec> result;

    ojson json_templ = ojson::parse(mcconfig::config_template().toJSON());
    ojson utallySpec = spec_for_path(json_templ, ojson::json_pointer(path.toUtf8().constData()));

    ojson::array_t subFields = utallySpec["fields"];
    for (size_t k = 0; k < subFields.size(); ++k) {
        const ojson &sub = subFields[k];
        std::string sname;
        sub["name"].get_to(sname);

        FieldSpec spec;
        sub["min"].get_to(spec.min);
        sub["max"].get_to(spec.max);
        sub["digits"].get_to(spec.digits);
        std::string lbl, tt;
        sub["label"].get_to(lbl);
        sub["toolTip"].get_to(tt);
        spec.label = QString::fromStdString(lbl);
        spec.toolTip = QString::fromStdString(tt);

        result.insert(QString::fromStdString(sname), spec);
    }

    return result;
}
// Order matches user_tally::bin_var_t's declaration order in user_tally.h
// and the field order in parse_json.cpp's
// MY_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(user_tally::bin_var_t, ...).
const char *kBinVarNames[] = { "x",  "y",  "z",  "r",        "rho", "cosTheta", "nx",
                               "ny", "nz", "E",  "Tdam",     "V",   "atom_id",  "recoil_id" };
const int kBinVarCount = int(sizeof(kBinVarNames) / sizeof(kBinVarNames[0]));

// The 6 event types allowed by options_spec.json's "/UserTally/i/event"
// (a subset of the full Event enum -- the rest are internal-only events).
const char *kEventNames[] = { "IonExit",         "IonStop",  "Vacancy",
                              "Replacement",     "CascadeComplete", "BoundaryCrossing" };
const int kEventCount = int(sizeof(kEventNames) / sizeof(kEventNames[0]));

Event eventFromName(const QString &s)
{
    if (s == QLatin1String("IonExit"))
        return Event::IonExit;
    if (s == QLatin1String("IonStop"))
        return Event::IonStop;
    if (s == QLatin1String("Vacancy"))
        return Event::Vacancy;
    if (s == QLatin1String("Replacement"))
        return Event::Replacement;
    if (s == QLatin1String("CascadeComplete"))
        return Event::CascadeComplete;
    if (s == QLatin1String("BoundaryCrossing"))
        return Event::BoundaryCrossing;
    return Event::IonStop;
}

QString eventToName(Event e)
{
    switch (e) {
    case Event::IonExit:
        return QStringLiteral("IonExit");
    case Event::IonStop:
        return QStringLiteral("IonStop");
    case Event::Vacancy:
        return QStringLiteral("Vacancy");
    case Event::Replacement:
        return QStringLiteral("Replacement");
    case Event::CascadeComplete:
        return QStringLiteral("CascadeComplete");
    case Event::BoundaryCrossing:
        return QStringLiteral("BoundaryCrossing");
    default:
        return QStringLiteral("IonStop");
    }
}

} // namespace

/*****************************************************************************/
UserTallyBinsModel::UserTallyBinsModel(OptionsModel *m, QObject *parent)
    : QAbstractTableModel(parent), model_(m)
{
    utallyIndex_ = model_->index("UserTally", 1);

    QMap<QString, FieldSpec> specs = loadUserTallyFieldSpecs("/UserTally/0/bins");
    for (int v = 0; v < kBinVarCount; ++v) {
        bin_var_spec_t bv;
        bv.name = QString::fromLatin1(kBinVarNames[v]);
        auto it = specs.constFind(bv.name);
        if (it != specs.constEnd()) {
            bv.label = it->label;
            bv.toolTip = it->toolTip;
            bv.min = it->min;
            bv.max = it->max;
            bv.digits = it->digits;
        }
        varSpecs_.push_back(bv);
    }
}

QString UserTallyBinsModel::binPath(int varIdx) const
{
    return QString("/UserTally/%1/bins/%2").arg(tallyIdx_).arg(varSpecs_[varIdx].name);
}

void UserTallyBinsModel::notifyChanged()
{
    model_->dataChanged(utallyIndex_, utallyIndex_);
}

void UserTallyBinsModel::setTallyIdx(int i, const QVector<int> &savedOrder)
{
    beginResetModel();
    tallyIdx_ = i;

    if (i < 0) {
        rowVar_.clear();
    } else if (!savedOrder.isEmpty()) {
        rowVar_ = savedOrder;
    } else {
        rowVar_.clear();
        std::ostringstream os;
        for (int v = 0; v < varSpecs_.size(); ++v) {
            std::string jsonStr;
            model_->options()->get(binPath(v).toStdString(), jsonStr, &os);
            QString t = QString::fromStdString(jsonStr).trimmed();
            if (!t.isEmpty() && t != QLatin1String("[]"))
                rowVar_.push_back(v);
        }
    }

    endResetModel();
}

int UserTallyBinsModel::rowCount(const QModelIndex & /* parent */) const
{
    return rowVar_.size();
}
int UserTallyBinsModel::columnCount(const QModelIndex & /* parent */) const
{
    return 2;
}

QVariant UserTallyBinsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || tallyIdx_ < 0)
        return QVariant();

    int i = index.row(), j = index.column();
    if (i < 0 || i >= rowVar_.size() || j < 0 || j >= columnCount())
        return QVariant();

    int varIdx = rowVar_[i];
    const bin_var_spec_t &spec = varSpecs_[varIdx];

    if (role == Qt::ToolTipRole)
        return spec.toolTip;

    if (role != Qt::DisplayRole && role != Qt::EditRole)
        return QVariant();

    if (j == 0)
        return spec.name;

    std::string jsonStr;
    std::ostringstream os;
    model_->options()->get(binPath(varIdx).toStdString(), jsonStr, &os);
    return QString::fromStdString(jsonStr);
}

QVariant UserTallyBinsModel::headerData(int c, Qt::Orientation o, int role) const
{
    if (role == Qt::DisplayRole && o == Qt::Horizontal)
        return c == 0 ? QStringLiteral("Variable") : QStringLiteral("Bin edges");
    if (role == Qt::DisplayRole && o == Qt::Vertical)
        return c + 1;
    return QVariant();
}

Qt::ItemFlags UserTallyBinsModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    return Qt::ItemIsEditable | QAbstractTableModel::flags(index);
}

bool UserTallyBinsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || role != Qt::EditRole || tallyIdx_ < 0)
        return false;

    int i = index.row(), j = index.column();
    if (i < 0 || i >= rowVar_.size() || j < 0 || j >= columnCount())
        return false;

    std::ostringstream os;

    if (j == 0) {
        QString newName = value.toString();
        int newVarIdx = -1;
        for (int v = 0; v < varSpecs_.size(); ++v) {
            if (varSpecs_[v].name == newName) {
                newVarIdx = v;
                break;
            }
        }
        if (newVarIdx < 0 || newVarIdx == rowVar_[i])
            return false;

        int oldVarIdx = rowVar_[i];
        QString oldPath = binPath(oldVarIdx);
        QString newPath = binPath(newVarIdx);

        std::string jsonStr;
        model_->options()->get(oldPath.toStdString(), jsonStr, &os);

        bool ok = model_->options()->set(newPath.toStdString(), jsonStr, &os);
        if (!ok)
            qDebug() << QString::fromStdString(os.str());

        os.str(std::string());
        ok = model_->options()->set(oldPath.toStdString(), "[]", &os);
        if (!ok)
            qDebug() << QString::fromStdString(os.str());

        rowVar_[i] = newVarIdx;

        emit dataChanged(this->index(i, 0), this->index(i, 1),
                         { Qt::DisplayRole, Qt::EditRole, Qt::ToolTipRole });
    } else {
        int varIdx = rowVar_[i];
        QString path = binPath(varIdx);
        std::string text = value.toString().toStdString();

        bool ok = model_->options()->set(path.toStdString(), text, &os);
        if (!ok)
            qDebug() << QString::fromStdString(os.str());

        emit dataChanged(index, index, { Qt::DisplayRole, Qt::EditRole });
    }

    notifyChanged();
    return true;
}

bool UserTallyBinsModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    assert(rows == 1);
    assert(position == rowCount());
    if (tallyIdx_ < 0)
        return false;

    int newVarIdx = -1;
    for (int v = 0; v < varSpecs_.size(); ++v) {
        if (!rowVar_.contains(v)) {
            newVarIdx = v;
            break;
        }
    }
    if (newVarIdx < 0)
        return false;

    beginInsertRows(parent, position, position);
    rowVar_.push_back(newVarIdx);
    model_->options()->set(binPath(newVarIdx).toStdString(), "[0, 1]");
    endInsertRows();

    notifyChanged();
    return true;
}

bool UserTallyBinsModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    assert(rows == 1);
    if (tallyIdx_ < 0 || position < 0 || position >= rowVar_.size())
        return false;

    QString path = binPath(rowVar_[position]);

    beginRemoveRows(parent, position, position);
    rowVar_.erase(rowVar_.begin() + position);
    endRemoveRows();

    std::ostringstream os;
    bool ok = model_->options()->set(path.toStdString(), "[]", &os);
    if (!ok)
        qDebug() << QString::fromStdString(os.str());

    notifyChanged();
    return true;
}

/*****************************************************************************/
UserTallyBinDelegate::UserTallyBinDelegate(QObject *parent) : ValidatingItemDelegate(parent) { }

QWidget *UserTallyBinDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem & /* option */,
                                            const QModelIndex &index) const
{
    if (!index.isValid())
        return nullptr;

    const UserTallyBinsModel *m = qobject_cast<const UserTallyBinsModel *>(index.model());
    if (!m)
        return nullptr;

    int row = index.row();
    if (row < 0 || row >= m->rowVar_.size())
        return nullptr;

    int col = index.column();
    QWidget *w = nullptr;

    switch (col) {
    case 0: {
        QComboBox *cb = new QComboBox(parent);
        QStandardItemModel *sm = new QStandardItemModel(cb);
        int curVar = m->rowVar_[row];
        for (int v = 0; v < m->varSpecs_.size(); ++v) {
            const auto &spec = m->varSpecs_[v];
            QStandardItem *it = new QStandardItem(spec.name);
            it->setToolTip(spec.label);
            bool usedElsewhere = (v != curVar) && m->rowVar_.contains(v);
            if (usedElsewhere)
                it->setFlags(it->flags() & ~Qt::ItemIsEnabled);
            sm->appendRow(it);
        }
        cb->setModel(sm);
        w = cb;
    } break;
    case 1: {
        QVectorEdit *edt = new QVectorEdit(parent);
        edt->setElementType(QVectorEdit::Real);
        edt->setSizeMode(QVectorEdit::Variable);
        edt->setSizeRange({ 2, 1 << 20 });
        const auto &spec = m->varSpecs_[m->rowVar_[row]];
        edt->setMinimum(spec.min);
        edt->setMaximum(spec.max);
        edt->setPrecision(spec.digits);
        edt->setShowEllipsisButton(true);
        w = edt;
    } break;
    }

    return w;
}

void UserTallyBinDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if (!index.isValid())
        return;

    QVariant v = index.model()->data(index, Qt::EditRole);

    switch (index.column()) {
    case 0: {
        QComboBox *cb = static_cast<QComboBox *>(editor);
        cb->setCurrentText(v.toString());
    } break;
    case 1: {
        QVectorEdit *edt = static_cast<QVectorEdit *>(editor);
        edt->setText(v.toString());
    } break;
    }
}

void UserTallyBinDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                        const QModelIndex &index) const
{
    if (!index.isValid())
        return;

    QVariant v;
    switch (index.column()) {
    case 0: {
        QComboBox *cb = static_cast<QComboBox *>(editor);
        v = cb->currentText();
    } break;
    case 1: {
        QVectorEdit *edt = static_cast<QVectorEdit *>(editor);
        v = edt->text();
    } break;
    }

    model->setData(index, v, Qt::EditRole);
}

void UserTallyBinDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                                                const QModelIndex & /* index */) const
{
    editor->setGeometry(option.rect);
}

/*****************************************************************************/
UserTallyView::UserTallyView(OptionsView *v, QWidget *parent)
    : QWidget{ parent }, model_(v->mainui->optionsModel)
{
    utallyIndex_ = model_->index("UserTally", 1);

    QVBoxLayout *vbox = new QVBoxLayout;
    QFormLayout *flayout = new QFormLayout;

    // --- header row: id combo + add/remove/rename buttons ---
    cbTallyID = new MyComboBox;
    cbTallyID->setPlaceholderText("UserTally id");
    cbTallyID->setMinimumContentsLength(15);
    connect(cbTallyID, &MyComboBox::doubleClicked, this, &UserTallyView::editTallyName);
    connect(cbTallyID, &MyComboBox::currentTextChanged, this, &UserTallyView::updateSelectedTally);
    v->helpPanel->addStaticHelp(cbTallyID, "/UserTally/0/id");

    btAddTally = new QToolButton;
    btAddTally->setIcon(QIcon(":/assets/ionicons/add-outline.svg"));
    btAddTally->setToolTip("Add new UserTally");

    btDelTally = new QToolButton;
    btDelTally->setIcon(QIcon(":/assets/ionicons/remove-outline.svg"));
    btDelTally->setToolTip("Remove UserTally");
    btDelTally->setEnabled(false);

    btEdtTally = new QToolButton;
    btEdtTally->setIcon(QIcon(":/assets/ionicons/create-outline.svg"));
    btEdtTally->setToolTip("Rename UserTally");
    btEdtTally->setEnabled(false);

    connect(btAddTally, &QToolButton::clicked, this, &UserTallyView::addTally);
    connect(btDelTally, &QToolButton::clicked, this, &UserTallyView::removeTally);
    connect(btEdtTally, &QToolButton::clicked, this, &UserTallyView::editTallyName);

    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->addWidget(cbTallyID);
    hbox->addWidget(btAddTally);
    hbox->addWidget(btDelTally);
    hbox->addWidget(btEdtTally);
    hbox->setSpacing(0);

    flayout->addRow("Tally ID", hbox);
    v->helpPanel->addStaticHelp(flayout->labelForField(hbox), "/UserTally/0/id");

    // --- description ---
    edtDescription = new DescriptionTextEdit;
    edtDescription->setPlaceholderText("Optional short description");
    {
        QFontMetrics fm = edtDescription->fontMetrics();
        int h = fm.lineSpacing() * 2 + 2 * edtDescription->frameWidth() + 8;
        edtDescription->setFixedHeight(h);
    }
    connect(edtDescription, &DescriptionTextEdit::editingFinished, this,
            &UserTallyView::setDescription);
    flayout->addRow("Description", edtDescription);
    v->helpPanel->addStaticHelp(edtDescription, "/UserTally/0/description");
    v->helpPanel->addStaticHelp(flayout->labelForField(edtDescription), "/UserTally/0/description");

    // --- event ---
    cbEvent = new QComboBox;
    for (int i = 0; i < kEventCount; ++i)
        cbEvent->addItem(QString::fromLatin1(kEventNames[i]));
    connect(cbEvent, QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &UserTallyView::setEvent);
    flayout->addRow("Event", cbEvent);
    v->helpPanel->addStaticHelp(cbEvent, "/UserTally/0/event");
    v->helpPanel->addStaticHelp(flayout->labelForField(cbEvent), "/UserTally/0/event");

    // --- coordinate system ---
    coordBox = new QGroupBox("Coordinate System");
    QFormLayout *coordForm = new QFormLayout;

    QMap<QString, FieldSpec> coordSpecs = loadUserTallyFieldSpecs("/UserTally/0/coordinate_system");
    FieldSpec originSpec = coordSpecs.value("origin");
    FieldSpec zaxisSpec = coordSpecs.value("zaxis");
    FieldSpec xzvectorSpec = coordSpecs.value("xzvector");

    auto makeVectorEdit = [](const FieldSpec &spec) {
        QVectorEdit *edt = new QVectorEdit;
        edt->setElementType(QVectorEdit::Real);
        edt->setSizeMode(QVectorEdit::Fixed);
        edt->setSize(3);
        edt->setMinimum(spec.min);
        edt->setMaximum(spec.max);
        edt->setPrecision(spec.digits);
        edt->setShowEllipsisButton(false);
        return edt;
    };

    edtOrigin = makeVectorEdit(originSpec);
    connect(edtOrigin, &QVectorEdit::editingFinished, this, &UserTallyView::setOrigin);
    coordForm->addRow("Origin", edtOrigin);
    v->helpPanel->addStaticHelp(edtOrigin, "/UserTally/0/coordinate_system/origin");
    v->helpPanel->addStaticHelp(coordForm->labelForField(edtOrigin),
                                "/UserTally/0/coordinate_system/origin");

    edtZAxis = makeVectorEdit(zaxisSpec);
    connect(edtZAxis, &QVectorEdit::editingFinished, this, &UserTallyView::setZAxis);
    coordForm->addRow("Z-axis", edtZAxis);
    v->helpPanel->addStaticHelp(edtZAxis, "/UserTally/0/coordinate_system/zaxis");
    v->helpPanel->addStaticHelp(coordForm->labelForField(edtZAxis),
                                "/UserTally/0/coordinate_system/zaxis");

    edtXZVector = makeVectorEdit(xzvectorSpec);
    connect(edtXZVector, &QVectorEdit::editingFinished, this, &UserTallyView::setXZVector);
    coordForm->addRow("XZ-plane vector", edtXZVector);
    v->helpPanel->addStaticHelp(edtXZVector, "/UserTally/0/coordinate_system/xzvector");
    v->helpPanel->addStaticHelp(coordForm->labelForField(edtXZVector),
                                "/UserTally/0/coordinate_system/xzvector");

    coordBox->setLayout(coordForm);
    v->helpPanel->addStaticHelp(coordBox, "/UserTally/0/coordinate_system");

    // --- top panel layout ---
    {
        QVBoxLayout *vbox1 = new QVBoxLayout;
        vbox1->addLayout(flayout);
        vbox1->addSpacing(12);
        vbox1->addWidget(coordBox);

        QHBoxLayout *hbox = new QHBoxLayout;
        hbox->addLayout(vbox1);
        hbox->addStretch();
        vbox->addLayout(hbox);
    }
    vbox->addSpacing(12);

    // --- bins table ---
    QLabel *binsLabel = new QLabel("Bins");
    v->helpPanel->addStaticHelp(binsLabel, "/UserTally/0/bins");

    btAddBin = new QToolButton;
    btAddBin->setIcon(QIcon(":/assets/ionicons/add-outline.svg"));
    btAddBin->setToolTip("Add bin variable");
    btAddBin->setEnabled(false);

    btRemoveBin = new QToolButton;
    btRemoveBin->setIcon(QIcon(":/assets/ionicons/remove-outline.svg"));
    btRemoveBin->setToolTip("Remove bin variable");
    btRemoveBin->setEnabled(false);

    connect(btAddBin, &QToolButton::clicked, this, &UserTallyView::addBin);
    connect(btRemoveBin, &QToolButton::clicked, this, &UserTallyView::removeBin);

    binsModel = new UserTallyBinsModel(model_, this);
    binsDelegate = new UserTallyBinDelegate(this);

    binsTable = new QTableView;
    binsTable->setModel(binsModel);
    binsTable->setItemDelegate(binsDelegate);

    QFontMetrics fm = binsTable->fontMetrics();
    int minWidth = fm.horizontalAdvance("recoil_id") + 20;
    binsTable->horizontalHeader()->setMinimumSectionSize(minWidth);
    binsTable->horizontalHeader()->setDefaultSectionSize(minWidth);
    binsTable->horizontalHeader()->setStretchLastSection(true);

    // give the table & header a name, so that helpPanel can identify it
    binsTable->setObjectName("binsTable");
    binsTable->horizontalHeader()->setObjectName("binsTable");
    // paths to all bin variables + the path to the bin container object
    const QStringList binPaths = {
        "/UserTally/0/bins/x",       "/UserTally/0/bins/y",         "/UserTally/0/bins/z",
        "/UserTally/0/bins/r",       "/UserTally/0/bins/rho",       "/UserTally/0/bins/cosTheta",
        "/UserTally/0/bins/nx",      "/UserTally/0/bins/ny",        "/UserTally/0/bins/nz",
        "/UserTally/0/bins/E",       "/UserTally/0/bins/Tdam",      "/UserTally/0/bins/V",
        "/UserTally/0/bins/atom_id", "/UserTally/0/bins/recoil_id", "/UserTally/0/bins"
    };
    // set a header property to be read by helpPanel
    // when the mouse is over the header helpPanel shows data for /UserTally/0/bins
    binsTable->horizontalHeader()->setProperty(
            "section", binPaths.size() - 1); // this section num for the header
    v->helpPanel->addStaticHelp(binsTable, binPaths);

    binsSelectionModel = binsTable->selectionModel();
    connect(binsSelectionModel, &QItemSelectionModel::selectionChanged, this,
            &UserTallyView::onBinSelectionChanged);
    connect(binsModel, &UserTallyBinsModel::dataChanged, this, &UserTallyView::onBinsChanged);
    connect(binsModel, &UserTallyBinsModel::rowsInserted, this, &UserTallyView::onBinsChanged);
    connect(binsModel, &UserTallyBinsModel::rowsRemoved, this, &UserTallyView::onBinsChanged);

    hbox = new QHBoxLayout;
    hbox->addWidget(binsLabel);
    hbox->addWidget(btAddBin);
    hbox->addWidget(btRemoveBin);
    hbox->addStretch();

    vbox->addLayout(hbox);
    vbox->addWidget(binsTable);

    setLayout(vbox);

    updateSelectedTally();
}

void UserTallyView::setWidgetData()
{
    auto &tallies = model_->options()->UserTally;

    int i = cbTallyID->currentIndex();
    cbTallyID->blockSignals(true);
    cbTallyID->clear();

    for (const auto &t : tallies) {
        cbTallyID->addItem(QString::fromStdString(t.id));
        cbTallyID->setItemData(cbTallyID->count() - 1, QVariant::fromValue(QVector<int>()),
                               Qt::UserRole);
    }

    int n = tallies.size();
    if (n) {
        if (i < 0)
            i = 0;
        else if (i >= n)
            i = n - 1;
    } else
        i = -1;

    cbTallyID->setCurrentIndex(i);
    cbTallyID->blockSignals(false);

    updateSelectedTally();
}

void UserTallyView::addTally()
{
    QInputDialog dlg(this);
    dlg.setWindowFlags(dlg.windowFlags() & ~Qt::WindowContextHelpButtonHint
                       & ~Qt::WindowMinMaxButtonsHint);
    QString wTitle = tr("OpenTRIM - Add UserTally");
    dlg.setWindowTitle(wTitle);
    dlg.setLabelText(tr("New UserTally id:"));
    dlg.setTextValue(QString("UserTally%1").arg(cbTallyID->count() + 1));
    QFontMetrics fm = fontMetrics();
    QRect rect = fm.boundingRect(wTitle);
    int minW = rect.width() * 2;
    if (auto *edit = dlg.findChild<QLineEdit *>()) {
        edit->setMinimumWidth(minW);
    }
    if (dlg.exec() == QDialog::Rejected)
        return;

    QString id = dlg.textValue();
    if (id.isEmpty())
        return;

    auto &tallies = model_->options()->UserTally;
    user_tally::parameters newTally;
    newTally.id = id.toStdString();
    tallies.push_back(newTally);

    cbTallyID->addItem(id);
    int newIndex = cbTallyID->count() - 1;
    cbTallyID->setItemData(newIndex, QVariant::fromValue(QVector<int>()), Qt::UserRole);
    cbTallyID->setCurrentIndex(newIndex);

    btDelTally->setEnabled(true);
    btEdtTally->setEnabled(true);

    notifyChanged();
}

void UserTallyView::removeTally()
{
    QString id = cbTallyID->currentText();
    int i = cbTallyID->currentIndex();
    if (id.isEmpty() || i < 0)
        return;

    QMessageBox::StandardButton ret = QMessageBox::warning(
            this, "Remove UserTally", QString("%1 is being removed.\nClick OK to proceed.").arg(id),
            QMessageBox::Ok | QMessageBox::Cancel);
    if (ret != QMessageBox::Ok)
        return;

    auto &tallies = model_->options()->UserTally;
    tallies.erase(tallies.begin() + i);

    cbTallyID->removeItem(i);

    bool empty = cbTallyID->count() == 0;
    btDelTally->setEnabled(!empty);
    btEdtTally->setEnabled(!empty);

    notifyChanged();
}

void UserTallyView::editTallyName()
{
    if (cbTallyID->count() == 0)
        return;

    int i = cbTallyID->currentIndex();
    bool ok;
    QString id = QInputDialog::getText(this, tr("Edit UserTally id"), tr("Enter the new id"),
                                       QLineEdit::Normal, cbTallyID->currentText(), &ok);
    if (ok && !id.isEmpty()) {
        cbTallyID->setItemText(i, id);
        model_->options()->UserTally[i].id = id.toStdString();
        notifyChanged();
    }
}

void UserTallyView::updateSelectedTally()
{
    int i = cbTallyID->currentIndex();
    btDelTally->setEnabled(i >= 0);
    btEdtTally->setEnabled(i >= 0);

    if (i < 0) {
        edtDescription->blockSignals(true);
        edtDescription->clear();
        edtDescription->blockSignals(false);

        cbEvent->blockSignals(true);
        cbEvent->setCurrentIndex(-1);
        cbEvent->blockSignals(false);

        // Note: QVectorEdit::setText()/clear() must NOT be wrapped in
        // blockSignals() -- QNumberEdit self-connects its own textChanged
        // signal to handleTextChanged() (qnumberedit.cpp) to drive its
        // "invalid" red-border state, and blockSignals() would suppress
        // that internal connection too, leaving stale invalid styling.
        // setText()/clear() don't emit editingFinished() on their own, so
        // there is nothing to guard against here anyway.

        QVector<float> defaultValue = { 0, 0, 0 };
        edtOrigin->setValue(QVariant::fromValue(defaultValue));
        defaultValue = { 0, 0, 1 };
        edtZAxis->setValue(QVariant::fromValue(defaultValue));
        defaultValue = { 1, 0, 1 };
        edtXZVector->setValue(QVariant::fromValue(defaultValue));

        // edtOrigin->clear();
        // edtZAxis->clear();
        // edtXZVector->clear();

        binsModel->setTallyIdx(-1);
        btAddBin->setEnabled(false);
        btRemoveBin->setEnabled(false);
        return;
    }

    const user_tally::parameters &t = model_->options()->UserTally[i];

    edtDescription->blockSignals(true);
    edtDescription->setPlainText(QString::fromStdString(t.description));
    edtDescription->blockSignals(false);

    cbEvent->blockSignals(true);
    cbEvent->setCurrentText(eventToName(t.event));
    cbEvent->blockSignals(false);

    edtOrigin->setText(qstring_serialize<vector3>::toString(t.coordinate_system.origin));
    edtZAxis->setText(qstring_serialize<vector3>::toString(t.coordinate_system.zaxis));
    edtXZVector->setText(qstring_serialize<vector3>::toString(t.coordinate_system.xzvector));

    QVector<int> savedOrder = cbTallyID->itemData(i, Qt::UserRole).value<QVector<int>>();
    binsModel->setTallyIdx(i, savedOrder);
    btAddBin->setEnabled(!binsModel->isFull());
    btRemoveBin->setEnabled(false);
}

void UserTallyView::setDescription()
{
    int i = cbTallyID->currentIndex();
    if (i < 0)
        return;
    model_->options()->UserTally[i].description = edtDescription->toPlainText().toStdString();
    notifyChanged();
}

void UserTallyView::setEvent(int idx)
{
    int i = cbTallyID->currentIndex();
    if (i < 0 || idx < 0)
        return;
    model_->options()->UserTally[i].event = eventFromName(cbEvent->itemText(idx));
    notifyChanged();
}

void UserTallyView::setOrigin()
{
    int i = cbTallyID->currentIndex();
    if (i < 0)
        return;
    qstring_serialize<vector3>::fromString(edtOrigin->text(),
                                           model_->options()->UserTally[i].coordinate_system.origin);
    notifyChanged();
}

void UserTallyView::setZAxis()
{
    int i = cbTallyID->currentIndex();
    if (i < 0)
        return;
    qstring_serialize<vector3>::fromString(edtZAxis->text(),
                                           model_->options()->UserTally[i].coordinate_system.zaxis);
    notifyChanged();
}

void UserTallyView::setXZVector()
{
    int i = cbTallyID->currentIndex();
    if (i < 0)
        return;
    qstring_serialize<vector3>::fromString(
            edtXZVector->text(), model_->options()->UserTally[i].coordinate_system.xzvector);
    notifyChanged();
}

void UserTallyView::addBin()
{
    int r = binsModel->rowCount();
    binsModel->insertRows(r, 1, QModelIndex());
}

void UserTallyView::removeBin()
{
    int n = binsModel->rowCount();
    for (int i = 0; i < n; ++i) {
        if (binsSelectionModel->isRowSelected(i)) {
            binsModel->removeRows(i, 1);
            break;
        }
    }
}

void UserTallyView::onBinSelectionChanged(const QItemSelection & /* selected */,
                                          const QItemSelection & /* deselected */)
{
    int n = binsModel->rowCount();
    for (int i = 0; i < n; ++i) {
        if (binsSelectionModel->isRowSelected(i)) {
            btRemoveBin->setEnabled(true);
            return;
        }
    }
    btRemoveBin->setEnabled(false);
}

void UserTallyView::onBinsChanged()
{
    int i = cbTallyID->currentIndex();
    if (i >= 0)
        cbTallyID->setItemData(i, QVariant::fromValue(binsModel->rowVar()), Qt::UserRole);
    btAddBin->setEnabled(!binsModel->isFull());
}

void UserTallyView::notifyChanged()
{
    model_->dataChanged(utallyIndex_, utallyIndex_);
}
