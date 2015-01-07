# plugins.pro
# 7/31/2011 jichi
#
# Dependence
# - texthook => pytexthook

TEMPLATE = subdirs

SUBDIRS += qmleffectsplugin
SUBDIRS += qmlgradientplugin
SUBDIRS += qmlhelperplugin
SUBDIRS += qmltexscriptplugin
SUBDIRS += qmltextplugin

SUBDIRS += pyhangul
SUBDIRS += pytahscript
SUBDIRS += pyvnrmem
#SUBDIRS += pyreader
#SUBDIRS += pyqthelper
#SUBDIRS += pymetacall
#SUBDIRS += pytexscript

#win32: SUBDIRS += mousehook #pymousehook

win32: SUBDIRS += pycc
win32: SUBDIRS += pytrscript

win32: SUBDIRS += ith
win32: SUBDIRS += texthook pytexthook
#win32: SUBDIRS += texthookxp pytexthookxp

win32: SUBDIRS += pykbsignal
win32: SUBDIRS += pymousesel

win32: SUBDIRS += vnragent
win32: SUBDIRS += vnrlocale

#win32: SUBDIRS += pyeffects
win32: SUBDIRS += pymodi
win32: SUBDIRS += pymsime
win32: SUBDIRS += pysapi
win32: SUBDIRS += pywinutil
win32: SUBDIRS += pywmp
win32: SUBDIRS += pyzunko

# EOF
