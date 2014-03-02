# texscript.pri
# 6/28/2011 jichi

DEFINES += WITH_LIB_TEXSCRIPT

DEPENDPATH +=$$PWD

HEADERS += \
  $$PWD/texhtml_p.h \
  $$PWD/texhtml.h \
  $$PWD/textag.h

SOURCES += \
  $$PWD/texhtml_p.cc \
  $$PWD/texhtml.cc

QT      += core

# EOF
