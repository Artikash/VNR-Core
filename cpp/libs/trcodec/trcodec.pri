# trcodec.pri
# 9/20/2014 jichi

DEFINES += WITH_LIB_TRCODEC

DEPENDPATH += $$PWD

LIBS += -lboost_regex$$BOOST_VARIANT

#QT += core

HEADERS += \
  $$PWD/trdefine.h \
  $$PWD/trescape.h \
  $$PWD/trencode.h \
  $$PWD/trencoderule.h

SOURCES += \
  $$PWD/trencode.cc \
  $$PWD/trencoderule.cc

# EOF
