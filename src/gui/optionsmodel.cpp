#include "optionsmodel.h"

#include "qjsonpath/qjsonpath.h"

#include <QComboBox>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
//#include <QDoubleValidator>

#include "floatlineedit.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonArray>
#include <QFile>

#include <QDebug>


OptionsItem::OptionsItem(OptionsItem *parent)
    : m_parentItem(parent)
{
    if (parent) P_ = parent->P_;
    else P_ = std::make_shared<Private>();
}
OptionsItem::OptionsItem(const QString& key,
                         OptionsItem *parent)
    : m_parentItem(parent), key_(key), name_(key), P_(parent->P_)
{
    jpath_ = m_parentItem->isRoot() ? key :
                 QString("%1/%2")
                     .arg(m_parentItem->jpath_)
                     .arg(key);
}
OptionsItem::OptionsItem(const QString& key,
                         const QString& name,
                         OptionsItem *parent)
    : m_parentItem(parent), key_(key), name_(name), P_(parent->P_)
{
    jpath_ = QString("%1/%2")
                 .arg(m_parentItem->jpath_)
                 .arg(key);
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
    if (P_) {
        return QJsonPath::get(P_->jdoc, jpath_).toVariant();
    } else return QVariant();
}
bool OptionsItem::setValue(const QVariant& v)
{
    if (!P_) return false;
    if (value() != v) {
        QJsonValue j = v.toJsonValue();
        QJsonPath::set(P_->jdoc, jpath_, j);
        return true;
    }
    return false;
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
    if (P_) {
        QString s = QJsonPath::get(P_->jdoc, jpath_).toString();
        int i = enumValues_.indexOf(s);
        return i;
    } else return QVariant();
}
bool EnumOptionsItem::setValue(const QVariant& v)
{   
    if (!P_) return false;
    if (value() != v) {
        QJsonValue j = enumValues_.at(v.toInt());
        QJsonPath::set(P_->jdoc, jpath_, j);
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
    if (P_) {
        return QJsonPath::get(P_->jdoc, jpath_).toDouble();
    } else return QVariant();
}
QVariant FloatOptionsItem::displayValue() const
{
    return QString::number(value().toDouble(),'g',digits_);
}
bool FloatOptionsItem::setValue(const QVariant& v)
{
    if (!P_) return false;
    double d = v.toDouble();
    double d0 = value().toDouble();
    if (d0 != d) {
        QJsonPath::set(P_->jdoc, jpath_, d);
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
    if (P_) {
        return QJsonPath::get(P_->jdoc, jpath_).toInt();
    } else return QVariant();
}
bool IntOptionsItem::setValue(const QVariant& v)
{
    if (!P_) return false;
    int d = v.toDouble();
    if (value().toInt() != d) {
        QJsonPath::set(P_->jdoc, jpath_, d);
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
    if (P_) {
        bool ret = false;
        QJsonValue v = QJsonPath::get(P_->jdoc, jpath_);
        if (v.isBool()) ret = v.toBool();
        else if (v.isDouble()) ret = v.toInt();
        return ret;
    } else return QVariant();
}
bool BoolOptionsItem::setValue(const QVariant& v)
{
    if (!P_) return false;

    bool b = v.toBool();

    QJsonValue v0 = QJsonPath::get(P_->jdoc, jpath_);
    if (v0.isBool()) {
        if (b != v0.toBool()) {
            QJsonPath::set(P_->jdoc, jpath_, b);
            return true;
        }
    } else if (v0.isDouble()) {
        bool b0 = v0.toInt();
        if (b != b0) {
            QJsonPath::set(P_->jdoc, jpath_, int(b));
            return true;
        }
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
    if (P_) {
        return QJsonPath::get(P_->jdoc, jpath_).toString();
    } else return QVariant();
}
bool StringOptionsItem::setValue(const QVariant& v)
{
    if (!P_) return false;
    QString s = v.toString();
    if (s != value().toString()) {
        QJsonPath::set(P_->jdoc, jpath_, v.toString());
        return true;
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
    if (P_) {
        QJsonValue jv = QJsonPath::get(P_->jdoc, jpath_);
        return QVariant::fromValue(Vector3D::fromJsonValue(jv));
    } else return QVariant();
}
QVariant Vector3dOptionsItem::displayValue() const
{
    return value().value<Vector3D>().toString();
}
bool Vector3dOptionsItem::setValue(const QVariant& v)
{
    if (!P_) return false;
    Vector3D v3d = v.value<Vector3D>();
    if (v3d != value().value<Vector3D>()) {
        QJsonPath::set(P_->jdoc, jpath_,
                       v3d.toJsonValue());
        return true;
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
    ((Vector3dLineEdit*)editor)->setValue(v.value<Vector3D>());
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
    if (P_) {
        QJsonValue jv = QJsonPath::get(P_->jdoc, jpath_);
        return QVariant::fromValue(IntVector3D::fromJsonValue(jv));
    } else return QVariant();
}
QVariant IVector3dOptionsItem::displayValue() const
{
    return value().value<IntVector3D>().toString();
}
bool IVector3dOptionsItem::setValue(const QVariant& v)
{
    if (!P_) return false;
    IntVector3D i3d = v.value<IntVector3D>();
    if (i3d != value().value<IntVector3D>()) {
        QJsonPath::set(P_->jdoc, jpath_,
                       i3d.toJsonValue());
        return true;
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
    ((IntVector3dLineEdit*)editor)->setValue(v.value<IntVector3D>());
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

void OptionsModel::setJsonOptions(const QJsonDocument& jdoc)
{
    //beginResetModel();
    rootItem->P_->jdoc = jdoc;
    //endResetModel();
}
const QJsonDocument& OptionsModel::jsonOptions() const
{
    return rootItem->P_->jdoc;
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
