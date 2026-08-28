#include "optionwidgetmapper.h"

#include "optionsmodel.h"

#include <QMetaObject>
#include <QMetaMethod>
#include <QDebug>
#include <QLabel>

OptionWidgetMapper::OptionWidgetMapper(OptionsModel *m, QObject *parent)
    : QObject{ parent }, model_(m), delegate_(new OptionsItemDelegate(this))
{
    connect(model_, &QAbstractItemModel::dataChanged, this, &OptionWidgetMapper::dataChanged);
}

void OptionWidgetMapper::addMapping(QWidget *w, const QModelIndex &idx, bool isEditor,
                                    const char *signal)
{
    removeMapping(w);
    widgetMap.push_back({ w, idx, signal });
    if (isEditor) {
        w->installEventFilter(delegate_);
        if (signal) {
            const QMetaObject *wmo = w->metaObject();
            int is = wmo->indexOfSignal(signal);
            QMetaMethod M1 = wmo->method(is);
            is = metaObject()->indexOfSlot("commitData_()");
            QMetaMethod M2 = metaObject()->method(is);
            bool ret = connect(w, M1, this, M2);
            assert(ret);
        }
    }
}

void OptionWidgetMapper::removeMapping(const QString &key)
{
    QWidget *w = findWidget(key);
    if (w)
        removeMapping(w);
}

void OptionWidgetMapper::removeMapping(QWidget *w)
{
    int idx = findWidget(w);
    if (idx == -1)
        return;

    if (widgetMap[idx].signal)
        disconnect(w, widgetMap[idx].signal, this, SLOT(commitData_()));
    widgetMap.erase(widgetMap.begin() + idx);
    w->removeEventFilter(delegate_);
}

int OptionWidgetMapper::findWidget(QWidget *w) const
{
    for (const mapItem &m : widgetMap) {
        if (m.widget == w)
            return int(&m - &widgetMap.front());
    }
    return -1;
}

QWidget *OptionWidgetMapper::findWidget(const QString &key) const
{
    for (const mapItem &m : widgetMap) {
        assert(!m.widget.isNull()); // widget must be there
        if (m.widget->objectName() == key)
            return m.widget;
    }
    return nullptr;
}

QModelIndex OptionWidgetMapper::widgetToIndex(QWidget *w) const
{
    int i = findWidget(w);
    if (i >= 0)
        return widgetMap[i].idx;
    return QModelIndex();
}

QWidgetList OptionWidgetMapper::widgets() const
{
    QWidgetList w;
    for (const mapItem &m : widgetMap)
        w << m.widget;
    return w;
}

void OptionWidgetMapper::setToolTip(const QString &txt)
{
    bool set_global_tooltip = !txt.isEmpty();
    for (const mapItem &m : widgetMap) {
        assert(!m.widget.isNull()); // widget must be there
        if (set_global_tooltip)
            m.widget->setToolTip(txt);
        else {
            OptionsItem *item = static_cast<OptionsItem *>(m.idx.internalPointer());
            m.widget->setToolTip(item->toolTip());
        }
    }
}

void OptionWidgetMapper::revert()
{
    for (mapItem &e : widgetMap)
        populate(e);
}

bool OptionWidgetMapper::submit()
{
    for (auto &m : widgetMap) {
        if (m.widget.isNull())
            continue;

        if (m.idx.isValid())
            delegate_->setModelData(m.widget, model_, m.idx);
        else
            return false;
    }

    return model_->submit();
}

void OptionWidgetMapper::setEnabled(bool b)
{
    for (auto &m : widgetMap) {
        if (m.widget.isNull())
            continue;

        m.widget->setEnabled(b);
    }
}

static bool qContainsIndex(const QModelIndex &idx, const QModelIndex &topLeft,
                           const QModelIndex &bottomRight)
{
    return idx.row() >= topLeft.row() && idx.row() <= bottomRight.row()
            && idx.column() >= topLeft.column() && idx.column() <= bottomRight.column();
}

void OptionWidgetMapper::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight,
                                     const QVector<int> &)
{
    for (mapItem &m : widgetMap) {
        if (qContainsIndex(m.idx, topLeft, bottomRight))
            populate(m);
    }
}

void OptionWidgetMapper::populate(mapItem &m)
{
    if (m.widget.isNull())
        return;

    if (m.idx.isValid() && m.signal)
        delegate_->setEditorData(m.widget, m.idx);
}

void OptionWidgetMapper::commitData(QWidget *w)
{
    // if (submitPolicy == QDataWidgetMapper::ManualSubmit)
    //     return;

    int idx = findWidget(w);
    if (idx == -1)
        return; // not our widget

    commit(widgetMap[idx]);
}

void OptionWidgetMapper::commitData_()
{
    QWidget *w = qobject_cast<QWidget *>(sender());
    commitData(w);
}

bool OptionWidgetMapper::commit(const mapItem &m)
{
    if (m.widget.isNull())
        return true; // just ignore

    if (!m.idx.isValid())
        return false;

    delegate_->setModelData(m.widget, model_, m.idx);

    return true;
}

void OptionWidgetMapper::closeEditor(QWidget *w, int i)
{
    int idx = findWidget(w);
    if (idx == -1)
        return; // not our widget

    QAbstractItemDelegate::EndEditHint hint = (QAbstractItemDelegate::EndEditHint)i;

    switch (hint) {
    case QAbstractItemDelegate::RevertModelCache: {
        populate(widgetMap[idx]);
        break;
    }
    case QAbstractItemDelegate::EditNextItem:
    case QAbstractItemDelegate::EditPreviousItem:
    case QAbstractItemDelegate::SubmitModelCache:
    case QAbstractItemDelegate::NoHint:
        // Tab/Backtab focus movement is handled directly in
        // OptionsItemDelegate::eventFilter() -- this slot is never reached
        // for those hints since it isn't connected to the delegate's
        // closeEditor signal at all (editors here are permanent form
        // widgets, not transient QAbstractItemView editors).
        break;
    }
}
