# uitest.pro
# 4/5/2014

include(../../../config.pri)
include($$LIBDIR/graphicseffect/graphicseffect.pri)

# Source

TEMPLATE = app
TARGET = uitest

DEPENDPATH += .
INCLUDEPATH += .

SOURCES += main.cc

# EOF
