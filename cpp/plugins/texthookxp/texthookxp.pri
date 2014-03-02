# texthookxp.pri
# 10/13/2011 jichi

DEFINES += WITH_LIB_TEXTHOOK

INCLUDEPATH += $$PWD/../texthook
DEPENDPATH += $$PWD $$PWD/../texthook

QT      += core
LIBS    += -ltexthookxp

HEADERS += \
  $$PWD/../texthook/texthook_config.h

# texthook.h should not be in HEADERS, or it will be processed by moc
OTHER_FILES += \
  $$PWD/../texthook/texthook.h \
  $$PWD/../texthook/texthook.pro

# EOF
