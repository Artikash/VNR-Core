# vnragent.pro
# 1/22/2013 jichi

CONFIG += noqtgui dll
include(../../../config.pri)
include($$LIBDIR/detoursutil/detoursutil.pri)
include($$LIBDIR/libqxt/libqxt.pri)
include($$LIBDIR/memdbg/memdbg.pri)
include($$LIBDIR/ntinspect/ntinspect.pri)
#include($$LIBDIR/qtembedded/qtembedded.pri) # needed by app runner
#include($$LIBDIR/qtmetacall/qtmetacall.pri)
include($$LIBDIR/qtjson/qtjson.pri)
include($$LIBDIR/qtsocketsvc/qtsocketpack.pri)
include($$LIBDIR/qtsocketsvc/qtsocketpipe.pri)
include($$LIBDIR/qtsocketsvc/qtlocalcli.pri)
#include($$LIBDIR/qtsocketsvc/qttcpcli.pri)
include($$LIBDIR/sakurakit/sakurakit.pri)
include($$LIBDIR/vnrsharedmemory/vnrsharedmemory.pri)
include($$LIBDIR/windbg/windbg.pri)
#include($$LIBDIR/winevent/winevent.pri)
include($$LIBDIR/winkey/winkey.pri)
include($$LIBDIR/winiter/winiter.pri)
include($$LIBDIR/winquery/winquery.pri)
#include($$LIBDIR/wintimer/wintimer.pri)
include($$LIBDIR/winmutex/winmutex.pri)
include($$LIBDIR/winsinglemutex/winsinglemutex.pri)

#include($$LIBDIR/disasm/disasm.pri)
#include($$LIBDIR/vnragent/vnragent.pri)

# Services
#HEADERS += $$SERVICEDIR/reader/metacall.h

## Libraries

#CONFIG  += noqt
QT      += core network
QT      -= gui

#INCLUDEPATH += $$D3D_HOME/include
#LIBS    += -ld3d9 -L$$D3D_HOME/lib/x86

#LIBS    += -lkernel32
#LIBS    += -luser32 -lpsapi
LIBS    += -lgdi32   # needed by game engines
LIBS    += -lshell32 # needed by get system path

## Sources

TEMPLATE = lib
TARGET  = vnragent

HEADERS += \
  driver/maindriver.h \
  driver/maindriver_p.h \
  driver/rpcclient.h \
  driver/rpcclient_p.h \
  driver/settings.h \
  embed/embeddriver.h \
  embed/embedmanager.h \
  embed/embedmemory.h \
  engine/engine.h \
  engine/enginedef.h \
  engine/engineenv.h \
  engine/enginehash.h \
  engine/engineloader.h \
  engine/enginesettings.h \
  hijack/hijackdriver.h \
  hijack/hijackfuncs.h \
  hijack/hijackfuncs_p.h \
  hijack/hijackhelper.h \
  window/windowdriver.h \
  window/windowdriver_p.h \
  window/windowhash.h \
  window/windowmanager.h \
  util/codepage.h \
  util/location.h \
  util/msghandler.h \
  util/pathutil.h \
  util/textutil.h \
  config.h \
  debug.h \
  growl.h \
  loader.h

SOURCES += \
  driver/maindriver.cc \
  driver/rpcclient.cc \
  driver/settings.cc \
  embed/embeddriver.cc \
  embed/embedmanager.cc \
  embed/embedmemory.cc \
  engine/engine.cc \
  engine/engineenv.cc \
  engine/engineloader.cc \
  hijack/hijackdriver.cc \
  hijack/hijackfuncs.cc \
  hijack/hijackfuncs_gdi32.cc \
  hijack/hijackfuncs_kernel32.cc \
  hijack/hijackfuncs_user32.cc \
  hijack/hijackhelper.cc \
  window/windowdriver.cc \
  window/windowdriver_p.cc \
  window/windowmanager.cc \
  util/codepage.cc \
  util/location.cc \
  util/msghandler.cc \
  util/pathutil.cc \
  util/textutil.cc \
  growl.cc \
  loader.cc \
  main.cc

# Engine models
HEADERS += \
  engine/model/bgi.h \
  engine/model/majiro.h
  #engine/model/test.h
SOURCES += \
  engine/model/bgi.cc \
  engine/model/bgi1.cc \
  engine/model/bgi2.cc \
  engine/model/majiro.cc
  #engine/model/test.cc

#!wince*: LIBS += -lshell32
#RC_FILE += vnragent.rc

OTHER_FILES += vnragent.rc

# EOF
