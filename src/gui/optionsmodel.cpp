#include "optionsmodel.h"

#include <QComboBox>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>

#include "floatlineedit.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

#include <QDebug>


OptionsItem::OptionsItem(OptionsItem *parent)
    : m_parentItem(parent)
{
    if (parent) options_ = parent->options_;
    else options_ = std::make_shared<mcdriver::options>();
}
OptionsItem::OptionsItem(const QString& key,
                         OptionsItem *parent)
    : OptionsItem(key, key, parent)
{}
OptionsItem::OptionsItem(const QString& key,
                         const QString& name,
                         OptionsItem *parent)
    : m_parentItem(parent), key_(key), name_(name),
    options_(parent->options_)
{
    if (!m_parentItem->isRoot())
        jpath_ = m_parentItem->jpath_;
    jpath_ += std::string("/");
    jpath_ += key.toStdString();
}
OptionsItem::~OptionsItem()
{
    qDeleteAll(m_childItems);
}
OptionsItem *OptionsItem::child(int number)
{
    return (number >= 0 && number < childCount())
               ? m_childItems.at(number) : nullptr;
}
int OptionsItem::childCount() const
{
    return int(m_childItems.size());
}
int OptionsItem::row() const
{
    if (!m_parentItem)
        return 0;
    const auto it = std::find_if(m_parentItem->m_childItems.cbegin(), m_parentItem->m_childItems.cend(),
                                 [this](const OptionsItem* treeItem) {
                                     return treeItem == this;
                                 });

    if (it != m_parentItem->m_childItems.cend())
        return std::distance(m_parentItem->m_childItems.cbegin(), it);

    Q_ASSERT(false); // should not happen
    return -1;
}
QVariant OptionsItem::value() const
{
//    QString s;
//    if (get_(s)) {
//        return s;
//    } else
    return QVariant();
}
bool OptionsItem::setValue(const QVariant& v)
{
//    if (value() != v) {
//        set_(v.toString());
//        return true;
//    }
//    return false;
    return true;
}
void OptionsItem::appendChild(OptionsItem *item)
{
    m_childItems.push_back(item);
}
void OptionsItem::prepareWidget(QWidget* w) const
{
    w->setToolTip(toolTip_);
    w->setWhatsThis(whatsThis_);
    w->setObjectName(key_);
}
OptionsItem::type_t OptionsItem::toType(const QString &typeName)
{
    if (typeName == "enum") return tEnum;
    else if (typeName == "float") return tFloat;
    else if (typeName == "int") return tInt;
    else if (typeName == "bool") return tBool;
    else if (typeName == "string") return tString;
    else if (typeName == "vector3d") return tVector3D;
    else if (typeName == "ivector3d") return tIntVector3D;
    else return tInvalid;
}
bool OptionsItem::get_(QString& qs) const
{
    std::string s;
    std::ostringstream os;
    if (!options_->get(jpath_,s,&os)) return false;
    qs = QString::fromStdString(s);
    return true;
}
bool OptionsItem::set_(const QString& qs)
{
    std::string s = qs.toStdString();
    std::ostringstream os;
    bool ret = options_->set(jpath_,s,&os);
    if (!ret) {
        qDebug() << QString::fromStdString(os.str());
    }
    return ret;
}
EnumOptionsItem::EnumOptionsItem(const QStringList& values,
                const QStringList& labels,
                const QString& key,
                const QString& name,
                OptionsItem *parent) :
    OptionsItem(key,name,parent),
    enumValues_(values),
    enumValueLabels_(labels)
{}
QVariant EnumOptionsItem::value() const
{
    QString s;
    if (get_(s)) {
        if (s.size()>2) { // This has double ", e.g. ""Off""
            s.chop(1);
            s.remove(0,1);
        } else return QVariant();
        return enumValues_.indexOf(s); // i=-1 means not found = invalid
    }
    return QVariant();
}
bool EnumOptionsItem::setValue(const QVariant& v)
{   
    if (value() != v) {
        QString s = QString("\"%1\"").arg(enumValues_.at(v.toInt()));
        set_(s);
        return true;
    }
    return false;
}
QWidget* EnumOptionsItem::createEditor(QWidget* parent) const
{
    QComboBox *w = new QComboBox(parent);
    w->addItems(enumValueLabels_);
    prepareWidget(w);
    return w;
}
void EnumOptionsItem::setEditorData(QWidget *editor, const QVariant &v) const
{
    ((QComboBox*)editor)->setCurrentIndex(v.toInt());
}
QVariant EnumOptionsItem::getEditorData(QWidget *editor)
{
    return ((QComboBox*)editor)->currentIndex();
}
FloatOptionsItem::FloatOptionsItem(double fmin, double fmax, int digits,
                                 const QString& key,
                                 const QString& name,
                                 OptionsItem *parent) :
    OptionsItem(key,name,parent),
    fmin_(fmin), fmax_(fmax),
    digits_(digits)
{}
QVariant FloatOptionsItem::value() const
{
    QString s;
    if (get_(s)) {
        return s.toFloat();
    } else return QVariant();
}
QVariant FloatOptionsItem::displayValue() const
{
    return QString::number(value().toFloat(),'g',digits_);
}
bool FloatOptionsItem::setValue(const QVariant& v)
{
    float d = v.toFloat();
    float d0 = value().toFloat();
    if (d0 != d) {
        QString s = QString::number(d);
        set_(s);
        return true;
    }
    return false;
}
QWidget* FloatOptionsItem::createEditor(QWidget* parent) const
{
    QWidget* w = new FloatLineEdit(fmin_,fmax_,digits_,parent);
    prepareWidget(w);
    return w;
}
void FloatOptionsItem::setEditorData(QWidget *editor, const QVariant &v) const
{
    ((FloatLineEdit*)editor)->setValue(v.toDouble());
}
QVariant FloatOptionsItem::getEditorData(QWidget *editor)
{
    return ((FloatLineEdit*)editor)->value();
}
IntOptionsItem::IntOptionsItem(int imin, int imax,
                                   const QString& key,
                                   const QString& name,
                                   OptionsItem *parent) :
    OptionsItem(key,name,parent),
    imin_(imin), imax_(imax)
{}
QVariant IntOptionsItem::value() const
{
    QString s;
    if (get_(s)) {
        return s.toInt();
    } else return QVariant();
}
bool IntOptionsItem::setValue(const QVariant& v)
{
    int d = v.toInt();
    if (value().toInt() != d) {
        QString s = QString::number(d);
        set_(s);
        return true;
    }
    return false;
}
QWidget* IntOptionsItem::createEditor(QWidget* parent) const
{
    QSpinBox* sb = new QSpinBox(parent);
    sb->setMinimum(imin_);
    sb->setMaximum(imax_);
    prepareWidget(sb);
    return sb;
}
void IntOptionsItem::setEditorData(QWidget *editor, const QVariant &v) const
{
    ((QSpinBox*)editor)->setValue(v.toInt());
}
QVariant IntOptionsItem::getEditorData(QWidget *editor)
{
    return ((QSpinBox*)editor)->value();
}
BoolOptionsItem::BoolOptionsItem(const QString& key,
                               const QString& name,
                               OptionsItem *parent) :
    OptionsItem(key,name,parent)
{}
QVariant BoolOptionsItem::value() const
{
    QString s;
    if (get_(s)) {
        if (s == "true") return true;
        if (s == "false") return false;
        bool ok = false;
        int i = s.toInt(&ok);
        if (ok) {
            bool ret = i;
            return ret;
        }
        return QVariant();
    } else return QVariant();
}
bool BoolOptionsItem::setValue(const QVariant& v)
{

    QString s0;
    if (get_(s0)) {
        bool b = v.toBool();
        if (s0=="true") {
            bool b0 = true;
            if (b!=b0) set_(b ? "true" : "false");
            return true;
        }
        if (s0=="false") {
            bool b0 = false;
            if (b!=b0) set_(b ? "true" : "false");
            return true;
        }
        bool ok = false;
        int i = s0.toInt(&ok);
        if (ok) {
            bool b0 = i;
            if (b!=b0) set_(QString::number(i));
            return true;
        }
        return false;
    }
    return false;
}
QWidget* BoolOptionsItem::createEditor(QWidget* parent) const
{
    QComboBox *w = new QComboBox(parent);
    w->addItem("false");
    w->addItem("true");
    prepareWidget(w);
    return w;
}
void BoolOptionsItem::setEditorData(QWidget *editor, const QVariant &v) const
{
    ((QComboBox*)editor)->setCurrentIndex(v.toInt());
}
QVariant BoolOptionsItem::getEditorData(QWidget *editor)
{
    return ((QComboBox*)editor)->currentIndex();
}
StringOptionsItem::StringOptionsItem(const QString& key,
                                 const QString& name,
                                 OptionsItem *parent) :
    OptionsItem(key,name,parent)
{}
QVariant StringOptionsItem::value() const
{
    QString s;
    if (get_(s)) {
        if (s.size()>2) {
            s.chop(1);
            s.remove(0,1);
            return s;
        }
        return QString("");
    } else return QVariant();
}
bool StringOptionsItem::setValue(const QVariant& v)
{
    QString s = v.toString();
    if (s != value().toString()) {
        return set_(QString("\"%1\"").arg(s));
    }
    return false;
}
QWidget* StringOptionsItem::createEditor(QWidget* parent) const
{
    QWidget* w = new QLineEdit(parent);
    prepareWidget(w);
    return w;
}
void StringOptionsItem::setEditorData(QWidget *editor, const QVariant &v) const
{
    ((QLineEdit*)editor)->setText(v.toString());
}
QVariant StringOptionsItem::getEditorData(QWidget *editor)
{
    return ((QLineEdit*)editor)->text();
}
Vector3dOptionsItem::Vector3dOptionsItem(double fmin, double fmax, int digits,
                                   const QString& key,
                                   const QString& name,
                                   OptionsItem *parent) :
    FloatOptionsItem(fmin,fmax,digits,key,name,parent)
{}
QVariant Vector3dOptionsItem::value() const
{
    QString s;
    if (get_(s)) {
        return QVariant::fromValue(qstring_serialize<vector3>::fromString(s));
    } else return QVariant();
}
QVariant Vector3dOptionsItem::displayValue() const
{
    return qstring_serialize<vector3>::toString(value().value<vector3>());
}
bool Vector3dOptionsItem::setValue(const QVariant& v)
{
    vector3 v3d = v.value<vector3>();
    if (v3d != value().value<vector3>()) {
        return set_(qstring_serialize<vector3>::toString(v3d));
    }
    return false;
}
QWidget* Vector3dOptionsItem::createEditor(QWidget* parent) const
{
    QWidget* w = new Vector3dLineEdit(fmin_,fmax_,digits_,parent);
    prepareWidget(w);
    return w;
}
void Vector3dOptionsItem::setEditorData(QWidget *editor, const QVariant &v) const
{
    ((Vector3dLineEdit*)editor)->setValue(v.value<vector3>());
}
QVariant Vector3dOptionsItem::getEditorData(QWidget *editor)
{
    return QVariant::fromValue(((Vector3dLineEdit*)editor)->value());
}
IVector3dOptionsItem::IVector3dOptionsItem(int fmin, int fmax,
                                         const QString& key,
                                         const QString& name,
                                         OptionsItem *parent) :
    IntOptionsItem(fmin,fmax,key,name,parent)
{}
QVariant IVector3dOptionsItem::value() const
{
    QString s;
    if (get_(s)) {
        return QVariant::fromValue(qstring_serialize<ivector3>::fromString(s));
    } else return QVariant();
}
QVariant IVector3dOptionsItem::displayValue() const
{
    return qstring_serialize<ivector3>::toString(value().value<ivector3>());
}
bool IVector3dOptionsItem::setValue(const QVariant& v)
{
    ivector3 i3d = v.value<ivector3>();
    if (i3d != value().value<ivector3>()) {
        return set_(qstring_serialize<ivector3>::toString(i3d));
    }
    return false;
}
QWidget* IVector3dOptionsItem::createEditor(QWidget* parent) const
{
    QWidget* w = new IntVector3dLineEdit(imin_,imax_,parent);
    prepareWidget(w);
    return w;
}
void IVector3dOptionsItem::setEditorData(QWidget *editor, const QVariant &v) const
{
    ((IntVector3dLineEdit*)editor)->setValue(v.value<ivector3>());
}
QVariant IVector3dOptionsItem::getEditorData(QWidget *editor)
{
    return QVariant::fromValue(((IntVector3dLineEdit*)editor)->value());
}
/*********************************************************/
OptionsItemDelegate::OptionsItemDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}
QWidget *OptionsItemDelegate::createEditor(QWidget *parent,
                                           const QStyleOptionViewItem &/* option */,
                                           const QModelIndex &index) const
{
    OptionsItem* i = static_cast<OptionsItem*>(index.internalPointer());
    return i->createEditor(parent);
}
void OptionsItemDelegate::setEditorData(QWidget *editor,
                                                const QModelIndex &index) const
{
    if (!index.isValid()) return;
    OptionsItem* i = static_cast<OptionsItem*>(index.internalPointer());
    return i->setEditorData(editor, i->value());
}
void OptionsItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                               const QModelIndex &index) const
{
    if (!index.isValid()) return;

    OptionsItem* i = static_cast<OptionsItem*>(index.internalPointer());
    model->setData(index, i->getEditorData(editor), Qt::EditRole);
}
void OptionsItemDelegate::updateEditorGeometry(QWidget *editor,
                                                       const QStyleOptionViewItem &option,
                                                       const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}
