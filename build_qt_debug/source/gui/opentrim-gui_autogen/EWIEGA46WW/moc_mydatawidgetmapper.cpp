/****************************************************************************
** Meta object code from reading C++ file 'mydatawidgetmapper.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.13)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../../source/gui/mydatawidgetmapper.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#include <QtCore/QVector>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mydatawidgetmapper.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.13. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_MyDataWidgetMapper_t {
    QByteArrayData data[15];
    char stringdata0[148];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MyDataWidgetMapper_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MyDataWidgetMapper_t qt_meta_stringdata_MyDataWidgetMapper = {
    {
QT_MOC_LITERAL(0, 0, 18), // "MyDataWidgetMapper"
QT_MOC_LITERAL(1, 19, 6), // "revert"
QT_MOC_LITERAL(2, 26, 0), // ""
QT_MOC_LITERAL(3, 27, 6), // "submit"
QT_MOC_LITERAL(4, 34, 10), // "setEnabled"
QT_MOC_LITERAL(5, 45, 1), // "b"
QT_MOC_LITERAL(6, 47, 11), // "dataChanged"
QT_MOC_LITERAL(7, 59, 11), // "QModelIndex"
QT_MOC_LITERAL(8, 71, 7), // "topLeft"
QT_MOC_LITERAL(9, 79, 11), // "bottomRight"
QT_MOC_LITERAL(10, 91, 12), // "QVector<int>"
QT_MOC_LITERAL(11, 104, 10), // "commitData"
QT_MOC_LITERAL(12, 115, 8), // "QWidget*"
QT_MOC_LITERAL(13, 124, 11), // "commitData_"
QT_MOC_LITERAL(14, 136, 11) // "closeEditor"

    },
    "MyDataWidgetMapper\0revert\0\0submit\0"
    "setEnabled\0b\0dataChanged\0QModelIndex\0"
    "topLeft\0bottomRight\0QVector<int>\0"
    "commitData\0QWidget*\0commitData_\0"
    "closeEditor"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MyDataWidgetMapper[] = {

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
       1,    0,   49,    2, 0x0a /* Public */,
       3,    0,   50,    2, 0x0a /* Public */,
       4,    1,   51,    2, 0x0a /* Public */,
       6,    3,   54,    2, 0x08 /* Private */,
      11,    1,   61,    2, 0x08 /* Private */,
      13,    0,   64,    2, 0x08 /* Private */,
      14,    2,   65,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Bool,
    QMetaType::Void, QMetaType::Bool,    5,
    QMetaType::Void, 0x80000000 | 7, 0x80000000 | 7, 0x80000000 | 10,    8,    9,    2,
    QMetaType::Void, 0x80000000 | 12,    2,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 12, QMetaType::Int,    2,    2,

       0        // eod
};

void MyDataWidgetMapper::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MyDataWidgetMapper *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->revert(); break;
        case 1: { bool _r = _t->submit();
            if (_a[0]) *reinterpret_cast< bool*>(_a[0]) = std::move(_r); }  break;
        case 2: _t->setEnabled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 3: _t->dataChanged((*reinterpret_cast< const QModelIndex(*)>(_a[1])),(*reinterpret_cast< const QModelIndex(*)>(_a[2])),(*reinterpret_cast< const QVector<int>(*)>(_a[3]))); break;
        case 4: _t->commitData((*reinterpret_cast< QWidget*(*)>(_a[1]))); break;
        case 5: _t->commitData_(); break;
        case 6: _t->closeEditor((*reinterpret_cast< QWidget*(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 3:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 2:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QVector<int> >(); break;
            }
            break;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject MyDataWidgetMapper::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_MyDataWidgetMapper.data,
    qt_meta_data_MyDataWidgetMapper,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *MyDataWidgetMapper::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MyDataWidgetMapper::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MyDataWidgetMapper.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int MyDataWidgetMapper::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
