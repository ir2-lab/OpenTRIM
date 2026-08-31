#ifndef OPTIONSMODEL_H
#define OPTIONSMODEL_H

#include <memory>
#include <QAbstractItemModel>
#include "mcdriver.h"
#include "validatingitemdelegate.h"

class OptionsItem
{
public:
    virtual ~OptionsItem();
    OptionsItem *child(int number);
    int childCount() const;
    void appendChild(OptionsItem *item);
    OptionsItem *parent() const { return m_parentItem; }
    int row() const;

    QString key() const { return key_; }
    QString name() const { return name_; }
    QString path() const { return QString::fromStdString(jpath_); }
    mcconfig::option_type_t type() const { return type_; }
    QString toolTip() const { return toolTip_; }
    QString whatsThis() const { return whatsThis_; }
    QString notes() const { return notes_; }
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
    friend class OptionsModel;

    template <class J>
    static OptionsItem *jsonHelper(const J &j, OptionsItem *parentItem);

protected:
    OptionsItem(const QString &key, mcconfig::option_type_t t, OptionsItem *parent);
    OptionsItem(const QString &key, const QString &name, mcconfig::option_type_t t,
                OptionsItem *parent);
    void prepareWidget(QWidget *w) const;
    std::vector<OptionsItem *> m_childItems;
    OptionsItem *m_parentItem;
    QString key_, name_;
    mcconfig::option_type_t type_{ mcconfig::tStruct };
    std::string jpath_;
    QString toolTip_, whatsThis_, notes_;

    bool get_(QString &qs) const;
    bool set_(const QString &qs);

    std::shared_ptr<mcconfig> options_;
};

class EnumOptionsItem : public OptionsItem
{
public:
    EnumOptionsItem(const QStringList &values, const QStringList &labels, const QStringList &desc,
                    const QString &key, const QString &name, OptionsItem *parent);
    const QStringList &values() { return enumValues_; }
    const QStringList &valueLabels() { return enumValueLabels_; }
    const QStringList &valueValueDescriptions() { return enumValueDescriptions_; }
    virtual QWidget *createEditor(QWidget *parent) const override;
    virtual void setEditorData(QWidget *editor, const QVariant &v) const override;
    virtual QVariant getEditorData(QWidget *editor) override;
    virtual const char *editorSignal() const override { return "currentIndexChanged(int)"; }

protected:
    QStringList enumValues_, enumValueLabels_, enumValueDescriptions_;
};

class FloatOptionsItem : public OptionsItem
{
public:

    double min() const { return fmin_; }
    double max() const { return fmax_; }
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
    int min() const { return imin_; }
    int max() const { return imax_; }

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
    int size() const { return sz_; }
};

class IVectorOptionsItem : public IntOptionsItem
{
    int sz_;
    typedef std::vector<int> vector_t;

public:
    IVectorOptionsItem(int size, int imin, int imax, const QString &key, const QString &name,
                       OptionsItem *parent);
    virtual QWidget *createEditor(QWidget *parent) const override;
    virtual void setEditorData(QWidget *editor, const QVariant &v) const override;
    virtual QVariant getEditorData(QWidget *editor) override;
    virtual const char *editorSignal() const override { return "editingFinished()"; }
    int size() const { return sz_; }
};

class OptionsItemDelegate : public ValidatingItemDelegate
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

protected:
    // Editors created via OptionWidgetMapper live permanently in a
    // QFormLayout, not inside a QAbstractItemView -- there is no view to
    // "close the editor and move to the next cell", so the base
    // QAbstractItemDelegate::eventFilter()'s Tab/Backtab handling (which
    // swallows the key and emits closeEditor(EditNextItem/EditPreviousItem))
    // has nothing to act on it. Move focus ourselves instead.
    bool eventFilter(QObject *object, QEvent *event) override;
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
    QModelIndex indexFromPath(const QString &path) const;
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
