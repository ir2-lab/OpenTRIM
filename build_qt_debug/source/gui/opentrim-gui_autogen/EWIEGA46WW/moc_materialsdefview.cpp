/****************************************************************************
** Meta object code from reading C++ file 'materialsdefview.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.13)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../../../../source/gui/materialsdefview.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'materialsdefview.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.13. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_MaterialsDefView_t {
    QByteArrayData data[10];
    char stringdata0[125];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MaterialsDefView_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MaterialsDefView_t qt_meta_stringdata_MaterialsDefView = {
    {
QT_MOC_LITERAL(0, 0, 16), // "MaterialsDefView"
QT_MOC_LITERAL(1, 17, 11), // "addMaterial"
QT_MOC_LITERAL(2, 29, 0), // ""
QT_MOC_LITERAL(3, 30, 14), // "removeMaterial"
QT_MOC_LITERAL(4, 45, 16), // "editMaterialName"
QT_MOC_LITERAL(5, 62, 13), // "setWidgetData"
QT_MOC_LITERAL(6, 76, 12), // "setValueData"
QT_MOC_LITERAL(7, 89, 22), // "updateSelectedMaterial"
QT_MOC_LITERAL(8, 112, 10), // "setDensity"
QT_MOC_LITERAL(9, 123, 1) // "v"

    },
    "MaterialsDefView\0addMaterial\0\0"
    "removeMaterial\0editMaterialName\0"
    "setWidgetData\0setValueData\0"
    "updateSelectedMaterial\0setDensity\0v"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MaterialsDefView[] = {

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
       4,    0,   51,    2, 0x0a /* Public */,
       5,    0,   52,    2, 0x0a /* Public */,
       6,    0,   53,    2, 0x0a /* Public */,
       7,    0,   54,    2, 0x0a /* Public */,
       8,    1,   55,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Double,    9,

       0        // eod
};

void MaterialsDefView::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MaterialsDefView *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->addMaterial(); break;
        case 1: _t->removeMaterial(); break;
        case 2: _t->editMaterialName(); break;
        case 3: _t->setWidgetData(); break;
        case 4: _t->setValueData(); break;
        case 5: _t->updateSelectedMaterial(); break;
        case 6: _t->setDensity((*reinterpret_cast< double(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject MaterialsDefView::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_MaterialsDefView.data,
    qt_meta_data_MaterialsDefView,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *MaterialsDefView::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MaterialsDefView::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MaterialsDefView.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int MaterialsDefView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
struct qt_meta_stringdata_MyComboBox_t {
    QByteArrayData data[3];
    char stringdata0[26];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MyComboBox_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MyComboBox_t qt_meta_stringdata_MyComboBox = {
    {
QT_MOC_LITERAL(0, 0, 10), // "MyComboBox"
QT_MOC_LITERAL(1, 11, 13), // "doubleClicked"
QT_MOC_LITERAL(2, 25, 0) // ""

    },
    "MyComboBox\0doubleClicked\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MyComboBox[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   19,    2, 0x06 /* Public */,

 // signals: parameters
    QMetaType::Void,

       0        // eod
};

void MyComboBox::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MyComboBox *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->doubleClicked(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (MyComboBox::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MyComboBox::doubleClicked)) {
                *result = 0;
                return;
            }
        }
    }
    (void)_a;
}

QT_INIT_METAOBJECT const QMetaObject MyComboBox::staticMetaObject = { {
    QMetaObject::SuperData::link<QComboBox::staticMetaObject>(),
    qt_meta_stringdata_MyComboBox.data,
    qt_meta_data_MyComboBox,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *MyComboBox::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MyComboBox::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MyComboBox.stringdata0))
        return static_cast<void*>(this);
    return QComboBox::qt_metacast(_clname);
}

int MyComboBox::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QComboBox::qt_metacall(_c, _id, _a);
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

