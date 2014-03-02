# libqxt.pri
# 2/3/2013 jichi
#
# http://dev.libqxt.org
# Version: 0.7
# Checkout: 2/3/2012

DEFINES += WITH_LIB_HTMLUTIL
DEFINES += QXT_STATIC QXT_DOXYGEN_RUN

INCLUDEPATH += \
  $$PWD/include \
  $$PWD/include/QxtCore \
  $$PWD/src \
  $$PWD/src/core

DEPENDPATH += \
  $$PWD/include/QxtCore \
  $$PWD/src/core

HEADERS += \
  $$PWD/src/core/qxtglobal.h \
  $$PWD/src/core/qxtjson.h

SOURCES += \
  $$PWD/src/core/qxtglobal.cpp \
  $$PWD/src/core/qxtjson.cpp

QT += core

# EOF
