TEMPLATE    = subdirs

mac {
    CONFIG += x86 ppc x86_64 ppc64
}

win32 {
    SUBDIRS += portaudio
}

SUBDIRS += \
    portmidi \
    common \
    vstboard

win32 {
    SUBDIRS += vstdll \
                dllLoader
}

CONFIG += ordered
