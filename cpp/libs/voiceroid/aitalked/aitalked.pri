# aitalked.pri
# 10/11/2014 jichi
win32 {

DEFINES += WITH_LIB_AITALKED

AITALKED_SRC = $$PWD/cpp

INCLUDEPATH += $$AITALKED_SRC
DEPENDPATH += $$AITALKED_SRC/aitalk

LIBS += -luser32 # for hook functions

HEADERS += \
    $$AITALKED_SRC/aitalk/_windef.h \
    $$AITALKED_SRC/aitalk/aitalkapi.h \
    $$AITALKED_SRC/aitalk/aitalkconf.h \
    $$AITALKED_SRC/aitalk/aitalkdef.h \
    $$AITALKED_SRC/aitalk/aitalkutil.h
SOURCES += \
    $$AITALKED_SRC/aitalk/aitalkapi.cc \
    $$AITALKED_SRC/aitalk/aitalkutil.cc
}

# EOF
