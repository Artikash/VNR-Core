# ttstest.pro
# 6/15/2014

CONFIG += console noqt
include(../../../config.pri)

# Source

TEMPLATE = app
TARGET = dlltest

DEPENDPATH += .
INCLUDEPATH += .

#HEADERS += main.h
SOURCES += main.cc

# EOF
