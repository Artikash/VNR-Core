# texthookxp.pro
# 10/13/2011 jichi
# Build ith texthook dll.

VPATH += ../texthook
INCLUDEPATH += ../texthook

CONFIG += noqtgui dll #eha # do not catch all exceptions on windows XP
include(../../../config.pri)
include($$PLUGINDIR/ith/srv/srv.pri)
include($$LIBDIR/winmaker/winmaker.pri)
include($$LIBDIR/winmutex/winmutex.pri)
include($$LIBDIR/wintimer/wintimer.pri)

DEFINES  += ITH_HAS_CRT # Use native CRT

## Libraries

QT += core
QT -= gui

#WDK_HOME  = c:/winddk
#LIBS    += -L$$WDK_HOME/lib
# override folder must come before winddk/inc/api
#INCLUDEPATH += $$PWD/override/wdk/vc10
#INCLUDEPATH += $$WDK_HOME/include/api
#INCLUDEPATH += $$WDK_HOME/include/crt
#INCLUDEPATH += $$WDK_HOME/include/ddk

#ITH_HOME  = c:/dev/ith
#INCLUDEPATH += $$ITH_HOME/include
#LIBS    += -L$$ITH_HOME/lib
#LIBS    += -lITH_DLL #-lITH_SYS

#INCLUDEPATH += $$ITH_HOME/include
#LIBS        += -L$$ITH_HOME/lib -lihf #-lihf_dll
#LIBS        += -lvnrsrv

# Tell IHF not to override new operators, see: ith/mem.h
#DEFINES += ITH_HAS_CXX
#LIBS    += -lith_sys -lntdll
#LIBS    += -lith_tls -lntdll
#LIBS    += -lntoskrnl

#LIBS  += -lmsvcrtd
#LIBS  += -lmsvcrt
#QMAKE_LFLAGS += /NODEFAULTLIB:msvcrt.lib /NODEFAULTLIB:msvcrtd.lib
DEFINES += _CRT_NON_CONFORMING_SWPRINTFS

## Sources

TEMPLATE = lib
TARGET  = texthookxp

#CONFIG += staticlib
DEFINES += TEXTHOOK_BUILD_LIB

HEADERS += \
  ihf_p.h \
  ith_p.h \
  texthook_config.h \
  texthook.h \
  texthook_p.h \
  textthread_p.h \
  winapi_p.h

SOURCES += \
  ihf_p.cc \
  ith_p.cc \
  texthook.cc \
  textthread_p.cc \
  winapi_p.cc

#!wince*: LIBS += -lshell32
#RC_FILE += texthook.rc

OTHER_FILES += \
  texthook.pri \
  texthook_static.pri \
  texthook.rc

# EOF
