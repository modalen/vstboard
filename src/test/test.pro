#-------------------------------------------------
#
# Project created by QtCreator 2011-08-17T21:10:12
#
#-------------------------------------------------
include(../config.pri)

QT       += testlib
#QT       -= gui

TARGET = test_vstboard
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

#win32 {
#    LIBS += -lwinmm
#    LIBS += -luser32
#    LIBS += -ladvapi32
#    LIBS += -lole32
#    LIBS += -lsetupapi
#    LIBS += -ldsound
#}

CONFIG += qtwinmigrate-uselib
include($${_PRO_FILE_PWD_}/../../libs/qtwinmigrate/src/qtwinmigrate.pri)

SOURCES += \
    vst.cpp \
    gui.cpp \
    mainwindowvst.cpp \
    test_metadata.cpp \
    test_metainfo.cpp \
    test_mainhost.cpp \
    mainhostvst.cpp \
    resizehandle.cpp \
    connectables/objectfactoryvst.cpp \
    connectables/vstmididevice.cpp \
    connectables/vstautomation.cpp \
    connectables/vstaudiodeviceout.cpp \
    connectables/vstaudiodevicein.cpp \
    views/configdialogvst.cpp


HEADERS += \
    vst.h \
    gui.h \
    mainwindowvst.h \
    mainhostvst.h \
    resizehandle.h \
    connectables/objectfactoryvst.h \
    connectables/vstmididevice.h \
    connectables/vstautomation.h \
    connectables/vstaudiodeviceout.h \
    connectables/vstaudiodevicein.h \
    views/configdialogvst.h


win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../common/release/ -lcommon
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../common/debug/ -lcommon
else:unix:!symbian: LIBS += -L$$OUT_PWD/../common/ -lcommon

INCLUDEPATH += $$PWD/../common
DEPENDPATH += $$PWD/../common
INCLUDEPATH += $$PWD/../vstdll
DEPENDPATH += $$PWD/../vstdll

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../common/release/common.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../common/debug/common.lib
else:unix:!symbian: PRE_TARGETDEPS += $$OUT_PWD/../common/libcommon.a