QSize OptionsItemDelegate::sizeHint(const QStyleOptionViewItem &option,
                                         const QModelIndex &index) const
{
    return QStyledItemDelegate::sizeHint(option, index) + QSize(3, 4);
}
/*********************************************************/
OptionsModel::OptionsModel(QObject *parent)
    : QAbstractItemModel{parent}, rootItem(new OptionsItem)
{
    rootItem = new OptionsItem();

    QFile loadFile(QStringLiteral(":/option_def/options.json"));
    loadFile.open(QIODevice::ReadOnly);
    QByteArray ba = loadFile.readAll();
    QJsonParseError err;
    QJsonDocument opt_spec(QJsonDocument::fromJson(ba,&err));
    assert(err.error == QJsonParseError::NoError);

    QStringList categories;
    categories << "Simulation"
               << "Transport"
               << "IonBeam"
               << "Output"
               << "Driver"
               << "Target";

    foreach(const QString& category, categories) {
        OptionsItem* categoryItem = new OptionsItem(category,rootItem);
        QJsonArray arr = opt_spec[category].toArray();
        for (int i = 0; i < arr.size(); ++i) {
            QJsonObject obj = arr[i].toObject();
            OptionsItem* item;
            switch(OptionsItem::toType(obj["type"].toString())) {
            case OptionsItem::tEnum:
                item = new EnumOptionsItem(
                           obj["values"].toVariant().toStringList(),
                           obj["valueLabels"].toVariant().toStringList(),
                           obj["name"].toString(),
                           obj["label"].toString(),
                           categoryItem
                    );
                break;
            case OptionsItem::tFloat:
                item = new FloatOptionsItem(
                    obj["min"].toDouble(),
                    obj["max"].toDouble(),
                    obj["digits"].toInt(),
                    obj["name"].toString(),
                    obj["label"].toString(),
                    categoryItem
                    );
                break;
            case OptionsItem::tVector3D:
                item = new Vector3dOptionsItem(
                    obj["min"].toDouble(),
                    obj["max"].toDouble(),
                    obj["digits"].toInt(),
                    obj["name"].toString(),
                    obj["label"].toString(),
                    categoryItem
                    );
                break;
            case OptionsItem::tIntVector3D:
                item = new IVector3dOptionsItem(
                    obj["min"].toInt(),
                    obj["max"].toInt(),
                    obj["name"].toString(),
                    obj["label"].toString(),
                    categoryItem
                    );
                break;
            case OptionsItem::tInt:
                item = new IntOptionsItem(
                    obj["min"].toInt(),
                    obj["max"].toInt(),
                    obj["name"].toString(),
                    obj["label"].toString(),
                    categoryItem
                    );
                break;
            case OptionsItem::tBool:
                item = new BoolOptionsItem(
                    obj["name"].toString(),
                    obj["label"].toString(),
                    categoryItem
                    );
                break;
            case OptionsItem::tString:
                item = new StringOptionsItem(
                    obj["name"].toString(),
                    obj["label"].toString(),
                    categoryItem
                    );
                break;
            default:
                assert(0);
                break;
            }


            QString txt = obj["toolTip"].toString();
            item->toolTip_ = txt;
            if (obj.contains("whatsThis")) {
                txt += "\n\n";
                if (obj["whatsThis"].isArray())
                    txt += obj["whatsThis"].toVariant().toStringList().join('\n');
                else
                    txt += obj["whatsThis"].toString();
            }
            item->whatsThis_ = txt;

            categoryItem->appendChild(item);
        }
        rootItem->appendChild(categoryItem);
    }

    OptionsItem* target = rootItem->child(rootItem->childCount()-1);
    target->appendChild(new OptionsItem("materials",target));
    target->appendChild(new OptionsItem("regions",target));

    // rootItem->appendChild(target);
}
OptionsModel::~OptionsModel()
{
    delete rootItem;
}

