# mousehook.pri
# 11/26/2011

DEFINES += WITH_LIB_MOUSEHOOK

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += \
    $$PWD/mousehook_config.h \
    $$PWD/mousehook.h

QT += core #gui

LIBS += -lmousehook

OTHER_FILES += \
  $$PWD/mousehook.pro

# EOF
