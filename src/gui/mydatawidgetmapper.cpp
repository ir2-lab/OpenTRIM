#include "mydatawidgetmapper.h"

#include "optionsmodel.h"

#include <QMetaObject>
#include <QMetaMethod>

MyDataWidgetMapper::MyDataWidgetMapper(OptionsModel *m,
                                       QObject *parent)
    : QObject{parent}, model_(m),
    delegate_(new OptionsItemDelegate(this))
{
    connect(model_, &QAbstractItemModel::dataChanged,
            this, &MyDataWidgetMapper::dataChanged);

    // The following connections are for Qt Views (QTableView etc)
//    connect(delegate_, &QAbstractItemDelegate::commitData,
//            this, &MyDataWidgetMapper::commitData);
//    connect(delegate_, &QAbstractItemDelegate::closeEditor,
//            this, &MyDataWidgetMapper::closeEditor);
}

void MyDataWidgetMapper::addMapping(QWidget *widget, const QModelIndex &idx, const char *signal)
{
    removeMapping(widget);
    widgetMap.push_back({widget, idx, signal});
    widget->installEventFilter(delegate_);
    if (signal) {
        const QMetaObject* wmo = widget->metaObject();
        int is = wmo->indexOfSignal(signal);
        QMetaMethod M1 = wmo->method(is);
        is = metaObject()->indexOfSlot("commitData_()");
        QMetaMethod M2 = metaObject()->method(is);
        bool ret = connect(widget, M1,
                           this, M2);
        assert(ret);
    }
}

void MyDataWidgetMapper::removeMapping(QWidget *widget)
{
    int idx = findWidget(widget);
    if (idx == -1)
        return;

    if (widgetMap[idx].signal)
        disconnect(widget, widgetMap[idx].signal, this, SLOT(commitData_()));
    widgetMap.erase(widgetMap.begin() + idx);
    widget->removeEventFilter(delegate_);
}

int MyDataWidgetMapper::findWidget(QWidget *w) const
{
    for (const mapItem &m : widgetMap) {
        if (m.widget == w)
            return int(&m - &widgetMap.front());
    }
    return -1;
}

QWidget* MyDataWidgetMapper::findWidget(const QString& key) const
{
    for (const mapItem &m : widgetMap) {
        if (m.widget->objectName() == key)
            return m.widget;
    }
    return nullptr;
}

void MyDataWidgetMapper::revert()
{
    for (mapItem &e : widgetMap)
        populate(e);
}

bool MyDataWidgetMapper::submit()
{
    for (auto &m : widgetMap) {
        if (m.widget.isNull()) continue;

        if (m.idx.isValid())
            delegate_->setModelData(m.widget, model_, m.idx);
        else return false;
    }

    return model_->submit();
}

static bool qContainsIndex(const QModelIndex &idx, const QModelIndex &topLeft,
                           const QModelIndex &bottomRight)
{
    return idx.row() >= topLeft.row() && idx.row() <= bottomRight.row()
           && idx.column() >= topLeft.column() && idx.column() <= bottomRight.column();
}

void MyDataWidgetMapper::dataChanged(const QModelIndex &topLeft,
                                     const QModelIndex &bottomRight,
                                     const QVector<int> &)
{
    for (mapItem &m : widgetMap) {
        if (qContainsIndex(m.idx, topLeft, bottomRight))
            populate(m);
    }
}

void MyDataWidgetMapper::populate(mapItem &m)
{
    if (m.widget.isNull())
        return;

    if (m.idx.isValid())
        delegate_->setEditorData(m.widget, m.idx);
}

void MyDataWidgetMapper::commitData(QWidget *w)
{
    //if (submitPolicy == QDataWidgetMapper::ManualSubmit)
    //    return;

    int idx = findWidget(w);
    if (idx == -1)
        return; // not our widget

    commit(widgetMap[idx]);
}

void MyDataWidgetMapper::commitData_()
{
    QWidget *w = qobject_cast<QWidget*>(sender());
    commitData(w);
}

bool MyDataWidgetMapper::commit(const mapItem &m)
{
    if (m.widget.isNull())
        return true; // just ignore

    if (!m.idx.isValid())
        return false;

    delegate_->setModelData(m.widget, model_, m.idx);

    return true;
}

void MyDataWidgetMapper::closeEditor(QWidget *w, int i)
{
    int idx = findWidget(w);
    if (idx == -1)
        return; // not our widget

    QAbstractItemDelegate::EndEditHint hint =
        (QAbstractItemDelegate::EndEditHint)i;

    switch (hint) {
    case QAbstractItemDelegate::RevertModelCache:
    {
        populate(widgetMap[idx]);
        break;
    }
//    case QAbstractItemDelegate::EditNextItem:
//        w->focusNextChild();
//        break;
//    case QAbstractItemDelegate::EditPreviousItem:
//        w->focusPreviousChild();
//        break;
    case QAbstractItemDelegate::EditNextItem:
    case QAbstractItemDelegate::EditPreviousItem:
    case QAbstractItemDelegate::SubmitModelCache:
    case QAbstractItemDelegate::NoHint:
        // nothing
        break;
    }
}

