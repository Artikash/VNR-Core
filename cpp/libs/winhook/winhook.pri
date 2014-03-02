# winhook.pri
# 7/20/2011 jichi
win32 {

DEFINES += WITH_LIB_WINHOOK

DEPENDPATH      += $$PWD

HEADERS += $$PWD/funchook.h
SOURCES += $$PWD/funchook.cc

LIBS    += -lkernel32 -luser32
}

# EOF
