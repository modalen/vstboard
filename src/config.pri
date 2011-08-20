include(../mainconfig.pri)

ringBuffers {
    DEFINES += CIRCULAR_BUFFER
}
zeroDbClipping {
    DEFINES += BUFFER_ZERODB_CLIPPING
}
simpleCables {
    DEFINES += SIMPLE_CABLES
}
debugMutex {
    DEFINES += DEBUG_MUTEX
}
debugUnusedMutex {
    DEFINES += DEUBG_MUTEX_REPORT_UNUSED
}
debugSemaphore {
    DEFINES += DEBUG_SEMAPHORE
}
debugUnusedSemaphore {
    DEFINES += DEBUG_SEMAPHORE_REPORT_UNUSED
}
debugReadWriteLock {
    DEFINES += DEBUG_READWRITELOCK
}
debugUnusedReadWriteLock {
    DEFINES += DEBUG_READWRITELOCK_REPORT_UNUSED
}
vstsdk {
    DEFINES += VSTSDK
    INCLUDEPATH += $$VSTSDK_PATH \
        $$VSTSDK_PATH/public.sdk/source/vst2.x
}
scriptengine {
    DEFINES += SCRIPTENGINE
    QT += script
}
