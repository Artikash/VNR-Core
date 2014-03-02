# launcher.pri
# 3/29/2012 jichi

DEFINES     += WITH_SRC_LAUNCHER
INCLUDEPATH += $$PWD/include
DEPENDPATH  += $$PWD/include

HEADERS += \
  $$PWD/include/cc/ccmacro.h \
  $$PWD/include/main.tcc \
  $$PWD/include/pymain.tcc

QT      += core

# EOF
