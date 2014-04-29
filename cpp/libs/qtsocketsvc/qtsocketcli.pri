# qtsocketcli.pri
# 4/29/2014 jichi
# Socket service client.
#include(../../../config.pri)
DEFINES += WITH_LIB_QTSOCKETCLI

DEPENDPATH += $$PWD

QT += core network

HEADERS += \
  $$PWD/socketclient.h \
  $$PWD/socketpack.h

SOURCES += \
  $$PWD/socketclient.cc

# EOF
