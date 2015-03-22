TARGET = qmapcontrol
TEMPLATE = lib
QT += network
QT += gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
greaterThan(QT_MAJOR_VERSION, 4): cache()

VERSION = 0.9.7.8

DEFINES += QMAPCONTROL_LIBRARY

CONFIG(debug, debug|release) {
    DEST_FOLDER = pc/debug
} else {
    DEST_FOLDER = pc/release
}

OBJECTS_DIR = ./.obj/$${DEST_FOLDER}
MOC_DIR = ./.moc/$${DEST_FOLDER}
RCC_DIR = ./.rcc/$${DEST_FOLDER}
UI_DIR = ./.ui/$${DEST_FOLDER}
DESTDIR = ../lib/$${DEST_FOLDER}

HEADERS += curve.h \
           geometry.h \
           imagemanager.h \
           layer.h \
           layermanager.h \
           linestring.h \
           mapadapter.h \
           mapcontrol.h \
           mapnetwork.h \
           point.h \
           tilemapadapter.h \
           wmsmapadapter.h \
           circlepoint.h \
           imagepoint.h \
           gps_position.h \
           osmmapadapter.h \
           maplayer.h \
           geometrylayer.h \
           yahoomapadapter.h \
           googlemapadapter.h \
           openaerialmapadapter.h \
           fixedimageoverlay.h \
           emptymapadapter.h \
           arrowpoint.h \
           invisiblepoint.h \
           qmapcontrol_global.h

SOURCES += curve.cpp \
           geometry.cpp \
           imagemanager.cpp \
           layer.cpp \
           layermanager.cpp \
           linestring.cpp \
           mapadapter.cpp \
           mapcontrol.cpp \
           mapnetwork.cpp \
           point.cpp \
           tilemapadapter.cpp \
           wmsmapadapter.cpp \
           circlepoint.cpp \
           imagepoint.cpp \
           gps_position.cpp \
           osmmapadapter.cpp \
           maplayer.cpp \
           geometrylayer.cpp \
           yahoomapadapter.cpp \
           googlemapadapter.cpp \
           openaerialmapadapter.cpp \
           fixedimageoverlay.cpp \
           arrowpoint.cpp \
           invisiblepoint.cpp \
           emptymapadapter.cpp