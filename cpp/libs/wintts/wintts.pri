# wintts.pri
# 4/7/2013 jichi
win32 {

DEFINES += WITH_LIB_WINTTS

DEPENDPATH      += $$PWD

HEADERS += $$PWD/wintts.h
SOURCES += $$PWD/wintts.cc

INCLUDEPATH += $$SAPI_HOME/include
LIBS    += -lole32
}

# EOF
