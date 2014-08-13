# pymodiocr.pro
# 8/13/2014 jichi
# Build pymodiocr.pyd
#
# Though Qt is not indispensible, shiboken could save me some time for development.
# Switch to pure PyC and get rid of QStringList if it becomes a performance bottle neck.

CONFIG += pysideplugin noqtgui
include(../../../config.pri)
include($$COMDIR/modio/modio.pri)
include($$LIBDIR/modiocr/modiocr.pri)

INCLUDEPATH += $$LIBDIR/modiocr # needed by shiboken generated code

## Libraries

QT += core
QT -= gui

INCLUDEPATH += $$PYSIDE_HOME/include/PySide/QtCore

## Sources

SRCPATH = binding/pymodiocr
INCLUDEPATH += $SRCPATH
DEPENDPATH += $SRCPATH

TEMPLATE = lib
TARGET = pymodiocr

HEADERS += \
  pymodiocr.h \
  pymodiocr_config.h \
  $$SRCPATH/pymodiocr_python.h \
  $$SRCPATH/modiocr_wrapper.h

SOURCES += \
  pymodiocr.cc \
  $$SRCPATH/pymodiocr_module_wrapper.cpp \
  $$SRCPATH/modiocr_wrapper.cpp

#!wince*: LIBS += -lshell32
#RC_FILE += modiocr.rc

OTHER_FILES += \
  typesystem_modiocr.xml \
  update_binding.cmd

# EOF
