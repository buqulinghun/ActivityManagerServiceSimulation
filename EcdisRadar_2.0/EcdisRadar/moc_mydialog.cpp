/****************************************************************************
** Meta object code from reading C++ file 'mydialog.h'
**
** Created: Wed Mar 22 16:26:11 2017
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "mydialog.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mydialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_MyDialog[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      15,   10,    9,    9, 0x0a,
      39,   10,    9,    9, 0x0a,
      61,    9,    9,    9, 0x0a,
      75,    9,    9,    9, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_MyDialog[] = {
    "MyDialog\0\0date\0setStartTime(QDateTime)\0"
    "setEndTime(QDateTime)\0queryRecord()\0"
    "closeDialog()\0"
};

void MyDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        MyDialog *_t = static_cast<MyDialog *>(_o);
        switch (_id) {
        case 0: _t->setStartTime((*reinterpret_cast< const QDateTime(*)>(_a[1]))); break;
        case 1: _t->setEndTime((*reinterpret_cast< const QDateTime(*)>(_a[1]))); break;
        case 2: _t->queryRecord(); break;
        case 3: _t->closeDialog(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData MyDialog::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject MyDialog::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_MyDialog,
      qt_meta_data_MyDialog, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MyDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MyDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MyDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MyDialog))
        return static_cast<void*>(const_cast< MyDialog*>(this));
    return QObject::qt_metacast(_clname);
}

int MyDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
