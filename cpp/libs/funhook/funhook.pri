# funhook.pri
# 7/20/2011 jichi
win32 {

# This library requires disasm

DEFINES += WITH_LIB_FUNHOOK

LIBS += -lkernel32 -luser32

DEPENDPATH += $$PWD

HEADERS += \
  $$PWD/funchook.h \
  $$PWD/funhookdef.h
SOURCES += \
   $$PWD/funchook.cc
}

# EOF
