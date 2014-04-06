# metacall.pri
# 4/9/2012 jichi

DEFINES += WITH_LIB_METACALL

DEPENDPATH += $$PWD

HEADERS += \
  $$PWD/d_qdeclarativetextedit.h \
  $$PWD/d_qobject.h \
  $$PWD/d_qgraphicsitem.h

INCLUDEPATH     += $$QT_SRC
#QT      += core gui declarative

# EOF
