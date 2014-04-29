# singleapp.pri
# 2/28/2013 jichi
# Currently, only implemented for windows

DEFINES += WITH_LIB_SINGLEAPP

DEPENDPATH += $$PWD

HEADERS += $$PWD/singleapp.h
win32: SOURCES += $$PWD/singleapp_win.cc

#LIBS    += -lkernel32 -luser32

# EOF
