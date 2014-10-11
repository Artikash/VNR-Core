# aitalked.pri
# 10/11/2014 jichi
win32 {

DEFINES += WITH_LIB_AITALKED

INCLUDEPATH += $$PWD/..
DEPENDPATH += $$PWD

LIBS += -luser32 # for hook functions

HEADERS += \
  $$PWD/aitalk.h \
  $$PWD/aitalked.h \
  $$PWD/aitalksync.h
SOURCES += \
  $$PWD/aitalk.cc \
  $$PWD/aitalked.cc \
  $$PWD/aitalksync.cc
}

# EOF
