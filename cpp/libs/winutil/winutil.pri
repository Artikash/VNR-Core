# winutil.pri
# 4/7/2013 jichi
win32 {
DEFINES += WITH_LIB_WINUTIL

DEPENDPATH      += $$PWD

HEADERS += $$PWD/winutil.h
SOURCES += $$PWD/winutil.cc

LIBS += -lshell32 -lole32
}

# EOF
