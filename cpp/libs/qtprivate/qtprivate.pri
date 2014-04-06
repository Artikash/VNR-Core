# qtprivate.pri
# 4/9/2012 jichi
#include(../../../config.pri)
DEFINES += WITH_LIB_QTPRIVATE

DEPENDPATH += $$PWD

HEADERS += \
  $$PWD/d/d_qdeclarativetextedit.h \
  $$PWD/d/d_qobject.h \
  $$PWD/d/d_qgraphicsitem.h

INCLUDEPATH     += $$QT_SRC
#QT      += core gui declarative

OTHER_FILES += \
  $$PWD/d/d.pri

# EOF
