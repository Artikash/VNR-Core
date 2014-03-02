# qtmodule.pri
# 2/1/2013 jichi
#
# Motivations:
# http://stackoverflow.com/questions/2150488/using-a-qt-based-dll-in-a-non-qt-application
# http://stackoverflow.com/questions/1786438/qt-library-event-loop-problems

DEFINES += WITH_LIB_QTMODULE

DEPENDPATH += $$PWD

HEADERS += \
  $$PWD/qapplicationloader.h
SOURCES += \
  $$PWD/qapplicationloader.cc

QT += core

#INCLUDEPATH += $$QT_SRC

# EOF
