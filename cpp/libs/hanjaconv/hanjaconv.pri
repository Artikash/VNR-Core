# hanjaconv.pri
# 1/6/2015 jichi

DEFINES += WITH_LIB_HANJACONV

DEPENDPATH += $$PWD

#QT += core

HEADERS += \
  $$PWD/hangul2hanja.h \
  $$PWD/hangul2hanja_p.h \
  $$PWD/hangulconfig.h
SOURCES += \
  $$PWD/hangul2hanja.cc \
  $$PWD/hangul2hanja_p.cc

# EOF
