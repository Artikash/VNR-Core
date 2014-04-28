# qtmetacall.pri
# 4/9/2012 jichi

DEFINES += WITH_LIB_METACALL

DEPENDPATH += $$PWD

HEADERS += \
  $$PWD/metacallfilter_p.h \
  $$PWD/metacallobserver.h \
  $$PWD/metacallpropagator.h \
  $$PWD/metacallpropagator_p.h \
  $$PWD/metacallrouter.h \
  $$PWD/qmetacallevent_p.h \
  $$PWD/qtmetacall.h
  #$$PWD/metacallthread.h
  #$$PWD/metacallthread_p.h
SOURCES += \
  $$PWD/metacallfilter_p.cc \
  $$PWD/metacallpropagator.cc
  #$$PWD/metacallthread.cc

QT += core network

#INCLUDEPATH += $$QT_SRC

# EOF
