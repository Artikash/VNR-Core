# tahscript.pro
# 8/14/2014 jichi
# Build tahscript.dll
# See: http://www.hongfire.com/forum/showthread.php/94395-Translation-Aggregator-v0-4-9/page51?p=2269439#post2269439
# See: http://www.hongfire.com/forum/showthread.php/94395-Translation-Aggregator-v0-4-9?p=1811363#post1811363

CONFIG += noqtgui dll
include(../../../config.pri)

## Libraries

QT += core
QT -= gui

## Sources

TEMPLATE = lib
TARGET  = tahscript

#CONFIG += staticlib
DEFINES += TAHSCRIPT_BUILD_LIB

HEADERS += \
  ihf_p.h \
  ith_p.h \

SOURCES += \
  ihf_p.cc \
  ith_p.cc \
  texthook.cc \
  textthread_p.cc \
  winapi_p.cc

#!wince*: LIBS += -lshell32
#RC_FILE += tahscript.rc

OTHER_FILES += \
  tahscript.pri \
  tahscript.rc

# EOF
