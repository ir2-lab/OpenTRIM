/****************************************************************************
** Meta object code from reading C++ file 'floatlineedit.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.13)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../../source/gui/floatlineedit.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'floatlineedit.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.13. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_FloatValidator_t {
    QByteArrayData data[8];
    char stringdata0[77];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_FloatValidator_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_FloatValidator_t qt_meta_stringdata_FloatValidator = {
    {
QT_MOC_LITERAL(0, 0, 14), // "FloatValidator"
QT_MOC_LITERAL(1, 15, 13), // "bottomChanged"
QT_MOC_LITERAL(2, 29, 0), // ""
QT_MOC_LITERAL(3, 30, 6), // "bottom"
QT_MOC_LITERAL(4, 37, 10), // "topChanged"
QT_MOC_LITERAL(5, 48, 3), // "top"
QT_MOC_LITERAL(6, 52, 15), // "decimalsChanged"
QT_MOC_LITERAL(7, 68, 8) // "decimals"

    },
    "FloatValidator\0bottomChanged\0\0bottom\0"
    "topChanged\0top\0decimalsChanged\0decimals"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_FloatValidator[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       3,   38, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   29,    2, 0x06 /* Public */,
       4,    1,   32,    2, 0x06 /* Public */,
       6,    1,   35,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Float,    3,
    QMetaType::Void, QMetaType::Float,    5,
    QMetaType::Void, QMetaType::Int,    7,

 // properties: name, type, flags
       3, QMetaType::Float, 0x00495103,
       5, QMetaType::Float, 0x00495103,
       7, QMetaType::Int, 0x00495103,

 // properties: notify_signal_id
       0,
       1,
       2,

       0        // eod
};

void FloatValidator::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<FloatValidator *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->bottomChanged((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 1: _t->topChanged((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 2: _t->decimalsChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (FloatValidator::*)(float );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&FloatValidator::bottomChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (FloatValidator::*)(float );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&FloatValidator::topChanged)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (FloatValidator::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&FloatValidator::decimalsChanged)) {
                *result = 2;
                return;
            }
        }
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<FloatValidator *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< float*>(_v) = _t->bottom(); break;
        case 1: *reinterpret_cast< float*>(_v) = _t->top(); break;
        case 2: *reinterpret_cast< int*>(_v) = _t->decimals(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        auto *_t = static_cast<FloatValidator *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setBottom(*reinterpret_cast< float*>(_v)); break;
        case 1: _t->setTop(*reinterpret_cast< float*>(_v)); break;
        case 2: _t->setDecimals(*reinterpret_cast< int*>(_v)); break;
        default: break;
        }
    } else if (_c == QMetaObject::ResetProperty) {
    }
#endif // QT_NO_PROPERTIES
}

QT_INIT_METAOBJECT const QMetaObject FloatValidator::staticMetaObject = { {
    QMetaObject::SuperData::link<QValidator::staticMetaObject>(),
    qt_meta_stringdata_FloatValidator.data,
    qt_meta_data_FloatValidator,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *FloatValidator::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *FloatValidator::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_FloatValidator.stringdata0))
        return static_cast<void*>(this);
    return QValidator::qt_metacast(_clname);
}