// SIGNAL 0
void MyComboBox::doubleClicked()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}
struct qt_meta_stringdata_MaterialCompositionModel_t {
    QByteArrayData data[1];
    char stringdata0[25];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MaterialCompositionModel_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MaterialCompositionModel_t qt_meta_stringdata_MaterialCompositionModel = {
    {
QT_MOC_LITERAL(0, 0, 24) // "MaterialCompositionModel"

    },
    "MaterialCompositionModel"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MaterialCompositionModel[] = {

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

void MaterialCompositionModel::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    (void)_o;
    (void)_id;
    (void)_c;
    (void)_a;
}

QT_INIT_METAOBJECT const QMetaObject MaterialCompositionModel::staticMetaObject = { {
    QMetaObject::SuperData::link<QAbstractTableModel::staticMetaObject>(),
    qt_meta_stringdata_MaterialCompositionModel.data,
    qt_meta_data_MaterialCompositionModel,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *MaterialCompositionModel::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MaterialCompositionModel::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MaterialCompositionModel.stringdata0))
        return static_cast<void*>(this);
    return QAbstractTableModel::qt_metacast(_clname);
}

int MaterialCompositionModel::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QAbstractTableModel::qt_metacall(_c, _id, _a);
    return _id;
}
struct qt_meta_stringdata_MaterialCompositionDelegate_t {
    QByteArrayData data[1];
    char stringdata0[28];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MaterialCompositionDelegate_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MaterialCompositionDelegate_t qt_meta_stringdata_MaterialCompositionDelegate = {
    {
QT_MOC_LITERAL(0, 0, 27) // "MaterialCompositionDelegate"

    },
    "MaterialCompositionDelegate"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MaterialCompositionDelegate[] = {

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

void MaterialCompositionDelegate::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    (void)_o;
    (void)_id;
    (void)_c;
    (void)_a;
}

QT_INIT_METAOBJECT const QMetaObject MaterialCompositionDelegate::staticMetaObject = { {
    QMetaObject::SuperData::link<QStyledItemDelegate::staticMetaObject>(),
    qt_meta_stringdata_MaterialCompositionDelegate.data,
    qt_meta_data_MaterialCompositionDelegate,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *MaterialCompositionDelegate::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MaterialCompositionDelegate::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MaterialCompositionDelegate.stringdata0))
        return static_cast<void*>(this);
    return QStyledItemDelegate::qt_metacast(_clname);
}

int MaterialCompositionDelegate::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QStyledItemDelegate::qt_metacall(_c, _id, _a);
    return _id;
}
struct qt_meta_stringdata_MaterialCompositionView_t {
    QByteArrayData data[8];
    char stringdata0[104];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MaterialCompositionView_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MaterialCompositionView_t qt_meta_stringdata_MaterialCompositionView = {
    {
QT_MOC_LITERAL(0, 0, 23), // "MaterialCompositionView"
QT_MOC_LITERAL(1, 24, 10), // "addElement"
QT_MOC_LITERAL(2, 35, 0), // ""
QT_MOC_LITERAL(3, 36, 13), // "removeElement"
QT_MOC_LITERAL(4, 50, 18), // "onSelectionChanged"
QT_MOC_LITERAL(5, 69, 14), // "QItemSelection"
QT_MOC_LITERAL(6, 84, 8), // "selected"
QT_MOC_LITERAL(7, 93, 10) // "deselected"

    },
    "MaterialCompositionView\0addElement\0\0"
    "removeElement\0onSelectionChanged\0"
    "QItemSelection\0selected\0deselected"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MaterialCompositionView[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   29,    2, 0x0a /* Public */,
       3,    0,   30,    2, 0x0a /* Public */,
       4,    2,   31,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 5, 0x80000000 | 5,    6,    7,

       0        // eod
};

void MaterialCompositionView::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MaterialCompositionView *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->addElement(); break;
        case 1: _t->removeElement(); break;
        case 2: _t->onSelectionChanged((*reinterpret_cast< const QItemSelection(*)>(_a[1])),(*reinterpret_cast< const QItemSelection(*)>(_a[2]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject MaterialCompositionView::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_MaterialCompositionView.data,
    qt_meta_data_MaterialCompositionView,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *MaterialCompositionView::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MaterialCompositionView::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MaterialCompositionView.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int MaterialCompositionView::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
QT_WARNING_POP
QT_END_MOC_NAMESPACE
