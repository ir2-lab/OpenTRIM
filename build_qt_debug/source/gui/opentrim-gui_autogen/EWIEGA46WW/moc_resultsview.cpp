/****************************************************************************
** Meta object code from reading C++ file 'resultsview.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.13)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../../source/gui/resultsview.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'resultsview.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.13. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_ResultsView_t {
    QByteArrayData data[14];
    char stringdata0[207];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ResultsView_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ResultsView_t qt_meta_stringdata_ResultsView = {
    {
QT_MOC_LITERAL(0, 0, 11), // "ResultsView"
QT_MOC_LITERAL(1, 12, 19), // "onSimulationCreated"
QT_MOC_LITERAL(2, 32, 0), // ""
QT_MOC_LITERAL(3, 33, 21), // "onSimulationDestroyed"
QT_MOC_LITERAL(4, 55, 13), // "onItemChanged"
QT_MOC_LITERAL(5, 69, 13), // "onTallyUpdate"
QT_MOC_LITERAL(6, 83, 16), // "updatePlotSeries"
QT_MOC_LITERAL(7, 100, 19), // "updateAxisSelection"
QT_MOC_LITERAL(8, 120, 19), // "onPlotSelectChanged"
QT_MOC_LITERAL(9, 140, 16), // "QListWidgetItem*"
QT_MOC_LITERAL(10, 157, 1), // "i"
QT_MOC_LITERAL(11, 159, 22), // "onDataSelectionChanged"
QT_MOC_LITERAL(12, 182, 11), // "onExportCSV"
QT_MOC_LITERAL(13, 194, 12) // "onExportPlot"

    },
    "ResultsView\0onSimulationCreated\0\0"
    "onSimulationDestroyed\0onItemChanged\0"
    "onTallyUpdate\0updatePlotSeries\0"
    "updateAxisSelection\0onPlotSelectChanged\0"
    "QListWidgetItem*\0i\0onDataSelectionChanged\0"
    "onExportCSV\0onExportPlot"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ResultsView[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   64,    2, 0x0a /* Public */,
       3,    0,   65,    2, 0x0a /* Public */,
       4,    0,   66,    2, 0x0a /* Public */,
       5,    0,   67,    2, 0x0a /* Public */,
       6,    0,   68,    2, 0x08 /* Private */,
       7,    0,   69,    2, 0x08 /* Private */,
       8,    1,   70,    2, 0x08 /* Private */,
      11,    0,   73,    2, 0x08 /* Private */,
      12,    0,   74,    2, 0x08 /* Private */,
      13,    0,   75,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 9,   10,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void ResultsView::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ResultsView *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->onSimulationCreated(); break;
        case 1: _t->onSimulationDestroyed(); break;
        case 2: _t->onItemChanged(); break;
        case 3: _t->onTallyUpdate(); break;
        case 4: _t->updatePlotSeries(); break;
        case 5: _t->updateAxisSelection(); break;
        case 6: _t->onPlotSelectChanged((*reinterpret_cast< QListWidgetItem*(*)>(_a[1]))); break;
        case 7: _t->onDataSelectionChanged(); break;
        case 8: _t->onExportCSV(); break;
        case 9: _t->onExportPlot(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject ResultsView::staticMetaObject = { {
    QMetaObject::SuperData::link<QSplitter::staticMetaObject>(),
    qt_meta_stringdata_ResultsView.data,
    qt_meta_data_ResultsView,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *ResultsView::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ResultsView::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ResultsView.stringdata0))
        return static_cast<void*>(this);
    return QSplitter::qt_metacast(_clname);
}

int ResultsView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QSplitter::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 10;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
