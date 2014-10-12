# aitalked.pri
# 10/11/2014 jichi
win32 {

DEFINES += WITH_LIB_AITALKED

INCLUDEPATH += $$PWD/..
DEPENDPATH += $$PWD

LIBS += -luser32 # for hook functions

HEADERS += \
    $$PWD/aitalkapi.h \
    $$PWD/aitalkconfig.h \
    $$PWD/aitalked.h \
    $$PWD/aitalksync.h
SOURCES += \
    $$PWD/aitalkapi.cc \
    $$PWD/aitalked.cc \
    $$PWD/aitalksync.cc
}

# EOF
