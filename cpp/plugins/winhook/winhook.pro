# winhook.pro
# 1/22/2013 jichi

CONFIG += noqtgui dll
include(../../../config.pri)
include($$LIBDIR/libqxt/libqxt.pri)
include($$LIBDIR/metacall/metacall.pri)
include($$LIBDIR/qtmodule/qtmodule.pri)
include($$LIBDIR/sakurakit/sakurakit.pri)
include($$LIBDIR/singleapp/singleapp.pri)
include($$LIBDIR/wintimer/wintimer.pri)

#include($$LIBDIR/disasm/disasm.pri)
#include($$LIBDIR/winhook/winhook.pri)

# Modules
HEADERS += $$MODULEDIR/reader/metacall.h

## Libraries

#CONFIG  += noqt
QT      += core network
QT      -= gui

#INCLUDEPATH += $$D3D_HOME/include
#LIBS    += -ld3d9 -L$$D3D_HOME/lib/x86

INCLUDEPATH += $$DETOURS_HOME/include
LIBS    += -ldetours -L$$DETOURS_HOME/lib

LIBS        += -L$$WDK_HOME/lib/wxp/i386 -lntdll

LIBS    += -luser32 -lpsapi
LIBS    += -lgdi32

#QMAKE_LFLAGS += /NODEFAULTLIB:msvcrt.lib /NODEFAULTLIB:msvcrtd.lib
#DEFINES += _CRT_SECURE_NO_WARNINGS _CRT_NON_CONFORMING_SWPRINTFS

## Sources

TEMPLATE = lib
TARGET  = winhook

HEADERS += \
  main.h \
  main_p.h \
  myfunc.h \
  myfunc_p.h \
  growl.h
SOURCES += \
  main.cc \
  main_p.cc \
  myfunc.cc \
  growl.cc

DEPENDPATH += util
INCLUDEPATH += util
HEADERS += \
  util/wingui.h \
  util/winsec.h
SOURCES += \
  util/wingui.cc \
  util/winsec.cc

DEPENDPATH += qt
HEADERS += \
  qt/dataman.h \
  qt/mainobj.h \
  qt/mainobj_p.h \
  qt/mydef.h \
  qt/myhash.h \
  qt/rpccli.h \
  qt/rpccli_p.h
SOURCES += \
  qt/dataman.cc \
  qt/mainobj.cc \
  qt/rpccli.cc

DEPENDPATH += hijack
HEADERS += \
  hijack/majiro.h \
  hijack/majiro_p.h
SOURCES += \
  hijack/majiro.cc

#!wince*: LIBS += -lshell32
#RC_FILE += winhook.rc

OTHER_FILES += winhook.rc

# EOF
