# test.pro
# 12/2/2013

CONFIG += console noqt nocrt noeh #nosafeseh
include(../../../config.pri)
include($$LIBDIR/winseh/winseh_safe.pri)

LIBS += -luser32

# Source

TEMPLATE = app
TARGET = test

DEPENDPATH += .
INCLUDEPATH += .

SOURCES += main.cc

# EOF