void OptionsModel::setOptions(const mcdriver::options& opt)
{
    //beginResetModel();
    *(rootItem->options_) = opt;
    //endResetModel();
}
const mcdriver::options* OptionsModel::options() const
{
    return rootItem->options_.get();
}

mcdriver::options *OptionsModel::options()
{
    return rootItem->options_.get();
}

QVariant OptionsModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid())
        return {};

    OptionsItem *item = static_cast<OptionsItem *>(index.internalPointer());

    int col = index.column();
    if (role == Qt::DisplayRole) {
        if (col == 0)
            return item->name();

        if (col == 1)
            return item->displayValue();
    } else if (Qt::EditRole == role) {
        if (col == 1)
            return item->value();
    }

    return {};
}
bool OptionsModel::setData(const QModelIndex &index, const QVariant &value,
                         int role) {
    // int col = index.column();
    if (Qt::EditRole == role) {
        //if (col == 1) {
        OptionsItem *item =
            static_cast<OptionsItem *>(index.internalPointer());
        if (item->setValue(value))
            emit dataChanged(index, index, {Qt::EditRole});
            return true;
        //}
    }
    return false;
}
QVariant OptionsModel::headerData(int section, Qt::Orientation orientation,
                                int role) const {
    if (role != Qt::DisplayRole)
        return {};

    if (orientation == Qt::Horizontal && (section==0 || section==1)) {
        const char* hdr_lbl[] = { "Property", "Value" };
        return hdr_lbl[section];
    } else
        return {};
}
QModelIndex OptionsModel::index(int row, int column,
                              const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return {};

    OptionsItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<OptionsItem *>(parent.internalPointer());

    OptionsItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return {};
}

