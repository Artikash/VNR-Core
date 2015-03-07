# trcodec.pri
# 9/20/2014 jichi

DEFINES += WITH_LIB_TRCODEC

DEPENDPATH += $$PWD

LIBS += -lboost_regex$$BOOST_VARIANT

#QT += core

HEADERS += \
  $$PWD/trcodec.h \
  $$PWD/trdefine.h \
  $$PWD/trescape.h \
  $$PWD/trencode.h \
  $$PWD/trencoderule.h \
  $$PWD/trrule.h

SOURCES += \
  $$PWD/trcodec.cc \
  $$PWD/trencode.cc \
  $$PWD/trencoderule.cc

# EOF
