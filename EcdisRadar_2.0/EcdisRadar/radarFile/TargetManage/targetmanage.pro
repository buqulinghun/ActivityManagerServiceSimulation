# -------------------------------------------------
# Project created by QtCreator 2010-03-03T17:48:28
# -------------------------------------------------
TARGET = TargetManage
TEMPLATE = lib
CONFIG += dylib
DEFINES += TARGETMANAGE_LIBRARY
SOURCES += plot.cpp \
    track.cpp \
    transform.cpp \
    math.cpp \
    radarview.cpp
HEADERS += plot.h \
    TargetManage_global.h \
    track.h \
    transform.h \
    radarview.h
OTHER_FILES += history.txt \
    readme.txt
