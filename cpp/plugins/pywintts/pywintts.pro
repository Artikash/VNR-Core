# pywintts.pro
# 4/7/2013 jichi
# Build pywintts.pyd
#
# Though Qt is not indispensible, shiboken could save me some time for development.
# Switch to pure PyC and get rid of QStringList if it becomes a performance bottle neck.

CONFIG += pysideplugin noqtgui
include(../../../config.pri)
include($$LIBDIR/wintts/wintts.pri)

INCLUDEPATH += $$LIBDIR/wintts # needed by shiboken generated code

## Libraries

QT += core
QT -= gui

INCLUDEPATH += $$PYSIDE_HOME/include/PySide/QtCore

## Sources

SRCPATH = binding/pywintts
INCLUDEPATH += $SRCPATH
DEPENDPATH += $SRCPATH

TEMPLATE = lib
TARGET = pywintts

HEADERS += \
  pywintts.h \
  pywintts_config.h \
  $$SRCPATH/pywintts_python.h \
  $$SRCPATH/wintts_wrapper.h

SOURCES += \
  pywintts.cc \
  $$SRCPATH/pywintts_module_wrapper.cpp \
  $$SRCPATH/wintts_wrapper.cpp

#!wince*: LIBS += -lshell32
#RC_FILE += wintts.rc

OTHER_FILES += \
  typesystem_wintts.xml \
  update_binding.cmd

# EOF
