# hanviet.pri
# 2/2/2015 jichi

DEFINES += WITH_LIB_HANVIET

DEPENDPATH += $$PWD

#QT += core

HEADERS += \
  $$PWD/hanviet.h \
  $$PWD/phrasedic.h \
  $$PWD/worddic.h

SOURCES += \
  $$PWD/hanviet.cc \
  $$PWD/phrasedic.cc \
  $$PWD/worddic.cc

# EOF
