TEMPLATE    = subdirs

win32 {
    SUBDIRS += portaudio
}

SUBDIRS += \
    portmidi \
    common \
    vstdll \
    vstboard

win32 {
    SUBDIRS += dllLoader
}

CONFIG += ordered
