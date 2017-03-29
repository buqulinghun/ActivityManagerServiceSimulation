/****************************************************************************
** Meta object code from reading C++ file 'mainwindow.h'
**
** Created: Wed Mar 29 09:34:50 2017
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "mainwindow.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mainwindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_MainWindow[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      28,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      12,   11,   11,   11, 0x08,
      39,   11,   11,   11, 0x08,
      66,   11,   11,   11, 0x08,
      94,   11,   11,   11, 0x08,
     121,   11,   11,   11, 0x08,
     142,   11,   11,   11, 0x08,
     161,   11,   11,   11, 0x08,
     180,   11,   11,   11, 0x08,
     198,   11,   11,   11, 0x08,
     220,   11,   11,   11, 0x08,
     248,   11,   11,   11, 0x08,
     272,   11,   11,   11, 0x08,
     296,   11,   11,   11, 0x08,
     321,   11,   11,   11, 0x08,
     344,   11,   11,   11, 0x08,
     371,   11,   11,   11, 0x08,
     397,   11,   11,   11, 0x08,
     420,   11,   11,   11, 0x08,
     444,   11,   11,   11, 0x08,
     468,   11,   11,   11, 0x08,
     494,   11,   11,   11, 0x08,
     520,   11,   11,   11, 0x08,
     545,   11,   11,   11, 0x08,
     566,   11,   11,   11, 0x08,
     588,   11,   11,   11, 0x08,
     607,   11,   11,   11, 0x08,
     632,   11,   11,   11, 0x08,
     659,   11,   11,   11, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_MainWindow[] = {
    "MainWindow\0\0on_safeContour_triggered()\0"
    "on_chartSelect_triggered()\0"
    "on_alarmRecoder_triggered()\0"
    "on_sailRecoder_triggered()\0"
    "display_clicked(int)\0color_clicked(int)\0"
    "point_clicked(int)\0area_clicked(int)\0"
    "language_clicked(int)\0on_openAndClose_triggered()\0"
    "on_rangeAdd_triggered()\0on_rangeSec_triggered()\0"
    "on_offsetOps_triggered()\0"
    "on_gainOps_triggered()\0"
    "on_restrainOps_triggered()\0"
    "on_clutterOps_triggered()\0"
    "on_tuneMan_triggered()\0on_tuneAuto_triggered()\0"
    "on_action_2_triggered()\0"
    "on_scaleMatch_triggered()\0"
    "on_closeRadar_triggered()\0"
    "on_showRadar_triggered()\0on_green_triggered()\0"
    "on_yellow_triggered()\0on_red_triggered()\0"
    "on_highAlpha_triggered()\0"
    "on_middleAlpha_triggered()\0"
    "on_lowAlpha_triggered()\0"
};

void MainWindow::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        MainWindow *_t = static_cast<MainWindow *>(_o);
        switch (_id) {
        case 0: _t->on_safeContour_triggered(); break;
        case 1: _t->on_chartSelect_triggered(); break;
        case 2: _t->on_alarmRecoder_triggered(); break;
        case 3: _t->on_sailRecoder_triggered(); break;
        case 4: _t->display_clicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->color_clicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 6: _t->point_clicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 7: _t->area_clicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 8: _t->language_clicked((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 9: _t->on_openAndClose_triggered(); break;
        case 10: _t->on_rangeAdd_triggered(); break;
        case 11: _t->on_rangeSec_triggered(); break;
        case 12: _t->on_offsetOps_triggered(); break;
        case 13: _t->on_gainOps_triggered(); break;
        case 14: _t->on_restrainOps_triggered(); break;
        case 15: _t->on_clutterOps_triggered(); break;
        case 16: _t->on_tuneMan_triggered(); break;
        case 17: _t->on_tuneAuto_triggered(); break;
        case 18: _t->on_action_2_triggered(); break;
        case 19: _t->on_scaleMatch_triggered(); break;
        case 20: _t->on_closeRadar_triggered(); break;
        case 21: _t->on_showRadar_triggered(); break;
        case 22: _t->on_green_triggered(); break;
        case 23: _t->on_yellow_triggered(); break;
        case 24: _t->on_red_triggered(); break;
        case 25: _t->on_highAlpha_triggered(); break;
        case 26: _t->on_middleAlpha_triggered(); break;
        case 27: _t->on_lowAlpha_triggered(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData MainWindow::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject MainWindow::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_MainWindow,
      qt_meta_data_MainWindow, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MainWindow::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MainWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MainWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MainWindow))
        return static_cast<void*>(const_cast< MainWindow*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int MainWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 28)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 28;
    }
    return _id;
}
static const uint qt_meta_data_FactorDiaog[] = {

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
      13,   12,   12,   12, 0x09,

       0        // eod
};

static const char qt_meta_stringdata_FactorDiaog[] = {
    "FactorDiaog\0\0on_okButton_clicked()\0"
};

void FactorDiaog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        FactorDiaog *_t = static_cast<FactorDiaog *>(_o);
        switch (_id) {
        case 0: _t->on_okButton_clicked(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData FactorDiaog::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject FactorDiaog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_FactorDiaog,
      qt_meta_data_FactorDiaog, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &FactorDiaog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *FactorDiaog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *FactorDiaog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_FactorDiaog))
        return static_cast<void*>(const_cast< FactorDiaog*>(this));
    return QDialog::qt_metacast(_clname);
}

int FactorDiaog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}
static const uint qt_meta_data_AdjustDialog[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      14,   13,   13,   13, 0x09,
      37,   13,   13,   13, 0x09,
      66,   60,   13,   13, 0x09,
      97,   90,   13,   13, 0x09,
     117,   13,   13,   13, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_AdjustDialog[] = {
    "AdjustDialog\0\0on_decButton_clicked()\0"
    "on_addButton_clicked()\0label\0"
    "setAdjustLabel(QString)\0number\0"
    "setNumBarValue(int)\0on_enterButton_clicked()\0"
};

void AdjustDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        AdjustDialog *_t = static_cast<AdjustDialog *>(_o);
        switch (_id) {
        case 0: _t->on_decButton_clicked(); break;
        case 1: _t->on_addButton_clicked(); break;
        case 2: _t->setAdjustLabel((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 3: _t->setNumBarValue((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: _t->on_enterButton_clicked(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData AdjustDialog::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject AdjustDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_AdjustDialog,
      qt_meta_data_AdjustDialog, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &AdjustDialog::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *AdjustDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *AdjustDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_AdjustDialog))
        return static_cast<void*>(const_cast< AdjustDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int AdjustDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
