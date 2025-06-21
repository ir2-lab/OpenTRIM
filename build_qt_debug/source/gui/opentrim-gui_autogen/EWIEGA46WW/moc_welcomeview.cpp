/****************************************************************************
** Meta object code from reading C++ file 'welcomeview.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.13)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../../source/gui/welcomeview.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'welcomeview.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.13. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_WelcomeView_t {
    QByteArrayData data[17];
    char stringdata0[209];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_WelcomeView_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_WelcomeView_t qt_meta_stringdata_WelcomeView = {
    {
QT_MOC_LITERAL(0, 0, 11), // "WelcomeView"
QT_MOC_LITERAL(1, 12, 18), // "changeCenterWidget"
QT_MOC_LITERAL(2, 31, 0), // ""
QT_MOC_LITERAL(3, 32, 2), // "id"
QT_MOC_LITERAL(4, 35, 5), // "onNew"
QT_MOC_LITERAL(5, 41, 10), // "onOpenJson"
QT_MOC_LITERAL(6, 52, 8), // "onOpenH5"
QT_MOC_LITERAL(7, 61, 12), // "onOpenRecent"
QT_MOC_LITERAL(8, 74, 20), // "onRecentFileSelected"
QT_MOC_LITERAL(9, 95, 10), // "onSaveJson"
QT_MOC_LITERAL(10, 106, 8), // "onSaveH5"
QT_MOC_LITERAL(11, 115, 12), // "onSaveJsonAs"
QT_MOC_LITERAL(12, 128, 10), // "onSaveH5As"
QT_MOC_LITERAL(13, 139, 13), // "onOpenExample"
QT_MOC_LITERAL(14, 153, 21), // "onDriverStatusChanged"
QT_MOC_LITERAL(15, 175, 15), // "exampleSelected"
QT_MOC_LITERAL(16, 191, 17) // "onFileNameChanged"

    },
    "WelcomeView\0changeCenterWidget\0\0id\0"
    "onNew\0onOpenJson\0onOpenH5\0onOpenRecent\0"
    "onRecentFileSelected\0onSaveJson\0"
    "onSaveH5\0onSaveJsonAs\0onSaveH5As\0"
    "onOpenExample\0onDriverStatusChanged\0"
    "exampleSelected\0onFileNameChanged"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_WelcomeView[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      14,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   84,    2, 0x08 /* Private */,
       4,    0,   87,    2, 0x08 /* Private */,
       5,    0,   88,    2, 0x08 /* Private */,
       6,    0,   89,    2, 0x08 /* Private */,
       7,    0,   90,    2, 0x08 /* Private */,
       8,    0,   91,    2, 0x08 /* Private */,
       9,    0,   92,    2, 0x08 /* Private */,
      10,    0,   93,    2, 0x08 /* Private */,
      11,    0,   94,    2, 0x08 /* Private */,
      12,    0,   95,    2, 0x08 /* Private */,
      13,    0,   96,    2, 0x08 /* Private */,
      14,    0,   97,    2, 0x08 /* Private */,
      15,    0,   98,    2, 0x08 /* Private */,
      16,    0,   99,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void, QMetaType::Int,    3,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void WelcomeView::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<WelcomeView *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->changeCenterWidget((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->onNew(); break;
        case 2: _t->onOpenJson(); break;
        case 3: _t->onOpenH5(); break;
        case 4: _t->onOpenRecent(); break;
        case 5: _t->onRecentFileSelected(); break;
        case 6: _t->onSaveJson(); break;
        case 7: _t->onSaveH5(); break;
        case 8: _t->onSaveJsonAs(); break;
        case 9: _t->onSaveH5As(); break;
        case 10: _t->onOpenExample(); break;
        case 11: _t->onDriverStatusChanged(); break;
        case 12: _t->exampleSelected(); break;
        case 13: _t->onFileNameChanged(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject WelcomeView::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_WelcomeView.data,
    qt_meta_data_WelcomeView,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *WelcomeView::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *WelcomeView::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_WelcomeView.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int WelcomeView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 14)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 14;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 14)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 14;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
