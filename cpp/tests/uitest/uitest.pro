# uitest.pro
# 4/5/2014

include(../../../config.pri)
#include($$LIBDIR/graphicseffect/graphicseffect.pri)

# Source

INCLUDEPATH     += $$QT_SRC

TEMPLATE = app
TARGET = uitest

DEPENDPATH += .
INCLUDEPATH += .

HEADERS += main.h
SOURCES += main.cc

# EOF
