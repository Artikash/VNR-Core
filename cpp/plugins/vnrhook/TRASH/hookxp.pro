# hookxp.pro
# 8/9/2013 jichi
# Build vnrhookxp.dll for Windows XP

CONFIG += noeh # msvcrt on Windows XP does not has exception handler
include(../dllconfig.pri)
include(../sys/sys.pri)
include($$LIBDIR/disasm/disasm.pri)
include($$LIBDIR/memdbg/memdbg.pri)
include($$LIBDIR/ntinspect/ntinspect.pri)
include($$LIBDIR/winkey/winkey.pri)
include($$LIBDIR/winseh/winseh_safe.pri)
include($$LIBDIR/winversion/winversion.pri)

VPATH += ../hook
INCLUDEPATH += ../hook

# 9/27/2013: disable ITH this game engine, only for debugging purpose
#DEFINES += ITH_DISABLE_ENGINE

# jichi 9/22/2013: When ITH is on wine, mutex is needed to protect NtWriteFile
#DEFINES += ITH_WINE
#DEFINES += ITH_SYNC_PIPE

DEFINES += MEMDBG_NO_STL NTINSPECT_NO_STL

## Libraries

LIBS    += -lkernel32 -luser32 -lgdi32 #-lgdiplus

## Sources

TEMPLATE = lib
TARGET   = vnrhookxp

#CONFIG += staticlib

HEADERS += \
  config.h \
  cli.h \
  hook.h \
  engine/engine.h \
  engine/hookdefs.h \
  engine/match.h \
  engine/pchooks.h \
  engine/util.h \
  tree/avl.h

SOURCES += \
  main.cc \
  rpc/pipe.cc \
  hijack/texthook.cc \
  engine/engine.cc \
  engine/match.cc \
  engine/pchooks.cc \
  engine/util.cc

#RC_FILE += vnrhook.rc
#OTHER_FILES += vnrhook.rc

# EOF
