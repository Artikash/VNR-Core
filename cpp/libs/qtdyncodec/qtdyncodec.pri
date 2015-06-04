# qtdyncodec.pri
# 6/3/2015 jichi

DEFINES += WITH_LIB_QTDYNCODEC

QT += core

DEPENDPATH += $$PWD

HEADERS += \
  $$PWD/dynsjis.h

SOURCES += \
  $$PWD/dynsjis.cc

# EOF
