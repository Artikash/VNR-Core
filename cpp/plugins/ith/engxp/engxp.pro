# engxp.pro
# 8/9/2013 jichi
# Build vnrengxp.dll for Windows XP

CONFIG += noeh # msvcrt on Windows XP does not has exception handler
include(../dllconfig.pri)
include(../clixp/clixp.pri)
include(../sys/sys.pri)
include($$LIBDIR/disasm/disasm.pri)
include($$LIBDIR/memdbg/memdbg.pri)
include($$LIBDIR/ntinspect/ntinspect.pri)
include($$LIBDIR/winseh/winseh_safe.pri)

VPATH += ../eng
INCLUDEPATH += ../eng

# 9/27/2013: disable ITH this game engine, only for debugging purpose
#DEFINES += ITH_DISABLE_ENGINE

## Libraries

LIBS    += -lkernel32 -luser32 -lgdi32

## Sources

TEMPLATE = lib
#TARGET   = ITH_Engine # compatible with ITHv3
TARGET   = vnrengxp

#CONFIG += staticlib

HEADERS += \
  config.h \
  engine.h \
  engine_p.h \
  hookdefs.h \
  pchooks.h \
  uniquemap.h \
  util.h

SOURCES += \
  engine.cc \
  engine_p.cc \
  main.cc \
  pchooks.cc \
  uniquemap.cc \
  util.cc

#RC_FILE += engine.rc
#OTHER_FILES += engine.rc

# EOF
