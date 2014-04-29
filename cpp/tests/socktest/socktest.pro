# socktest.pro
# 4/29/2014

CONFIG += nogui
include(../../../config.pri)
include($$LIBDIR/qtsocketsvc/qtsocketcli.pri)
include($$LIBDIR/qtsocketsvc/qtsocketpack.pri)

# Source

TEMPLATE = app
TARGET = socktest

DEPENDPATH += .
INCLUDEPATH += .

SOURCES += main.cc

# EOF
