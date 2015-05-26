# winhook.pri
# 5/25/2015 jichi
win32 {

# This library requires disasm

DEFINES += WITH_LIB_WINHOOK

#LIBS += -lkernel32 -luser32

DEPENDPATH += $$PWD

HEADERS += $$PWD/winhook.h
SOURCES += $$PWD/winhook.cc
}

# EOF
