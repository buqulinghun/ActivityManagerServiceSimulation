# -------------------------------------------------
# Project created by QtCreator 2016-01-10T16:26:28
# -------------------------------------------------
QT += network \
    opengl \
    svg \
    xml \
    xmlpatterns\
    sql
TARGET = Ecdis_2
TEMPLATE = app
SOURCES += main.cpp \
    mainwindow.cpp \
    s57/catalog.cpp \
    s57/connectednodevector.cpp \
    s57/dsid.cpp \
    s57/dspm.cpp \
    s57/edgevector.cpp \
    s57/feature.cpp \
    s57/isolatednodevector.cpp \
    s57/s57.cpp \
    s57/s57chart.cpp \
    iso8211lib/cpl_conv.cpp \
    iso8211lib/cpl_error.cpp \
    iso8211lib/cpl_string.cpp \
    iso8211lib/cpl_vsisimple.cpp \
    iso8211lib/ddffield.cpp \
    iso8211lib/ddffielddefn.cpp \
    iso8211lib/ddfmodule.cpp \
    iso8211lib/ddfrecord.cpp \
    iso8211lib/ddfsubfielddefn.cpp \
    iso8211lib/ddfutils.cpp \
    areaobject.cpp \
    chart.cpp \
    configuration.cpp \
    filepath.cpp \
    lineobject.cpp \
    longlat.cpp \
    pointobject.cpp \
    waypoint.cpp \
    myview.cpp \
    myscene.cpp \
    symbols.cpp \
    mysvgitem.cpp \
    mercatorproj.cpp \
    mycomplexlineitem.cpp \
    soundobject.cpp \
    sqlite3/sqlite3.c \
    database.cpp \
    recorder.cpp \
    mydialog.cpp \
    conditionalitem.cpp \
    transform.cpp \
    math.cpp \
    radarFile/dataprocess.cpp \
    radarFile/interact.cpp \
    radarFile/communicate/implserial.cpp \
    radarFile/communicate/implcommobject.cpp \
    radarFile/communicate/communicate.cpp \
    radarFile/qextserialport-1.2win-alpha/qextserialport.cpp \
    radarFile/qextserialport-1.2win-alpha/qextserialenumerator.cpp \
    radarFile/qextserialport-1.2win-alpha/qextserialbase.cpp \
    radarFile/qextserialport-1.2win-alpha/posix_qextserialport.cpp \
    radarFile/radaritem.cpp \
    spaceobject.cpp
HEADERS += mainwindow.h \
    s57/catalog.h \
    s57/connectednodevector.h \
    s57/dsid.h \
    s57/dspm.h \
    s57/edgevector.h \
    s57/feature.h \
    s57/isolatednodevector.h \
    s57/s57.h \
    s57/s57chart.h \
    iso8211lib/cpl_config.h \
    iso8211lib/cpl_conv.h \
    iso8211lib/cpl_error.h \
    iso8211lib/cpl_port.h \
    iso8211lib/cpl_string.h \
    iso8211lib/cpl_vsi.h \
    iso8211lib/iso8211.h \
    areaobject.h \
    chart.h \
    configuration.h \
    filepath.h \
    lineobject.h \
    longlat.h \
    pointobject.h \
    waypoint.h \
    myview.h \
    myscene.h \
    symbols.h \
    mysvgitem.h \
    mercatorproj.h \
    mycomplexlineitem.h \
    ecdis.h \
    soundobject.h \
    sqlite3/sqlite3.h \
    sqlite3/sqlite3ext.h \
    database.h \
    recorder.h \
    mydialog.h \
    conditionalitem.h \
    TargetManage_global.h \
    transform.h \
    radarFile/dataprocess.h \
    radarFile/interact.h \
    radarFile/communicate/implcommobject.h \
    radarFile/communicate/communicate.h \
    radarFile/communicate/communicate_global.h \
    radarFile/define.h \
    radarFile/qextserialport-1.2win-alpha/qextserialport.h \
    radarFile/qextserialport-1.2win-alpha/qextserialenumerator.h \
    radarFile/qextserialport-1.2win-alpha/qextserialbase.h \
    radarFile/qextserialport-1.2win-alpha/posix_qextserialport.h \
    radarFile/radaritem.h \
    spaceobject.h

FORMS += mainwindow.ui \
    factorDialog.ui \
    adjustDialog.ui

OTHER_FILES += \
    radarFile/communicate.cfg
