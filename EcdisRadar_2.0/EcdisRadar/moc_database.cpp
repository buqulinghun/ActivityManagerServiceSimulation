/****************************************************************************
** Meta object code from reading C++ file 'database.h'
**
** Created: Wed Mar 29 09:34:51 2017
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "database.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'database.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_DataBase[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: signature, parameters, type, tag, flags
      22,   10,    9,    9, 0x05,
      62,   49,    9,    9, 0x05,

 // slots: signature, parameters, type, tag, flags
     105,    9,    9,    9, 0x0a,
     119,    9,    9,    9, 0x0a,
     133,    9,    9,    9, 0x0a,
     147,    9,    9,    9, 0x0a,
     166,    9,    9,    9, 0x0a,
     183,  177,    9,    9, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_DataBase[] = {
    "DataBase\0\0description\0readyToRender(std::string)\0"
    "chartName,,,\0readyToRenderOther(QString,bool,bool,bool)\0"
    "queryRecord()\0closeDialog()\0setTextFlag()\0"
    "openChartClicked()\0addChart()\0index\0"
    "doubleRowClicked(QModelIndex)\0"
};

void DataBase::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        DataBase *_t = static_cast<DataBase *>(_o);
        switch (_id) {
        case 0: _t->readyToRender((*reinterpret_cast< std::string(*)>(_a[1]))); break;
        case 1: _t->readyToRenderOther((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3])),(*reinterpret_cast< bool(*)>(_a[4]))); break;
        case 2: _t->queryRecord(); break;
        case 3: _t->closeDialog(); break;
        case 4: _t->setTextFlag(); break;
        case 5: _t->openChartClicked(); break;
        case 6: _t->addChart(); break;
        case 7: _t->doubleRowClicked((*reinterpret_cast< const QModelIndex(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData DataBase::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject DataBase::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_DataBase,
      qt_meta_data_DataBase, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &DataBase::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *DataBase::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *DataBase::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_DataBase))
        return static_cast<void*>(const_cast< DataBase*>(this));
    return QObject::qt_metacast(_clname);
}

int DataBase::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void DataBase::readyToRender(std::string _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void DataBase::readyToRenderOther(QString _t1, bool _t2, bool _t3, bool _t4)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)), const_cast<void*>(reinterpret_cast<const void*>(&_t3)), const_cast<void*>(reinterpret_cast<const void*>(&_t4)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_END_MOC_NAMESPACE
