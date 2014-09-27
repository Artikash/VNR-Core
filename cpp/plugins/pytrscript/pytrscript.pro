# pytrscript.pro
# 9/21/2014 jichi
# Build pytrscript.pyd
#
# Though Qt is not indispensible, shiboken could save me some time for development.
# Switch to pure PyC and get rid of QStringList if it becomes a performance bottle neck.

#CONFIG += pysideplugin noqtgui
CONFIG += pysideplugin noqt
include(../../../config.pri)
include($$LIBDIR/trscript/trscript.pri)

INCLUDEPATH += $$LIBDIR/trscript # needed by shiboken generated code

## Libraries

QT -= core
QT -= gui

INCLUDEPATH += $$PYSIDE_HOME/include/PySide/QtCore

## Sources

SRCPATH = binding/pytrscript
INCLUDEPATH += $SRCPATH
DEPENDPATH += $SRCPATH

TEMPLATE = lib
TARGET = pytrscript

HEADERS += \
  pytrscript_config.h \
  $$SRCPATH/pytrscript_python.h \
  $$SRCPATH/translationscriptmanager_wrapper.h

SOURCES += \
  $$SRCPATH/pytrscript_module_wrapper.cpp \
  $$SRCPATH/translationscriptmanager_wrapper.cpp

#!wince*: LIBS += -lshell32
#RC_FILE += trscript.rc

OTHER_FILES += \
  typesystem_trscript.xml \
  update_binding.cmd

# EOF
