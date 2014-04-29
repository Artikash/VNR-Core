# qtsocketpack.pri
# 4/29/2014 jichi
# Socket service marshaller.
#include(../../../config.pri)
DEFINES += WITH_LIB_QTSOCKETPACK

DEPENDPATH += $$PWD

QT += core

HEADERS += \
  $$PWD/qtsocketpack.h \
  $$PWD/qtsocketsvc.h

SOURCES += \
  $$PWD/qtsocketpack.cc

# EOF
