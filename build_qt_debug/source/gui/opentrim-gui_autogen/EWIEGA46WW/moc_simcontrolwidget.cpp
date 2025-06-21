/****************************************************************************
** Meta object code from reading C++ file 'simcontrolwidget.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.13)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../../source/gui/simcontrolwidget.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'simcontrolwidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.13. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_SimControlWidget_t {
    QByteArrayData data[10];
    char stringdata0[116];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_SimControlWidget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_SimControlWidget_t qt_meta_stringdata_SimControlWidget = {
    {
QT_MOC_LITERAL(0, 0, 16), // "SimControlWidget"
QT_MOC_LITERAL(1, 17, 7), // "onStart"
QT_MOC_LITERAL(2, 25, 0), // ""
QT_MOC_LITERAL(3, 26, 1), // "b"
QT_MOC_LITERAL(4, 28, 7), // "onReset"
QT_MOC_LITERAL(5, 36, 21), // "onDriverStatusChanged"
QT_MOC_LITERAL(6, 58, 10), // "onSimTimer"
QT_MOC_LITERAL(7, 69, 19), // "onSimulationStarted"
QT_MOC_LITERAL(8, 89, 19), // "onSimulationCreated"
QT_MOC_LITERAL(9, 109, 6) // "revert"

    },
    "SimControlWidget\0onStart\0\0b\0onReset\0"
    "onDriverStatusChanged\0onSimTimer\0"
    "onSimulationStarted\0onSimulationCreated\0"
    "revert"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_SimControlWidget[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   49,    2, 0x08 /* Private */,
       4,    0,   52,    2, 0x08 /* Private */,
       5,    0,   53,    2, 0x08 /* Private */,
       6,    0,   54,    2, 0x08 /* Private */,
       7,    1,   55,    2, 0x08 /* Private */,
       8,    0,   58,    2, 0x08 /* Private */,
       9,    0,   59,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, QMetaType::Bool,    3,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    3,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void SimControlWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<SimControlWidget *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->onStart((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->onReset(); break;
        case 2: _t->onDriverStatusChanged(); break;
        case 3: _t->onSimTimer(); break;
        case 4: _t->onSimulationStarted((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 5: _t->onSimulationCreated(); break;
        case 6: _t->revert(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject SimControlWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_SimControlWidget.data,
    qt_meta_data_SimControlWidget,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *SimControlWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SimControlWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_SimControlWidget.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int SimControlWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 7;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
