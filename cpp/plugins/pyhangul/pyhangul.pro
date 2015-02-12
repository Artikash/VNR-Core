# pyhangul.pro
# 4/1/2013 jichi
# Build pyhangul.pyd
#
# Though Qt is not indispensible, it's containers could same memory.

CONFIG += pysideplugin noqtgui
include(../../../config.pri)
include($$LIBDIR/hanjaconv/hanjaconv.pri)

#INCLUDEPATH += $$LIBDIR/hanjaconv # needed by shiboken generated code

## Libraries

QT += core
QT -= gui

INCLUDEPATH += $$PYSIDE_HOME/include/PySide/QtCore

## Sources

SRCPATH = binding/pyhangul
INCLUDEPATH += $SRCPATH
DEPENDPATH += $SRCPATH

TEMPLATE = lib
TARGET = pyhangul

HEADERS += \
  hangulconv.h \
  hangulparse.h \
  pyhangul_config.h \
  $$SRCPATH/hangulhanjaconverter_wrapper.h \
  $$SRCPATH/pyhangul_python.h

SOURCES += \
  hangulconv.cc \
  hangulparse.cc \
  $$SRCPATH/hangulhanjaconverter_wrapper.cpp \
  $$SRCPATH/pyhangul_module_wrapper.cpp

#!wince*: LIBS += -lshell32
#RC_FILE += hangul.rc

OTHER_FILES += \
  typesystem_hangul.xml \
  update_binding.cmd

# EOF
