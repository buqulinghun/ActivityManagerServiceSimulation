# -------------------------------------------------
# Project created by QtCreator 2010-01-01T18:37:37
# -------------------------------------------------
QT -= gui
QT += network
TARGET = communicate
TEMPLATE = lib
CONFIG += dylib
DEFINES += COMMUNICATE_LIBRARY
win32:LIBS += ws2_32.lib
SOURCES += communicate.cpp \
    implcommobject.cpp
HEADERS += \
    communicate_global.h \
    communicate.h \
    implcommobject.h

OTHER_FILES += \
    Readme.txt
