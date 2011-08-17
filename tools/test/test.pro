#-------------------------------------------------
#
# Project created by QtCreator 2011-08-17T21:10:12
#
#-------------------------------------------------

QT       += testlib

#QT       -= gui

scriptengine {
    QT += script
}

TARGET = tst_testtest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


SOURCES += tst_testtest.cpp \
    objectinfo.cpp

HEADERS += \
    objectinfo.h

DEFINES += SRCDIR=\\\"$$PWD/\\\"

#win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../common/release/ -lcommon
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../common/debug/ -lcommon
#else:unix:!symbian: LIBS += -L$$OUT_PWD/../common/ -lcommon

INCLUDEPATH += $$PWD/../../src/common
DEPENDPATH += $$PWD/../../src/common

#win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../common/release/common.lib
#else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../common/debug/common.lib
#else:unix:!symbian: PRE_TARGETDEPS += $$OUT_PWD/../common/libcommon.a




