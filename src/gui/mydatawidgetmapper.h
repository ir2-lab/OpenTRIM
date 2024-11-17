#ifndef MYDATAWIDGETMAPPER_H
#define MYDATAWIDGETMAPPER_H

#include <QObject>
#include <QPointer>
#include <QPersistentModelIndex>

class OptionsItemDelegate;
class OptionsModel;
class QModelIndex;
class QDataWidgetMapperPrivate;

class MyDataWidgetMapper: public QObject
{
    Q_OBJECT

public:
    explicit MyDataWidgetMapper(OptionsModel* m,
                                QObject *parent = nullptr);

    void addMapping(QWidget *widget, const QModelIndex& idx, const char* signal = nullptr);
    void clearMapping();

    OptionsModel* model() const { return model_; }

    QWidget* findWidget(const QString& key) const;

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

    struct mapItem {
        QPointer<QWidget> widget;
        QPersistentModelIndex idx;
        const char* signal;
    };
    std::vector<mapItem> widgetMap;
    void removeMapping(QWidget *widget);
    int findWidget(QWidget *widget) const;
    void populate(mapItem &m);
    bool commit(const mapItem &m);
    QModelIndex indexAt(int i);

    OptionsModel *model_;
    OptionsItemDelegate *delegate_;
    Q_DISABLE_COPY(MyDataWidgetMapper)
};

#endif // MYDATAWIDGETMAPPER_H
