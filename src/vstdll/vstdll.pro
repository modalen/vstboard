include(../config.pri)

QT += core gui

TARGET = "VstBoardPlugin"
TEMPLATE = lib

LIBS += -L"C:/Program Files (x86)/Visual Leak Detector/lib/" -lvld
INCLUDEPATH += "C:/Program Files (x86)/Visual Leak Detector/include/"

CONFIG += qtwinmigrate-uselib
include($${_PRO_FILE_PWD_}/../../libs/qtwinmigrate/src/qtwinmigrate.pri)

win32 {
    LIBS += -lwinmm
    LIBS += -ladvapi32
    LIBS += -lws2_32
}

win32-g++ {
    DEFINES += WINVER=0x0501
    DEFINES += _WIN32_WINNT=0x0501
    QMAKE_LFLAGS += -static-libgcc
}

win32-msvc* {
    DEFINES += _CRT_SECURE_NO_WARNINGS
    RC_FILE = vstdll.rc
}

INCLUDEPATH += ../common

SOURCES += \
    main.cpp \
    gui.cpp \
    vst.cpp \
    mainwindowvst.cpp \
    mainhostvst.cpp \
    resizehandle.cpp \
    connectables/vstaudiodevicein.cpp \
    connectables/vstaudiodeviceout.cpp \
    connectables/vstautomation.cpp \
    connectables/vstmididevice.cpp \
    connectables/objectfactoryvst.cpp \
    views/configdialogvst.cpp


HEADERS  += \
    gui.h \
    vst.h \
    mainwindowvst.h \
    mainhostvst.h \
    resizehandle.h \
    connectables/vstaudiodevicein.h \
    connectables/vstaudiodeviceout.h \
    connectables/vstautomation.h \
    connectables/vstmididevice.h \
    connectables/objectfactoryvst.h \
    views/configdialogvst.h

RESOURCES += ../resources/resources.qrc

TRANSLATIONS = ../resources/translations/vstdll_fr.ts

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../common/release/ -lcommon
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../common/debug/ -lcommon
else:unix:!symbian: LIBS += -L$$OUT_PWD/../common/ -lcommon

INCLUDEPATH += $$PWD/../common
DEPENDPATH += $$PWD/../common

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../common/release/common.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../common/debug/common.lib
else:unix:!symbian: PRE_TARGETDEPS += $$OUT_PWD/../common/libcommon.a

PRECOMPILED_HEADER = precomp.h
