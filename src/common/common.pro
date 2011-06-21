srcdir      = common
include(../config.pri)

QT += core gui script

TEMPLATE = lib
CONFIG += staticlib

DEFINES += APP_NAME=\\\"VstBoardCommon\\\"

vstsdk {
    HEADERS += vst/cvsthost.h \
        connectables/vstplugin.h \
        vst/ceffect.h \
        vst/const.h \
        vst/vstbank.h \
        views/vstpluginwindow.h \
        views/vstshellselect.h

    SOURCES += vst/cvsthost.cpp \
        connectables/vstplugin.cpp \
        vst/ceffect.cpp \
        vst/vstbank.cpp \
        views/vstpluginwindow.cpp \
        views/vstshellselect.cpp \
        $$VSTSDK_PATH/public.sdk/source/vst2.x/audioeffectx.cpp \
        $$VSTSDK_PATH/public.sdk/source/vst2.x/audioeffect.cpp

    FORMS += views/vstpluginwindow.ui \
        views/vstshellselect.ui
}

SOURCES += \
    mainhost.cpp \
    views/connectablepinview.cpp \
    connectables/midipinin.cpp \
    connectables/midipinout.cpp \
    connectables/audiopinout.cpp \
    connectables/audiopinin.cpp \
    views/objectview.cpp \
    views/pinview.cpp \
    connectables/objectprogram.cpp \
    connectables/containerprogram.cpp \
    renderer/solvernode.cpp \
    audiobuffer.cpp \
    connectables/bridgepinin.cpp \
    connectables/bridgepinout.cpp \
    connectables/cable.cpp \
    connectables/objectparameter.cpp \
    views/sceneview.cpp \
    projectfile/setupfile.cpp \
    projectfile/projectfile.cpp \
    models/listtoolsmodel.cpp \
    views/cableview.cpp \
    views/maincontainerview.cpp \
    views/connectableobjectview.cpp \
    views/bridgepinview.cpp \
    views/containercontent.cpp \
    views/bridgeview.cpp \
    views/listpinsview.cpp \
    views/programlist.cpp \
    views/filebrowser.cpp \
    views/grouplistview.cpp \
    views/proglistview.cpp \
    connectables/objectfactory.cpp \
    connectables/object.cpp \
    connectables/container.cpp \
    connectables/maincontainer.cpp \
    renderer/pathsolver.cpp \
    renderer/renderer.cpp \
    models/hostmodel.cpp \
    programs.cpp \
    connectables/connectioninfo.cpp \
    connectables/objectinfo.cpp \
    connectables/pin.cpp \
    connectables/parameterpin.cpp \
    connectables/parameterpinin.cpp \
    connectables/parameterpinout.cpp \
    models/programsmodel.cpp \
    connectables/bridge.cpp \
    views/aboutdialog.cpp \
    connectables/miditoautomation.cpp \
    connectables/midisender.cpp \
    connectables/pinslist.cpp \
    views/minmaxpinview.cpp \
    views/cursorview.cpp \
    mainwindow.cpp \
    views/configdialog.cpp \
    views/maingraphicsview.cpp \
    connectables/hostcontroller.cpp \
    audiobufferd.cpp \
    connectables/script.cpp \
    views/splash.cpp \
    connectables/audiopin.cpp \
    views/vstpluginview.cpp \
    vst/vstbankbase.cpp \
    vst/vstprogram.cpp \
    views/viewconfigdialog.cpp \
    views/viewconfig.cpp \
    renderer/renderthread.cpp \
    renderer/optimizerstep.cpp \
    renderer/optimizer.cpp \
    renderer/optimizestepthread.cpp \
    renderer/renderernode.cpp \
    renderer/node.cpp \
    views/listaudiodevicesview.cpp \
    views/scripteditor.cpp

HEADERS  += \
    globals.h \
    mainhost.h \
    connectables/midisender.h \
    connectables/miditoautomation.h \
    connectables/hostcontroller.h \
    connectables/midipinout.h \
    connectables/audiopinout.h \
    connectables/audiopinin.h \
    views/configdialog.h \
    views/objectview.h \
    views/pinview.h \
    views/connectablepinview.h \
    connectables/objectprogram.h \
    connectables/containerprogram.h \
    renderer/solvernode.h \
    audiobuffer.h \
    connectables/bridgepinin.h \
    connectables/bridgepinout.h \
    connectables/cable.h \
    connectables/objectparameter.h \
    views/sceneview.h \
    projectfile/setupfile.h \
    projectfile/projectfile.h \
    models/listtoolsmodel.h \
    views/cableview.h \
    views/maincontainerview.h \
    views/connectableobjectview.h \
    views/bridgepinview.h \
    views/containercontent.h \
    views/bridgeview.h \
    views/listpinsview.h \
    views/programlist.h \
    views/filebrowser.h \
    views/grouplistview.h \
    views/proglistview.h \
    connectables/objectfactory.h \
    connectables/object.h \
    connectables/container.h \
    connectables/maincontainer.h \
    renderer/pathsolver.h \
    renderer/renderer.h \
    models/hostmodel.h \
    programs.h \
    connectables/connectioninfo.h \
    connectables/objectinfo.h \
    connectables/pin.h \
    connectables/parameterpin.h \
    connectables/parameterpinin.h \
    connectables/parameterpinout.h \
    models/programsmodel.h \
    connectables/bridge.h \
    views/aboutdialog.h \
    _version.h \
    connectables/midipinin.h \
    connectables/pinslist.h \
    views/minmaxpinview.h \
    views/cursorview.h \
    mainwindow.h \
    views/maingraphicsview.h \
    audiobufferd.h \
    connectables/script.h \
    views/splash.h \
    connectables/audiopin.h \
    views/vstpluginview.h \
    vst/vstbankbase.h \
    vst/vstprogram.h \
    views/viewconfigdialog.h \
    views/viewconfig.h \
    renderer/renderthread.h \
    renderer/optimizerstep.h \
    renderer/optimizer.h \
    renderer/optimizestepthread.h \
    renderer/renderernode.h \
    renderer/node.h \
    views/listaudiodevicesview.h \
    views/scripteditor.h

FORMS += \
    views/configdialog.ui \
    mainwindow.ui \
    views/filebrowser.ui \
    views/programlist.ui \
    views/aboutdialog.ui \
    views/splash.ui \
    views/viewconfigdialog.ui \
    views/scripteditor.ui

PRECOMPILED_HEADER = precomp.h

TRANSLATIONS = ../resources/translations/common_fr.ts

RESOURCES += \
    ../resources/resources.qrc
