# avtest.pro
# 10/27/2014

include(../../../config.pri)

QT += core gui
LIBS += -lavcodec -lavformat

# Source

TEMPLATE = app
TARGET = avtest

DEPENDPATH += .
INCLUDEPATH += .

HEADERS += main.h
SOURCES += main.cc

# EOF
