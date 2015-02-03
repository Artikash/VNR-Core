# pyhanviet.pro
# 2/2/2015 jichi
# Build pyhanviet.pyd
#
# Though Qt is not indispensible, it's containers could same memory.

CONFIG += pyplugin shiboken noqt
include(../../../config.pri)
include($$LIBDIR/hanviet/hanviet.pri)

INCLUDEPATH += $$LIBDIR/hanviet # needed by shiboken generated code

## Libraries

QT += core
QT -= gui

INCLUDEPATH += $$PYSIDE_HOME/include/PySide/QtCore

## Sources

SRCPATH = binding/pyhanviet
INCLUDEPATH += $SRCPATH
DEPENDPATH += $SRCPATH

TEMPLATE = lib
TARGET = pyhanviet

HEADERS += \
  pyhanviet_config.h \
  $$SRCPATH/hanviettranslator_wrapper..h \
  $$SRCPATH/pyhanviet_python.h

SOURCES += \
  $$SRCPATH/hanviettranslator_wrapper.cpp \
  $$SRCPATH/pyhanviet_module_wrapper.cpp

#!wince*: LIBS += -lshell32
#RC_FILE += hanviet.rc

OTHER_FILES += \
  typesystem_hanviet.xml \
  update_binding.cmd

# EOF
