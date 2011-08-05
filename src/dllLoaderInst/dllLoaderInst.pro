#-------------------------------------------------
#
# Project created by QtCreator 2011-02-03T20:52:43
#
#-------------------------------------------------

include(../config.pri)

DEFINES += AS_INSTRUMENT
TARGET = VstBoardInstrument
TEMPLATE = lib

QT       -= core gui
LIBS += -ladvapi32
LIBS += -luser32

SOURCES += \
    ../dllLoader/main.cpp

HEADERS +=

win32-msvc* {
    RC_FILE = ../dllLoader/dllLoader.rc
}


