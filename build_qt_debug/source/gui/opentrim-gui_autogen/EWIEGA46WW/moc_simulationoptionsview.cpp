/****************************************************************************
** Meta object code from reading C++ file 'simulationoptionsview.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.13)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../../source/gui/simulationoptionsview.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#include <QtCore/QVector>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'simulationoptionsview.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.13. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_SimulationOptionsView_t {
    QByteArrayData data[16];
    char stringdata0[183];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_SimulationOptionsView_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_SimulationOptionsView_t qt_meta_stringdata_SimulationOptionsView = {
    {
QT_MOC_LITERAL(0, 0, 21), // "SimulationOptionsView"
QT_MOC_LITERAL(1, 22, 15), // "modifiedChanged"
QT_MOC_LITERAL(2, 38, 0), // ""
QT_MOC_LITERAL(3, 39, 14), // "optionsChanged"
QT_MOC_LITERAL(4, 54, 11), // "setModified"
QT_MOC_LITERAL(5, 66, 1), // "b"
QT_MOC_LITERAL(6, 68, 12), // "setModified2"
QT_MOC_LITERAL(7, 81, 11), // "QModelIndex"
QT_MOC_LITERAL(8, 93, 12), // "QVector<int>"
QT_MOC_LITERAL(9, 106, 6), // "revert"
QT_MOC_LITERAL(10, 113, 6), // "submit"
QT_MOC_LITERAL(11, 120, 4), // "help"
QT_MOC_LITERAL(12, 125, 10), // "selectIonZ"
QT_MOC_LITERAL(13, 136, 15), // "validateOptions"
QT_MOC_LITERAL(14, 152, 21), // "onDriverStatusChanged"
QT_MOC_LITERAL(15, 174, 8) // "modified"

    },
    "SimulationOptionsView\0modifiedChanged\0"
    "\0optionsChanged\0setModified\0b\0"
    "setModified2\0QModelIndex\0QVector<int>\0"
    "revert\0submit\0help\0selectIonZ\0"
    "validateOptions\0onDriverStatusChanged\0"
    "modified"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_SimulationOptionsView[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       1,   90, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   69,    2, 0x06 /* Public */,
       3,    0,   72,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       4,    1,   73,    2, 0x0a /* Public */,
       4,    0,   76,    2, 0x2a /* Public | MethodCloned */,
       6,    3,   77,    2, 0x0a /* Public */,
       9,    0,   84,    2, 0x0a /* Public */,
      10,    0,   85,    2, 0x0a /* Public */,
      11,    0,   86,    2, 0x0a /* Public */,
      12,    0,   87,    2, 0x0a /* Public */,
      13,    0,   88,    2, 0x0a /* Public */,
      14,    0,   89,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, QMetaType::Bool,    5,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 7, 0x80000000 | 7, 0x80000000 | 8,    2,    2,    2,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // properties: name, type, flags
      15, QMetaType::Bool, 0x00495903,

 // properties: notify_signal_id
       0,

       0        // eod
};

void SimulationOptionsView::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<SimulationOptionsView *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->modifiedChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->optionsChanged(); break;
        case 2: _t->setModified((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 3: _t->setModified(); break;
        case 4: _t->setModified2((*reinterpret_cast< const QModelIndex(*)>(_a[1])),(*reinterpret_cast< const QModelIndex(*)>(_a[2])),(*reinterpret_cast< const QVector<int>(*)>(_a[3]))); break;
        case 5: _t->revert(); break;
        case 6: _t->submit(); break;
        case 7: _t->help(); break;
        case 8: _t->selectIonZ(); break;
        case 9: _t->validateOptions(); break;
        case 10: _t->onDriverStatusChanged(); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 4:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 2:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QVector<int> >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (SimulationOptionsView::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SimulationOptionsView::modifiedChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (SimulationOptionsView::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&SimulationOptionsView::optionsChanged)) {
                *result = 1;
                return;
            }
        }
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<SimulationOptionsView *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< bool*>(_v) = _t->modified(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        auto *_t = static_cast<SimulationOptionsView *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setModified(*reinterpret_cast< bool*>(_v)); break;
        default: break;
        }
    } else if (_c == QMetaObject::ResetProperty) {
    }
#endif // QT_NO_PROPERTIES
}

QT_INIT_METAOBJECT const QMetaObject SimulationOptionsView::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_SimulationOptionsView.data,
    qt_meta_data_SimulationOptionsView,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *SimulationOptionsView::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SimulationOptionsView::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_SimulationOptionsView.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int SimulationOptionsView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 1;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 1;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// SIGNAL 0
void SimulationOptionsView::modifiedChanged(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void SimulationOptionsView::optionsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
