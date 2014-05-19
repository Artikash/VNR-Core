# vnragent.pro
# 1/22/2013 jichi

CONFIG += noqt dll
include(../../../config.pri)
include($$LIBDIR/windbg/windbg.pri)
include($$LIBDIR/winsinglemutex/winsinglemutex.pri)

#LIBS    += -lkernel32
#LIBS    += -luser32 -lpsapi
#LIBS    += -lshell32 # needed by get system path

## Sources

TEMPLATE = lib
TARGET  = vnrloc

HEADERS += \
  config.h \
  debug.h \
  settings.h

SOURCES += \
  main.cc

#!wince*: LIBS += -lshell32
#RC_FILE += vnrlocale.rc

OTHER_FILES += vnrlocale.rc

# EOF
