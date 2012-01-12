include(../config.pri)

QT += core gui

QMAKE_LFLAGS+="/DEF:$${_PRO_FILE_PWD_}/vstboard.def"

TARGET = "VstBoardPlugin"
TEMPLATE = lib

#CONFIG(debug, debug|release) {
#    LIBS += -L"C:/Program Files (x86)/Visual Leak Detector/lib/" -lvld
#    INCLUDEPATH += "C:/Program Files (x86)/Visual Leak Detector/include/"
#}

win32 {
    LIBS += -lwinmm
    LIBS += -ladvapi32
    LIBS += -lws2_32
    LIBS += -lole32
}

win32-g++ {
    QMAKE_LFLAGS += -static-libgcc
}

win32-msvc* {
    DEFINES += _CRT_SECURE_NO_WARNINGS
    RC_FILE = vstdll.rc
}

INCLUDEPATH += ../common

CONFIG += qtwinmigrate-uselib
include($${_PRO_FILE_PWD_}/../../libs/qtwinmigrate/src/qtwinmigrate.pri)

scriptengine {
    QT += script
}

SOURCES += \
    main.cpp \
    gui.cpp \
    connectables/vstaudiodevicein.cpp \
    connectables/vstaudiodeviceout.cpp \
    vst.cpp \
    connectables/vstautomation.cpp \
    connectables/vstmididevice.cpp \
    mainwindowvst.cpp \
    connectables/objectfactoryvst.cpp \
    views/configdialogvst.cpp \
    resizehandle.cpp \
    factory.cpp \
    vstboardcontroller.cpp \
    vstboardprocessor.cpp \
    vsttest.cpp \
    ../../libs/VST3SDK/base/source/fobject.cpp \
    ../../libs/VST3SDK/base/source/fstring.cpp \
    ../../libs/VST3SDK/base/source/fatomic.cpp \
    ../../libs/VST3SDK/base/source/updatehandler.cpp \
    ../../libs/VST3SDK/base/source/fdebug.cpp \
    ../../libs/VST3SDK/base/source/fthread.cpp \
    ../../libs/VST3SDK/base/source/baseiids.cpp \
    ../../libs/VST3SDK/pluginterfaces/base/ustring.cpp \
    ../../libs/VST3SDK/pluginterfaces/base/funknown.cpp \
    ../../libs/VST3SDK/pluginterfaces/base/conststringtable.cpp \
    ../../libs/VST3SDK/public.sdk/source/vst/vstaudioeffect.cpp \
    ../../libs/VST3SDK/public.sdk/source/vst/vstcomponent.cpp \
    ../../libs/VST3SDK/public.sdk/source/vst/vstcomponentbase.cpp \
    ../../libs/VST3SDK/public.sdk/source/vst/vstparameters.cpp \
    ../../libs/VST3SDK/public.sdk/source/main/pluginfactoryvst3.cpp \
    ../../libs/VST3SDK/public.sdk/source/vst/vstbus.cpp \
    ../../libs/VST3SDK/public.sdk/source/common/pluginview.cpp \
    ../../libs/VST3SDK/public.sdk/source/vst/vsteditcontroller.cpp \
    ../../libs/VST3SDK/public.sdk/source/vst/vstinitiids.cpp


HEADERS  += \
    gui.h \
    connectables/vstaudiodevicein.h \
    connectables/vstaudiodeviceout.h \
    vst.h \
    connectables/vstautomation.h \
    connectables/vstmididevice.h \
    mainwindowvst.h \
    connectables/objectfactoryvst.h \
    views/configdialogvst.h \
    resizehandle.h \
    ids.h \
    vstboardcontroller.h \
    vstboardprocessor.h \
    vsttest.h


PRECOMPILED_HEADER = ../common/precomp.h

RESOURCES += ../resources/resources.qrc

TRANSLATIONS = ../resources/translations/vstdll_fr.ts

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../common/release/ -lcommon
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../common/debug/ -lcommon
else:unix:!symbian: LIBS += -L$$OUT_PWD/../common/ -lcommon

INCLUDEPATH += $$PWD/../common
DEPENDPATH += $$PWD/../common

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../common/release/libcommon.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../common/debug/libcommon.a
else:win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../common/release/common.lib
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$OUT_PWD/../common/debug/common.lib
else:unix:!symbian: PRE_TARGETDEPS += $$OUT_PWD/../common/libcommon.a

OTHER_FILES += \
    vstboard.def
































