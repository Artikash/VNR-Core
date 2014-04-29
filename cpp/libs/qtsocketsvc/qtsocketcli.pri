# qtsocketcli.pri
# 4/29/2014 jichi
#include(../../../config.pri)
DEFINES += WITH_LIB_QTSOCKETCLI

DEPENDPATH += $$PWD

QT += core network

HEADERS += \
  $$PWD/qtsocketcli.h \
  $$PWD/qtsocketsvc.h

SOURCES += \
  $$PWD/qtsocketcli.cc

# EOF
