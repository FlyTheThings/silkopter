TEMPLATE = lib
CONFIG += staticlib
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += c++11
INCLUDEPATH += ../../include

PRECOMPILED_HEADER = ../../include/QBaseStdAfx.h
CONFIG *= precompile_header

#QMAKE_CXXFLAGS_RELEASE += -g
#QMAKE_CFLAGS_RELEASE += -g
#QMAKE_LFLAGS_RELEASE =

QMAKE_CXXFLAGS += -Wno-unused-variable
QMAKE_CFLAGS += -Wno-unused-variable

rpi {
    DEFINES+=RASPBERRY_PI
    QMAKE_MAKEFILE = "Makefile.rpi"
    MAKEFILE = "Makefile.rpi"
    CONFIG(debug, debug|release) {
        DEST_FOLDER = rpi/debug
    }
    CONFIG(release, debug|release) {
        DEST_FOLDER = rpi/release
        DEFINES += NDEBUG
    }
} else {
    QMAKE_MAKEFILE = "Makefile"
    CONFIG(debug, debug|release) {
        DEST_FOLDER = pc/debug
    }
    CONFIG(release, debug|release) {
        DEST_FOLDER = pc/release
        DEFINES += NDEBUG
    }
}

OBJECTS_DIR = ./.obj/$${DEST_FOLDER}
MOC_DIR = ./.moc/$${DEST_FOLDER}
RCC_DIR = ./.rcc/$${DEST_FOLDER}
UI_DIR = ./.ui/$${DEST_FOLDER}
DESTDIR = ../../lib/$${DEST_FOLDER}

SOURCES += \
    ../../src/Clock.cpp \
    ../../src/Console_Logger.cpp \
    ../../src/Debug.cpp \
    ../../src/Interned_String.cpp \
    ../../src/Interned_String_DB.cpp \
    ../../src/Log.cpp \
    ../../src/Path.cpp \
    ../../src/Profiler.cpp \
    ../../src/thread/Scheduler.cpp \
    ../../src/thread/Task.cpp \
    ../../src/thread/Worker_Thread.cpp \
    ../../src/util/Class_Factory.cpp \
    ../../src/util/Encoder.cpp \
    ../../src/util/File_System_Utils.cpp \
    ../../src/util/Format.cpp \
    ../../src/util/Rand.cpp \
    ../../src/util/Util.cpp \
    ../../include/util/Iterators.inl

HEADERS += \
    ../../include/Clock.h \
    ../../include/Console_Logger.h \
    ../../include/Debug.h \
    ../../include/FString.h \
    ../../include/Interned_String.h \
    ../../include/Interned_String_DB.h \
    ../../include/Log.h \
    ../../include/Logger.h \
    ../../include/Path.h \
    ../../include/Platform.h \
    ../../include/Profiler.h \
    ../../include/QBase.h \
    ../../include/QBaseStdAfx.h \
    ../../include/Types.h \
    ../../include/mem/auto_buffer.h \
    ../../include/mem/Embedded_Allocator.h \
    ../../include/mem/FSBAllocator.hh \
    ../../include/thread/Scheduler.h \
    ../../include/thread/Task.h \
    ../../include/thread/Worker_Thread.h \
    ../../include/util/Circular_Buffer.h \
    ../../include/util/Class_Factory.h \
    ../../include/util/crc.h \
    ../../include/util/Delayer.h \
    ../../include/util/Encoder.h \
    ../../include/util/File_System_Utils.h \
    ../../include/util/Flag_Set.h \
    ../../include/util/Format.h \
    ../../include/util/Hash.h \
    ../../include/util/Iterators.h \
    ../../include/util/murmurhash.h \
    ../../include/util/Noncopyable.h \
    ../../include/util/qrtti.h \
    ../../include/util/Rand.h \
    ../../include/util/Signals.h \
    ../../include/util/Singleton.h \
    ../../include/util/Tween.h \
    ../../include/util/Util.h

