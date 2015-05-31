# winasm.pri
# 5/25/2013 jichi
win32 {

DEFINES += WITH_LIB_WINASM

DEPENDPATH += $$PWD

HEADERS += $$PWD/winasmdef.h
#SOURCES += $$PWD/winasm.cc
}

# EOF