int FloatValidator::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QValidator::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 3;
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 3;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 3;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 3;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 3;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 3;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// SIGNAL 0
void FloatValidator::bottomChanged(float _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void FloatValidator::topChanged(float _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void FloatValidator::decimalsChanged(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
struct qt_meta_stringdata_VectorValidator_t {
    QByteArrayData data[6];
    char stringdata0[53];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_VectorValidator_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_VectorValidator_t qt_meta_stringdata_VectorValidator = {
    {
QT_MOC_LITERAL(0, 0, 15), // "VectorValidator"
QT_MOC_LITERAL(1, 16, 13), // "bottomChanged"
QT_MOC_LITERAL(2, 30, 0), // ""
QT_MOC_LITERAL(3, 31, 6), // "bottom"
QT_MOC_LITERAL(4, 38, 10), // "topChanged"
QT_MOC_LITERAL(5, 49, 3) // "top"

    },
    "VectorValidator\0bottomChanged\0\0bottom\0"
    "topChanged\0top"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_VectorValidator[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       2,   30, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   24,    2, 0x06 /* Public */,
       4,    1,   27,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Float,    3,
    QMetaType::Void, QMetaType::Float,    5,

 // properties: name, type, flags
       3, QMetaType::Float, 0x00495103,
       5, QMetaType::Float, 0x00495103,

 // properties: notify_signal_id
       0,
       1,

       0        // eod
};

void VectorValidator::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<VectorValidator *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->bottomChanged((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 1: _t->topChanged((*reinterpret_cast< float(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (VectorValidator::*)(float );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&VectorValidator::bottomChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (VectorValidator::*)(float );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&VectorValidator::topChanged)) {
                *result = 1;
                return;
            }
        }
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<VectorValidator *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< float*>(_v) = _t->bottom(); break;
        case 1: *reinterpret_cast< float*>(_v) = _t->top(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        auto *_t = static_cast<VectorValidator *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setBottom(*reinterpret_cast< float*>(_v)); break;
        case 1: _t->setTop(*reinterpret_cast< float*>(_v)); break;
        default: break;
        }
    } else if (_c == QMetaObject::ResetProperty) {
    }
#endif // QT_NO_PROPERTIES
}

QT_INIT_METAOBJECT const QMetaObject VectorValidator::staticMetaObject = { {
    QMetaObject::SuperData::link<QValidator::staticMetaObject>(),
    qt_meta_stringdata_VectorValidator.data,
    qt_meta_data_VectorValidator,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *VectorValidator::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *VectorValidator::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_VectorValidator.stringdata0))
        return static_cast<void*>(this);
    return QValidator::qt_metacast(_clname);
}

int VectorValidator::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QValidator::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 2;
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 2;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// SIGNAL 0
void VectorValidator::bottomChanged(float _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void VectorValidator::topChanged(float _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
struct qt_meta_stringdata_IntVectorValidator_t {
    QByteArrayData data[6];
    char stringdata0[56];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_IntVectorValidator_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_IntVectorValidator_t qt_meta_stringdata_IntVectorValidator = {
    {
QT_MOC_LITERAL(0, 0, 18), // "IntVectorValidator"
QT_MOC_LITERAL(1, 19, 13), // "bottomChanged"
QT_MOC_LITERAL(2, 33, 0), // ""
QT_MOC_LITERAL(3, 34, 6), // "bottom"
QT_MOC_LITERAL(4, 41, 10), // "topChanged"
QT_MOC_LITERAL(5, 52, 3) // "top"

    },
    "IntVectorValidator\0bottomChanged\0\0"
    "bottom\0topChanged\0top"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_IntVectorValidator[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       2,   30, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   24,    2, 0x06 /* Public */,
       4,    1,   27,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void, QMetaType::Int,    5,

 // properties: name, type, flags
       3, QMetaType::Int, 0x00495103,
       5, QMetaType::Int, 0x00495103,

 // properties: notify_signal_id
       0,
       1,

       0        // eod
};

void IntVectorValidator::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<IntVectorValidator *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->bottomChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->topChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (IntVectorValidator::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&IntVectorValidator::bottomChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (IntVectorValidator::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&IntVectorValidator::topChanged)) {
                *result = 1;
                return;
            }
        }
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<IntVectorValidator *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< int*>(_v) = _t->bottom(); break;
        case 1: *reinterpret_cast< int*>(_v) = _t->top(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        auto *_t = static_cast<IntVectorValidator *>(_o);
        (void)_t;
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setBottom(*reinterpret_cast< int*>(_v)); break;
        case 1: _t->setTop(*reinterpret_cast< int*>(_v)); break;
        default: break;
        }
    } else if (_c == QMetaObject::ResetProperty) {
    }
#endif // QT_NO_PROPERTIES
}

QT_INIT_METAOBJECT const QMetaObject IntVectorValidator::staticMetaObject = { {
    QMetaObject::SuperData::link<QValidator::staticMetaObject>(),
    qt_meta_stringdata_IntVectorValidator.data,
    qt_meta_data_IntVectorValidator,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *IntVectorValidator::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *IntVectorValidator::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_IntVectorValidator.stringdata0))
        return static_cast<void*>(this);
    return QValidator::qt_metacast(_clname);
}

int IntVectorValidator::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QValidator::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 2;
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 2;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// SIGNAL 0
void IntVectorValidator::bottomChanged(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void IntVectorValidator::topChanged(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
struct qt_meta_stringdata_FloatLineEdit_t {
    QByteArrayData data[3];
    char stringdata0[26];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_FloatLineEdit_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_FloatLineEdit_t qt_meta_stringdata_FloatLineEdit = {
    {
QT_MOC_LITERAL(0, 0, 13), // "FloatLineEdit"
QT_MOC_LITERAL(1, 14, 10), // "checkInput"
QT_MOC_LITERAL(2, 25, 0) // ""

    },
    "FloatLineEdit\0checkInput\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_FloatLineEdit[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   19,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,

       0        // eod
};

void FloatLineEdit::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<FloatLineEdit *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->checkInput(); break;
        default: ;
        }
    }
    (void)_a;
}

QT_INIT_METAOBJECT const QMetaObject FloatLineEdit::staticMetaObject = { {
    QMetaObject::SuperData::link<QLineEdit::staticMetaObject>(),
    qt_meta_stringdata_FloatLineEdit.data,
    qt_meta_data_FloatLineEdit,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *FloatLineEdit::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *FloatLineEdit::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_FloatLineEdit.stringdata0))
        return static_cast<void*>(this);
    return QLineEdit::qt_metacast(_clname);
}

int FloatLineEdit::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QLineEdit::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 1;
    }
    return _id;
}
struct qt_meta_stringdata_VectorLineEdit_t {
    QByteArrayData data[3];
    char stringdata0[27];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_VectorLineEdit_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_VectorLineEdit_t qt_meta_stringdata_VectorLineEdit = {
    {
QT_MOC_LITERAL(0, 0, 14), // "VectorLineEdit"
QT_MOC_LITERAL(1, 15, 10), // "checkInput"
QT_MOC_LITERAL(2, 26, 0) // ""

    },
    "VectorLineEdit\0checkInput\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_VectorLineEdit[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   19,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,

       0        // eod
};

void VectorLineEdit::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<VectorLineEdit *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->checkInput(); break;
        default: ;
        }
    }
    (void)_a;
}

