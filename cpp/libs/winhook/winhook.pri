# winhook.pri
# 5/25/2015 jichi
win32 {

# This library requires disasm

DEFINES += WITH_LIB_WINHOOK

#LIBS += -lkernel32 -luser32

DEPENDPATH += $$PWD

HEADERS += \
  $$PWD/hookcode.h \
  $$PWD/hookdef.h
SOURCES += \
  $$PWD/hookcode.cc
}

# EOF
