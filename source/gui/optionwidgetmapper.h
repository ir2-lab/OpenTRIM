#ifndef OPTIONWIDGETMAPPER_H
#define OPTIONWIDGETMAPPER_H

#include <QObject>
#include <QPointer>
#include <QPersistentModelIndex>
#include <QWidgetList>

class OptionsItemDelegate;
class OptionsModel;
class QModelIndex;
class QDataWidgetMapperPrivate;

class OptionWidgetMapper : public QObject
{
    Q_OBJECT

public:
    explicit OptionWidgetMapper(OptionsModel *m, QObject *parent = nullptr);

    void addMapping(QWidget *w, const QModelIndex &idx, bool isEditor,
                    const char *signal = nullptr);

    void removeMapping(const QString &key);

    OptionsModel *model() const { return model_; }

    QWidget *findWidget(const QString &key) const;
    QModelIndex widgetToIndex(QWidget *w) const;
    QWidgetList widgets() const;

    void setToolTip(const QString &txt = QString());

public slots:
    void revert();
    bool submit();
    void setEnabled(bool b);

private slots:
    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight,
                     const QVector<int> &);
    void commitData(QWidget *);
    void commitData_();
    void closeEditor(QWidget *, int);

private:
    struct mapItem
    {
        QPointer<QWidget> widget;
        QPersistentModelIndex idx;
        const char *signal;
    };
    std::vector<mapItem> widgetMap;
    void removeMapping(QWidget *widget);
    int findWidget(QWidget *widget) const;
    void populate(mapItem &m);
    bool commit(const mapItem &m);
    QModelIndex indexAt(int i);

    OptionsModel *model_;
    OptionsItemDelegate *delegate_;
    Q_DISABLE_COPY(OptionWidgetMapper)
};

#endif // MYDATAWIDGETMAPPER_H
