# coding: utf8
# ocrman.py
# 8/13/2014 jichi

from PySide.QtCore import QObject
from sakurakit.skclass import memoized, Q_Q

@Q_Q
class _OcrManager(object):

  def __init__(self):
    self.language = 'ja'

  def readScreen(self):
    pass

class OcrManager(QObject):
  def __init__(self, parent=None):
    super(OcrManager, self).__init__(parent)
    self.__d = _OcrManager(self)


  def readScreen(x, y, width, height, language=''):
    if not language:
      language = self.language

  def readWidget(widget, x, y, width, height, language=''):
    if not language:
      language = self.language


@memoized
def manager(): return OcrManager()

# EOF
