# coding: utf8
# comets.py
# 8/31/2014 jichi

import qmldialog

def globalComet(): # -> QObject
  inst = qmldialog.Kagami.instance
  if inst:
    return inst.globalComet

def gameComet(): # -> QObject
  inst = qmldialog.Kagami.instance
  if inst:
    return inst.gameComet

def termComet(): # -> QObject or None  not available before termview is created
  inst = qmldialog.TermView.instance
  if inst:
    return inst.termComet

# EOF
