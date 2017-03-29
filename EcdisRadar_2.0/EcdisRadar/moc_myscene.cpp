/****************************************************************************
** Meta object code from reading C++ file 'myscene.h'
**
** Created: Wed Mar 22 16:26:08 2017
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "myscene.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'myscene.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_MyScene[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      27,    9,    8,    8, 0x0a,
      77,   52,    8,    8, 0x0a,
     115,    8,    8,    8, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_MyScene[] = {
    "MyScene\0\0chart_description\0"
    "renderChart(std::string)\0"
    "nowChart,area,line,point\0"
    "refreshScreen(QString,bool,bool,bool)\0"
    "slot_scale(float)\0"
};

void MyScene::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        MyScene *_t = static_cast<MyScene *>(_o);
        switch (_id) {
        case 0: _t->renderChart((*reinterpret_cast< std::string(*)>(_a[1]))); break;
        case 1: _t->refreshScreen((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3])),(*reinterpret_cast< bool(*)>(_a[4]))); break;
        case 2: _t->slot_scale((*reinterpret_cast< float(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData MyScene::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject MyScene::staticMetaObject = {
    { &QGraphicsScene::staticMetaObject, qt_meta_stringdata_MyScene,
      qt_meta_data_MyScene, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MyScene::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MyScene::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MyScene::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MyScene))
        return static_cast<void*>(const_cast< MyScene*>(this));
    return QGraphicsScene::qt_metacast(_clname);
}

int MyScene::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QGraphicsScene::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
