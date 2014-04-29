# winhook.pri
# 7/20/2011 jichi
win32 {

# This library requires disasm

DEFINES += WITH_LIB_WINHOOK

LIBS += -lkernel32 -luser32

DEPENDPATH += $$PWD

HEADERS += \
  $$PWD/funchook.h \
  $$PWD/winhook.h
SOURCES += \
   $$PWD/funchook.cc
}

# EOF
