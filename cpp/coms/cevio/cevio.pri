# disasm.pri
# 1/31/2012 jichi
#
# See: http://guide2.project-cevio.com/interface
# - COM: http://guide2.project-cevio.com/interface/com
# - .NET: http://guide2.project-cevio.com/interface/dotnet

win32 {

DEFINES     += WITH_LIB_CEVIO
DEPENDPATH  += $$PWD

HEADERS     += $$PWD/cevio.h
#SOURCES     += $$PWD/cevio.cc

}

# EOF
