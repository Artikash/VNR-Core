# metacall.pri
# 4/9/2012 jichi

DEFINES += WITH_LIB_METACALL

DEPENDPATH += $$PWD

HEADERS += \
  $$PWD/metacallevent_p.h \
  $$PWD/metacallfilter_p.h \
  $$PWD/metacallpropagator.h \
  $$PWD/metacallpropagator_p.h \
  $$PWD/metacallrouter.h
SOURCES += \
  $$PWD/metacallfilter_p.cc \
  $$PWD/metacallpropagator.cc

QT += core network

#INCLUDEPATH += $$QT_SRC

# EOF
