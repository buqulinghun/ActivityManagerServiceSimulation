/****************************************************************************
** Meta object code from reading C++ file 'implcommobject.h'
**
** Created: Wed Mar 22 16:26:12 2017
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "radarFile/communicate/implcommobject.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'implcommobject.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_CBaseDevice[] = {

 // content:
       6,       // revision
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

static const char qt_meta_stringdata_CBaseDevice[] = {
    "CBaseDevice\0"
};

void CBaseDevice::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData CBaseDevice::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject CBaseDevice::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_CBaseDevice,
      qt_meta_data_CBaseDevice, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &CBaseDevice::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *CBaseDevice::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *CBaseDevice::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_CBaseDevice))
        return static_cast<void*>(const_cast< CBaseDevice*>(this));
    return QObject::qt_metacast(_clname);
}

int CBaseDevice::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_CUdpDevice[] = {

 // content:
       6,       // revision
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

static const char qt_meta_stringdata_CUdpDevice[] = {
    "CUdpDevice\0"
};

void CUdpDevice::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    Q_UNUSED(_o);
    Q_UNUSED(_id);
    Q_UNUSED(_c);
    Q_UNUSED(_a);
}

const QMetaObjectExtraData CUdpDevice::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject CUdpDevice::staticMetaObject = {
    { &CBaseDevice::staticMetaObject, qt_meta_stringdata_CUdpDevice,
      qt_meta_data_CUdpDevice, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &CUdpDevice::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *CUdpDevice::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *CUdpDevice::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_CUdpDevice))
        return static_cast<void*>(const_cast< CUdpDevice*>(this));
    return CBaseDevice::qt_metacast(_clname);
}

int CUdpDevice::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = CBaseDevice::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    return _id;
}
static const uint qt_meta_data_CSerialDevice[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      15,   14,   14,   14, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_CSerialDevice[] = {
    "CSerialDevice\0\0recvData()\0"
};

void CSerialDevice::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        CSerialDevice *_t = static_cast<CSerialDevice *>(_o);
        switch (_id) {
        case 0: _t->recvData(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData CSerialDevice::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject CSerialDevice::staticMetaObject = {
    { &CBaseDevice::staticMetaObject, qt_meta_stringdata_CSerialDevice,
      qt_meta_data_CSerialDevice, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &CSerialDevice::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *CSerialDevice::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *CSerialDevice::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_CSerialDevice))
        return static_cast<void*>(const_cast< CSerialDevice*>(this));
    return CBaseDevice::qt_metacast(_clname);
}

int CSerialDevice::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = CBaseDevice::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
