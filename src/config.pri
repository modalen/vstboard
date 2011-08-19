include(../mainconfig.pri)

vstsdk {
    DEFINES += VSTSDK
    INCLUDEPATH += $$VSTSDK_PATH \
        $$VSTSDK_PATH/public.sdk/source/vst2.x
}

scriptengine {
    DEFINES += SCRIPTENGINE
    QT += script
}

PRECOMPILED_HEADER = precomp.h
