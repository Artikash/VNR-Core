# winhook.pro
# 1/22/2013 jichi

CONFIG += noqtgui dll
include(../../../config.pri)
include($$LIBDIR/detoursutil/detoursutil.pri)
include($$LIBDIR/libqxt/libqxt.pri)
include($$LIBDIR/memdbg/memdbg.pri)
include($$LIBDIR/ntinspect/ntinspect.pri)
include($$LIBDIR/qtembedded/qtembedded.pri)
include($$LIBDIR/qtmetacall/qtmetacall.pri)
include($$LIBDIR/sakurakit/sakurakit.pri)
include($$LIBDIR/singleapp/singleapp.pri)
include($$LIBDIR/windbg/windbg.pri)
include($$LIBDIR/winquery/winquery.pri)
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

LIBS    += -luser32 -lpsapi -lgdi32

## Sources

TEMPLATE = lib
TARGET  = winhook

#DEPENDPATH += driver engine game ui

HEADERS += \
  driver/mainobj.h \
  driver/mainobj_p.h \
  driver/rpccli.h \
  driver/rpccli_p.h
  ui/uidef.h \
  ui/uihash.h \
  ui/uihijack.h \
  ui/uihijack_p.h \
  ui/uitextman.h \
  main.h \
  main_p.h \
  growl.h
SOURCES += \
  driver/mainobj.cc \
  driver/rpccli.cc \
  ui/uihijack.cc \
  ui/uitextman.cc \
  main.cc \
  main_p.cc \
  growl.cc

HEADERS += \
  engine/majiro.h \
  engine/majiro_p.h
SOURCES += \
  engine/majiro.cc

#!wince*: LIBS += -lshell32
#RC_FILE += winhook.rc

OTHER_FILES += winhook.rc

# EOF
