# plugins.pro
# 7/31/2011 jichi
#
# Dependence
# - texthook => pytexthook

TEMPLATE = subdirs

SUBDIRS += qmleffectsplugin
SUBDIRS += qmlhelperplugin
SUBDIRS += qmltexscriptplugin
SUBDIRS += qmltextplugin

SUBDIRS += pyreader

#SUBDIRS += pyqthelper
#SUBDIRS += pymetacall
#SUBDIRS += pytexscript
win32: SUBDIRS += ith
win32: SUBDIRS += texthook pytexthook
#win32: SUBDIRS += texthookxp pytexthookxp

win32: SUBDIRS += vnragent

#win32: SUBDIRS += pyeffects
win32: SUBDIRS += pymsime
win32: SUBDIRS += pywintts
win32: SUBDIRS += pywinutil

# EOF
