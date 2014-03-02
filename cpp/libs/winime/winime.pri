# winime.pri
# 4/1/2013 jichi
win32 {

DEFINES += WITH_LIB_WINIME

DEPENDPATH      += $$PWD

HEADERS += $$PWD/winime.h
SOURCES += $$PWD/winime.cc

INCLUDEPATH += $$MSIME_HOME/include
LIBS    += -lole32
}

# EOF
