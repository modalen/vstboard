PORTAUDIO_PATH 	= ../../libs/portaudio
PORTMIDI_PATH 	= ../../libs/portmidi

win32|macx {
    VSTSDK_PATH	= ../../libs/vstsdk2.4
    CONFIG += vstsdk
    DEFINES += VSTSDK
    INCLUDEPATH += $$VSTSDK_PATH \
        $$VSTSDK_PATH/public.sdk/source/vst2.x
}

DEFINES += SCRIPTENGINE

CONFIG(debug, debug|release) {
    POST =
    build_postfix=debug
    DEFINES += DEBUG
} else {
    POST =
    build_postfix=release
}

win32-g++ {
    DEFINES += WINVER=0x0501
    DEFINES += _WIN32_WINNT=0x0501
 #   LIBS += -L$$quote($$MINGW_PATH/lib)
 #   INCLUDEPATH += $$quote($$MINGW_PATH/include)
}

win32-msvc* {
    DEFINES += _CRT_SECURE_NO_WARNINGS
    INCLUDEPATH += $$quote($$(INCLUDE))
    LIBS += -L$$quote($$(LIB))
 #   QMAKE_CFLAGS += -Fd$$top_destdir/$$TARGET

#to add symbols :
#    QMAKE_CFLAGS_RELEASE +=  -Zi
#    QMAKE_LFLAGS_RELEASE += /DEBUG

}
