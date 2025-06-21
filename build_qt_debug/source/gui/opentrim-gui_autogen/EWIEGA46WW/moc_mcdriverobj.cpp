/****************************************************************************
** Meta object code from reading C++ file 'mcdriverobj.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.13)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../../source/gui/mcdriverobj.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mcdriverobj.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.13. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_McDriverObj_t {
    QByteArrayData data[24];
    char stringdata0[271];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_McDriverObj_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_McDriverObj_t qt_meta_stringdata_McDriverObj = {
    {
QT_MOC_LITERAL(0, 0, 11), // "McDriverObj"
QT_MOC_LITERAL(1, 12, 19), // "modificationChanged"
QT_MOC_LITERAL(2, 32, 0), // ""
QT_MOC_LITERAL(3, 33, 1), // "b"
QT_MOC_LITERAL(4, 35, 13), // "configChanged"
QT_MOC_LITERAL(5, 49, 15), // "contentsChanged"
QT_MOC_LITERAL(6, 65, 15), // "fileNameChanged"
QT_MOC_LITERAL(7, 81, 17), // "simulationCreated"
QT_MOC_LITERAL(8, 99, 19), // "simulationDestroyed"
QT_MOC_LITERAL(9, 119, 11), // "startSignal"
QT_MOC_LITERAL(10, 131, 17), // "simulationStarted"
QT_MOC_LITERAL(11, 149, 7), // "loadH5_"
QT_MOC_LITERAL(12, 157, 4), // "path"
QT_MOC_LITERAL(13, 162, 7), // "saveH5_"
QT_MOC_LITERAL(14, 170, 13), // "statusChanged"
QT_MOC_LITERAL(15, 184, 11), // "tallyUpdate"
QT_MOC_LITERAL(16, 196, 10), // "setMaxIons"
QT_MOC_LITERAL(17, 207, 1), // "n"
QT_MOC_LITERAL(18, 209, 11), // "setNThreads"
QT_MOC_LITERAL(19, 221, 7), // "setSeed"
QT_MOC_LITERAL(20, 229, 14), // "setUpdInterval"
QT_MOC_LITERAL(21, 244, 6), // "start_"
QT_MOC_LITERAL(22, 251, 9), // "onLoadH5_"
QT_MOC_LITERAL(23, 261, 9) // "onSaveH5_"

    },
    "McDriverObj\0modificationChanged\0\0b\0"
    "configChanged\0contentsChanged\0"
    "fileNameChanged\0simulationCreated\0"
    "simulationDestroyed\0startSignal\0"
    "simulationStarted\0loadH5_\0path\0saveH5_\0"
    "statusChanged\0tallyUpdate\0setMaxIons\0"
    "n\0setNThreads\0setSeed\0setUpdInterval\0"
    "start_\0onLoadH5_\0onSaveH5_"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_McDriverObj[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      19,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
      12,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,  109,    2, 0x06 /* Public */,
       4,    0,  112,    2, 0x06 /* Public */,
       5,    0,  113,    2, 0x06 /* Public */,
       6,    0,  114,    2, 0x06 /* Public */,
       7,    0,  115,    2, 0x06 /* Public */,
       8,    0,  116,    2, 0x06 /* Public */,
       9,    0,  117,    2, 0x06 /* Public */,
      10,    1,  118,    2, 0x06 /* Public */,
      11,    1,  121,    2, 0x06 /* Public */,
      13,    0,  124,    2, 0x06 /* Public */,
      14,    0,  125,    2, 0x06 /* Public */,
      15,    0,  126,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      16,    1,  127,    2, 0x0a /* Public */,
      18,    1,  130,    2, 0x0a /* Public */,
      19,    1,  133,    2, 0x0a /* Public */,
      20,    1,  136,    2, 0x0a /* Public */,
      21,    0,  139,    2, 0x08 /* Private */,
      22,    1,  140,    2, 0x08 /* Private */,
      23,    0,  143,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::Bool,    3,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Bool,    3,
    QMetaType::Void, QMetaType::QString,   12,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,   17,
    QMetaType::Void, QMetaType::Int,   17,
    QMetaType::Void, QMetaType::Int,   17,
    QMetaType::Void, QMetaType::Int,   17,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   12,
    QMetaType::Void,

       0        // eod
};

void McDriverObj::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<McDriverObj *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->modificationChanged((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->configChanged(); break;
        case 2: _t->contentsChanged(); break;
        case 3: _t->fileNameChanged(); break;
        case 4: _t->simulationCreated(); break;
        case 5: _t->simulationDestroyed(); break;
        case 6: _t->startSignal(); break;
        case 7: _t->simulationStarted((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 8: _t->loadH5_((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 9: _t->saveH5_(); break;
        case 10: _t->statusChanged(); break;
        case 11: _t->tallyUpdate(); break;
        case 12: _t->setMaxIons((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 13: _t->setNThreads((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 14: _t->setSeed((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 15: _t->setUpdInterval((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 16: _t->start_(); break;
        case 17: _t->onLoadH5_((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 18: _t->onSaveH5_(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (McDriverObj::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&McDriverObj::modificationChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (McDriverObj::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&McDriverObj::configChanged)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (McDriverObj::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&McDriverObj::contentsChanged)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (McDriverObj::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&McDriverObj::fileNameChanged)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (McDriverObj::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&McDriverObj::simulationCreated)) {
                *result = 4;
                return;
            }
        }
        {
            using _t = void (McDriverObj::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&McDriverObj::simulationDestroyed)) {
                *result = 5;
                return;
            }
        }
        {
            using _t = void (McDriverObj::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&McDriverObj::startSignal)) {
                *result = 6;
                return;
            }
        }
        {
            using _t = void (McDriverObj::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&McDriverObj::simulationStarted)) {
                *result = 7;
                return;
            }
        }
        {
            using _t = void (McDriverObj::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&McDriverObj::loadH5_)) {
                *result = 8;
                return;
            }
        }
        {
            using _t = void (McDriverObj::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&McDriverObj::saveH5_)) {
                *result = 9;
                return;
            }
        }
        {
            using _t = void (McDriverObj::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&McDriverObj::statusChanged)) {
                *result = 10;
                return;
            }
        }
        {
            using _t = void (McDriverObj::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&McDriverObj::tallyUpdate)) {
                *result = 11;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject McDriverObj::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_McDriverObj.data,
    qt_meta_data_McDriverObj,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *McDriverObj::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *McDriverObj::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_McDriverObj.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int McDriverObj::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 19)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 19;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 19)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 19;
    }
    return _id;
}

// SIGNAL 0
void McDriverObj::modificationChanged(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void McDriverObj::configChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void McDriverObj::contentsChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void McDriverObj::fileNameChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void McDriverObj::simulationCreated()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void McDriverObj::simulationDestroyed()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}

// SIGNAL 6
void McDriverObj::startSignal()
{
    QMetaObject::activate(this, &staticMetaObject, 6, nullptr);
}

// SIGNAL 7
void McDriverObj::simulationStarted(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 7, _a);
}

// SIGNAL 8
void McDriverObj::loadH5_(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 8, _a);
}

// SIGNAL 9
void McDriverObj::saveH5_()
{
    QMetaObject::activate(this, &staticMetaObject, 9, nullptr);
}

// SIGNAL 10
void McDriverObj::statusChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 10, nullptr);
}

// SIGNAL 11
void McDriverObj::tallyUpdate()
{
    QMetaObject::activate(this, &staticMetaObject, 11, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