QT_INIT_METAOBJECT const QMetaObject VectorLineEdit::staticMetaObject = { {
    QMetaObject::SuperData::link<QLineEdit::staticMetaObject>(),
    qt_meta_stringdata_VectorLineEdit.data,
    qt_meta_data_VectorLineEdit,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *VectorLineEdit::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *VectorLineEdit::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_VectorLineEdit.stringdata0))
        return static_cast<void*>(this);
    return QLineEdit::qt_metacast(_clname);
}

int VectorLineEdit::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QLineEdit::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 1;
    }
    return _id;
}
struct qt_meta_stringdata_IntVectorLineEdit_t {
    QByteArrayData data[3];
    char stringdata0[30];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_IntVectorLineEdit_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_IntVectorLineEdit_t qt_meta_stringdata_IntVectorLineEdit = {
    {
QT_MOC_LITERAL(0, 0, 17), // "IntVectorLineEdit"
QT_MOC_LITERAL(1, 18, 10), // "checkInput"
QT_MOC_LITERAL(2, 29, 0) // ""

    },
    "IntVectorLineEdit\0checkInput\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_IntVectorLineEdit[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   19,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,

       0        // eod
};

void IntVectorLineEdit::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<IntVectorLineEdit *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->checkInput(); break;
        default: ;
        }
    }
    (void)_a;
}

QT_INIT_METAOBJECT const QMetaObject IntVectorLineEdit::staticMetaObject = { {
    QMetaObject::SuperData::link<QLineEdit::staticMetaObject>(),
    qt_meta_stringdata_IntVectorLineEdit.data,
    qt_meta_data_IntVectorLineEdit,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *IntVectorLineEdit::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *IntVectorLineEdit::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_IntVectorLineEdit.stringdata0))
        return static_cast<void*>(this);
    return QLineEdit::qt_metacast(_clname);
}

int IntVectorLineEdit::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QLineEdit::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 1)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 1;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
