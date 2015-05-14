# host.pri
# 8/9/2011 jichi

DEFINES += WITH_LIB_ITH_HOST

DEPENDPATH  += $$PWD

HEADERS += \
  $$PWD/avl_p.h \
  $$PWD/config.h \
  $$PWD/hookman.h \
  $$PWD/settings.h \
  $$PWD/host.h \
  $$PWD/host_p.h \
  $$PWD/textthread.h \
  $$PWD/textthread_p.h

SOURCES += \
  $$PWD/hookman.cc \
  $$PWD/host.cc \
  $$PWD/pipe.cc \
  $$PWD/textthread.cc

# EOF
