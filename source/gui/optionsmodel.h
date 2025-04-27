#ifndef OPTIONSMODEL_H
#define OPTIONSMODEL_H

#include <memory>
#include <QAbstractItemModel>
#include <QStyledItemDelegate>
#include "mcdriver.h"

// Q_DECLARE_TYPEINFO(vector3, Q_PRIMITIVE_TYPE);
// Q_DECLARE_METATYPE(vector3)

// Q_DECLARE_TYPEINFO(ivector3, Q_PRIMITIVE_TYPE);
// Q_DECLARE_METATYPE(ivector3)

class OptionsItem
{
    enum type_t { tEnum, tFloat, tInt, tBool, tString, tVector, tIntVector, tStruct, tInvalid };

public:
    virtual ~OptionsItem();
    OptionsItem *child(int number);
    int childCount() const;
    void appendChild(OptionsItem *item);
    OptionsItem *parent() const { return m_parentItem; }
    int row() const;

    QString key() const { return key_; }
    QString name() const { return name_; }
    QString toolTip() const { return toolTip_; }
    QString whatsThis() const { return whatsThis_; }
    QVariant displayValue() const { return value().toString(); }
    QVariant value() const;
    bool setValue(const QVariant &v);

    virtual QWidget *createEditor(QWidget *parent) const { return nullptr; }
    virtual void setEditorData(QWidget *editor, const QVariant &v) const { }
    virtual QVariant getEditorData(QWidget *editor) { return QVariant(); }
    virtual const char *editorSignal() const { return nullptr; }

    bool isRoot() const { return parent() == nullptr; }

    bool direct_set(const char *path, const char *json);

private:
    explicit OptionsItem(OptionsItem *parent = nullptr);
    static type_t toType(const QString &typeName);
    friend class OptionsModel;

    template <class J>
    static OptionsItem *jsonHelper(OptionsItem::type_t type, const J &j, OptionsItem *parentItem);

protected:
    OptionsItem(const QString &key, OptionsItem *parent);
    OptionsItem(const QString &key, const QString &name, OptionsItem *parent);
    void prepareWidget(QWidget *w) const;
    std::vector<OptionsItem *> m_childItems;
    OptionsItem *m_parentItem;
    QString key_, name_;
    std::string jpath_;
    QString toolTip_, whatsThis_;

    bool get_(QString &qs) const;
    bool set_(const QString &qs);

    std::shared_ptr<mcconfig> options_;
};

class EnumOptionsItem : public OptionsItem
{
public:
    EnumOptionsItem(const QStringList &values, const QStringList &labels, const QString &key,
                    const QString &name, OptionsItem *parent);
    const QStringList &values() { return enumValues_; }
    const QStringList &valueLabels() { return enumValueLabels_; }
    virtual QWidget *createEditor(QWidget *parent) const override;
    virtual void setEditorData(QWidget *editor, const QVariant &v) const override;
    virtual QVariant getEditorData(QWidget *editor) override;
    virtual const char *editorSignal() const override { return "currentIndexChanged(int)"; }

protected:
    QStringList enumValues_;
    QStringList enumValueLabels_;
};

class FloatOptionsItem : public OptionsItem
{
public:
    FloatOptionsItem(double fmin, double fmax, int digits, const QString &key, const QString &name,
                     OptionsItem *parent);
    virtual QWidget *createEditor(QWidget *parent) const override;
    virtual void setEditorData(QWidget *editor, const QVariant &v) const override;
    virtual QVariant getEditorData(QWidget *editor) override;
    virtual const char *editorSignal() const override { return "editingFinished()"; }

protected:
    double fmin_, fmax_;
    int digits_;
};

class IntOptionsItem : public OptionsItem
{
public:
    IntOptionsItem(int imin, int imax, const QString &key, const QString &name,
                   OptionsItem *parent);
    // virtual QVariant value() const override;
    // virtual bool setValue(const QVariant &v) override;
    virtual QWidget *createEditor(QWidget *parent) const override;
    virtual void setEditorData(QWidget *editor, const QVariant &v) const override;
    virtual QVariant getEditorData(QWidget *editor) override;
    virtual const char *editorSignal() const override { return "valueChanged(int)"; }

protected:
    int imin_, imax_;
};

class BoolOptionsItem : public OptionsItem
{
public:
    BoolOptionsItem(const QString &key, const QString &name, OptionsItem *parent);
    virtual QWidget *createEditor(QWidget *parent) const override;
    virtual void setEditorData(QWidget *editor, const QVariant &v) const override;
    virtual QVariant getEditorData(QWidget *editor) override;
    virtual const char *editorSignal() const override { return "currentIndexChanged(int)"; }
};

class StringOptionsItem : public OptionsItem
{
public:
    StringOptionsItem(const QString &key, const QString &name, OptionsItem *parent);
    virtual QWidget *createEditor(QWidget *parent) const override;
    virtual void setEditorData(QWidget *editor, const QVariant &v) const override;
    virtual QVariant getEditorData(QWidget *editor) override;
    virtual const char *editorSignal() const override { return "editingFinished()"; }
};

class VectorOptionsItem : public FloatOptionsItem
{
    int sz_;
    typedef std::vector<float> vector_t;

public:
    VectorOptionsItem(int size, double fmin, double fmax, int digits, const QString &key,
                      const QString &name, OptionsItem *parent);
    virtual QWidget *createEditor(QWidget *parent) const override;
    virtual void setEditorData(QWidget *editor, const QVariant &v) const override;
    virtual QVariant getEditorData(QWidget *editor) override;
};

class IVectorOptionsItem : public IntOptionsItem
{
    int sz_;
    typedef std::vector<int> vector_t;

public:
    IVectorOptionsItem(int size, int imin, int imax, const QString &key, const QString &name,
                       OptionsItem *parent);
    // virtual QVariant value() const override;
    // virtual QVariant displayValue() const override;
    // virtual bool setValue(const QVariant &v) override;
    virtual QWidget *createEditor(QWidget *parent) const override;
    virtual void setEditorData(QWidget *editor, const QVariant &v) const override;
    virtual QVariant getEditorData(QWidget *editor) override;
    virtual const char *editorSignal() const override { return "editingFinished()"; }
};

class OptionsItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    OptionsItemDelegate(QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override;

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const override;

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

class OptionsModel : public QAbstractItemModel
{

    Q_OBJECT

public:
    explicit OptionsModel(QObject *parent = nullptr);
    ~OptionsModel();

    void setOptions(const mcconfig &opt);
    const mcconfig *options() const;
    mcconfig *options();

    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    QModelIndex index(int row, int column, const QModelIndex &parent = {}) const override;
    QModelIndex index(const QString &key, int column = 0, const QModelIndex &parent = {}) const;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = {}) const override;
    int columnCount(const QModelIndex &parent = {}) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    OptionsItem *getItem(const QModelIndex &index) const;

private:
    OptionsItem *rootItem;
};

#endif // OPTIONSMODEL_H
