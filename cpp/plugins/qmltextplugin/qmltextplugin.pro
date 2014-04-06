# qmltexscriptplugin.pro
# 11/12/2011 jichi
# Build qml texscript plugin

CONFIG += qmlplugin
include(../../../config.pri)
include($$LIBDIR/imageutil/imageutil.pri)
include($$LIBDIR/qmltext/qmltext.pri)

## Sources

TEMPLATE = lib
TARGET  = qmltextplugin

HEADERS += qmltextplugin.h
SOURCES += qmltextplugin.cc

QT += core gui declarative

#!wince*: LIBS += -lshell32
#RC_FILE += qmltextplugin.rc

OTHER_FILES += qmldir

# EOF
