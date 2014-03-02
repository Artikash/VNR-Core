# coding: utf8
# qmlutil.py
# 1/6/2013 jichi

import os
from PySide.QtCore import Slot, QObject, QUrl
from sakurakit import skmeta
from sakurakit.skdebug import dwarn
from sakurakit.skqml import QmlObject
import bbcode

@QmlObject
class BBCodeParser(QObject):
  def __init__(self, parent=None):
    QObject.__init__(self, parent)

  @Slot(unicode, result=unicode)
  def parse(self, text): return bbcode.parse(text)

@QmlObject
class QmlUtil(QObject):
  def __init__(self, parent=None):
    QObject.__init__(self, parent)

  @Slot(unicode, result=bool)
  def fileExists(self, value):
    return bool(value) and os.path.exists(value)

  @Slot(unicode, result=bool)
  def urlExists(self, value):
    if not value:
      return False
    url = QUrl(value)
    if not url.isLocalFile():
      return True
    path = url.toLocalFile()
    return os.path.exists(path)

  @Slot(QObject, unicode, QObject, unicode)
  def bind(self, obj1, pty1, obj2, pty2):
    p = (obj1, pty1, obj2, pty2)
    if all(p):
      skmeta.bind_properties(p)
    else:
      dwarn("null property", p)

# EOF
