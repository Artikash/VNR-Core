# mousehook.pro
# 11/26/2011

CONFIG += dll
include(../../../config.pri)

## Libraries

QT += core gui

## Sources

TEMPLATE = lib
TARGET = mousehook

DEFINES += MOUSEHOOK_LIB
DEFINES += MOUSEHOOK_DLL_NAME=L\\\"$$TARGET\\\"

HEADERS = \
  mousehook_config.h \
  mousehook.h

SOURCES = \
  mousehook.cc

#!wince*: LIBS += -lshell32
#RC_FILE += mousehook.rc

OTHER_FILES += \
  mousehook.pri \
  mousehook.rc

# EOF
