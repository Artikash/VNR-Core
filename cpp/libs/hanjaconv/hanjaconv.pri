# hanjaconv.pri
# 1/6/2015 jichi

DEFINES += WITH_LIB_HANJACONV

DEPENDPATH += $$PWD

#QT += core

HEADERS += \
  $$PWD/hanjachars.h \
  $$PWD/hanjaconfig.h \
  $$PWD/hanjaconv.h \
  $$PWD/hanjaconv_p.h
SOURCES += \
  $$PWD/hanjaconv.cc \
  $$PWD/hanjaconv_p.cc

# EOF
