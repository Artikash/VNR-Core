# qtsocketsvc.pri
# 4/29/2014 jichi
# Socket service client.
#include(../../../config.pri)
DEFINES += WITH_LIB_QTSOCKETCLI

DEPENDPATH += $$PWD

QT += core network

HEADERS += \
  $$PWD/bufferedsocketclient.h \
  $$PWD/socketdef.h \
  $$PWD/socketclient.h \
  $$PWD/socketio_p.h \
  $$PWD/socketpack.h

SOURCES += \
  $$PWD/bufferedsocketclient.cc \
  $$PWD/socketclient.cc \
  $$PWD/socketio_p.cc \
  $$PWD/socketpack.cc

OTHER_FILES += \
  $$PWD/qtsocketcli.pri \
  $$PWD/qtsocketpack.pri
  #$$PWD/qtsocketsrv.pri

# EOF