QModelIndex OptionsModel::index(const QString& key, int column,
                  const QModelIndex &parent) const
{
    OptionsItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<OptionsItem *>(parent.internalPointer());

    OptionsItem *childItem = nullptr;
    int row = 0;
    for(; row<parentItem->m_childItems.size(); ++row) {
        auto i = parentItem->m_childItems[row];
        if (i->key() == key) {
            childItem = i;
            break;
        }
    }

    if (childItem)
        return createIndex(row, column, childItem);
    else
        return {};
}

QModelIndex OptionsModel::parent(const QModelIndex &index) const {
    if (!index.isValid())
        return {};

    OptionsItem *childItem =
        static_cast<OptionsItem *>(index.internalPointer());
    OptionsItem *parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int OptionsModel::rowCount(const QModelIndex &parent) const {
    OptionsItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<OptionsItem *>(parent.internalPointer());

    return parentItem->childCount();
}

int OptionsModel::columnCount(const QModelIndex &parent) const {
    Q_UNUSED(parent)
    return 2;
}

Qt::ItemFlags OptionsModel::flags(const QModelIndex &index) const {
    if (index.column() == 1)
        return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
    else
        return QAbstractItemModel::flags(index);
}

OptionsItem* OptionsModel::getItem(const QModelIndex &index) const
{
    if (!index.isValid())
        return nullptr;

    return static_cast<OptionsItem *>(index.internalPointer());
}
