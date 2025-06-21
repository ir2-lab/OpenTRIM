/****************************************************************************
** Meta object code from reading C++ file 'periodictablewidget.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.13)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../../source/gui/periodictablewidget.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'periodictablewidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.13. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_ElementButton_t {
    QByteArrayData data[4];
    char stringdata0[53];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ElementButton_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ElementButton_t qt_meta_stringdata_ElementButton = {
    {
QT_MOC_LITERAL(0, 0, 13), // "ElementButton"
QT_MOC_LITERAL(1, 14, 15), // "isotopeSelected"
QT_MOC_LITERAL(2, 30, 0), // ""
QT_MOC_LITERAL(3, 31, 21) // "isotopeActionTrigered"

    },
    "ElementButton\0isotopeSelected\0\0"
    "isotopeActionTrigered"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ElementButton[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   24,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       3,    0,   25,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,

       0        // eod
};

void ElementButton::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ElementButton *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->isotopeSelected(); break;
        case 1: _t->isotopeActionTrigered(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (ElementButton::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ElementButton::isotopeSelected)) {
                *result = 0;
                return;
            }
        }
    }
    (void)_a;
}

QT_INIT_METAOBJECT const QMetaObject ElementButton::staticMetaObject = { {
    QMetaObject::SuperData::link<QToolButton::staticMetaObject>(),
    qt_meta_stringdata_ElementButton.data,
    qt_meta_data_ElementButton,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *ElementButton::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ElementButton::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ElementButton.stringdata0))
        return static_cast<void*>(this);
    return QToolButton::qt_metacast(_clname);
}

int ElementButton::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QToolButton::qt_metacall(_c, _id, _a);
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
    return _id;
}

// SIGNAL 0
void ElementButton::isotopeSelected()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
struct qt_meta_stringdata_PeriodicTableWidget_t {
    QByteArrayData data[5];
    char stringdata0[71];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_PeriodicTableWidget_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_PeriodicTableWidget_t qt_meta_stringdata_PeriodicTableWidget = {
    {
QT_MOC_LITERAL(0, 0, 19), // "PeriodicTableWidget"
QT_MOC_LITERAL(1, 20, 15), // "elementSelected"
QT_MOC_LITERAL(2, 36, 0), // ""
QT_MOC_LITERAL(3, 37, 19), // "singleElementSelect"
QT_MOC_LITERAL(4, 57, 13) // "isotopeSelect"

    },
    "PeriodicTableWidget\0elementSelected\0"
    "\0singleElementSelect\0isotopeSelect"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_PeriodicTableWidget[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   29,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       3,    0,   30,    2, 0x0a /* Public */,
       4,    0,   31,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void PeriodicTableWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<PeriodicTableWidget *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->elementSelected(); break;
        case 1: _t->singleElementSelect(); break;
        case 2: _t->isotopeSelect(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (PeriodicTableWidget::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&PeriodicTableWidget::elementSelected)) {
                *result = 0;
                return;
            }
        }
    }
    (void)_a;
}

QT_INIT_METAOBJECT const QMetaObject PeriodicTableWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_PeriodicTableWidget.data,
    qt_meta_data_PeriodicTableWidget,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *PeriodicTableWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PeriodicTableWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_PeriodicTableWidget.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int PeriodicTableWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
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
    return _id;
}

// SIGNAL 0
void PeriodicTableWidget::elementSelected()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
struct qt_meta_stringdata_PeriodicTableDialog_t {
    QByteArrayData data[1];
    char stringdata0[20];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_PeriodicTableDialog_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_PeriodicTableDialog_t qt_meta_stringdata_PeriodicTableDialog = {
    {
QT_MOC_LITERAL(0, 0, 19) // "PeriodicTableDialog"

    },
    "PeriodicTableDialog"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_PeriodicTableDialog[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

void PeriodicTableDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    (void)_o;
    (void)_id;
    (void)_c;
    (void)_a;
}

QT_INIT_METAOBJECT const QMetaObject PeriodicTableDialog::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_PeriodicTableDialog.data,
    qt_meta_data_PeriodicTableDialog,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *PeriodicTableDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *PeriodicTableDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_PeriodicTableDialog.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int PeriodicTableDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
