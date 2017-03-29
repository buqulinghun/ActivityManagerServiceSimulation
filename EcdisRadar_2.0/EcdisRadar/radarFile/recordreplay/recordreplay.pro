#-------------------------------------------------
#
# Project created by QtCreator 2010-04-05T21:33:47
#
#-------------------------------------------------

QT       -= gui

TARGET = recordreplay
TEMPLATE = lib
CONFIG += dylib

DEFINES += RECORDREPLAY_LIBRARY

SOURCES += recordreplay.cpp \
    implrecrep.cpp \
    osfile.cpp

HEADERS += recordreplay.h\
        recordreplay_global.h \
    implrecrep.h \
    osfile.h

OTHER_FILES += \
    Readme.txt
