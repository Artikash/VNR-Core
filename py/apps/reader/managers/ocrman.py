# coding: utf8
# ocrman.py
# 8/13/2014 jichi

import os
from functools import partial
from PySide.QtCore import QObject, QCoreApplication
from PySide.QtGui import QPixmap
from sakurakit import skwidgets, skfileio, skthreads
from sakurakit.skclass import memoized, Q_Q
from sakurakit.skdebug import dprint, dwarn
from modiocr import modiocr
import rc

IMAGE_FORMAT = 'png'

@memoized
def manager(): return OcrManager()

@Q_Q
class _OcrManager(object):

  def __init__(self):
    self.language = 'ja'

  @staticmethod
  def _randomPath():
    """
    @return  unicode
    """
    from time import time
    return "%s/%f.%s" % (rc.DIR_TMP_OCR, time(), IMAGE_FORMAT)

  @staticmethod
  def _capturePixmap(x, y, width, height, wid=None):
    """
    @param  x  int
    @param  y  int
    @param  width  int
    @param  height  int
    @param  wid  WId
    @return  QPixmap
    """
    if wid:
      return QPixmap.grabWindow(skwidgets.to_wid(wid), x, y, width, height)
    else:
      qApp = QCoreApplication.instance()
      wid = qApp.desktop().winId()
      return QPixmap.grabWindow(wid, x, y, width, height)

  @staticmethod
  def _savePixmap(pm, path, format=IMAGE_FORMAT, quality=-1):
    """
    @param  pm  QPixmap
    @param  path  unicode
    @return  bool
    """
    return pm.save(path, format, quality)

  @staticmethod
  def _readImageFile(path):
    """
    @param  path  unicode
    @return  unicode  text
    """
    return modiocr.readtext(path, modiocr.LANG_JA)

  def readScreen(self, x, y, width, height, async=False):
    """
    @param  x  int
    @param  y  int
    @param  width  int
    @param  height  int
    @param* async  bool  FIXME: why async would crash OCR?
    @return  unicode or None
    """
    pm = self._capturePixmap(x, y, width, height)
    if not pm or pm.isNull():
      dwarn("failed to capture image")
      return
    # TODO: process pixmap here
    path = self._randomPath()
    if not self._savePixmap(pm, path) or not os.path.exists(path):
      dwarn("failed to save pixmap")
      return
    if async:
      ret = skthreads.runsync(partial(self._readImageFile, path))
    else:
      ret = self._readImageFile(path)
    #skfileio.removefile(path)
    return ret

class OcrManager(QObject):
  def __init__(self, parent=None):
    super(OcrManager, self).__init__(parent)
    self.__d = _OcrManager(self)

  def start(self):
    dprint("pass")
    from sakurakit.skprofiler import SkProfiler
    with SkProfiler():
      self.__d.readScreen(0, 0, 1440, 900)

  def stop(self):
    dprint("pass")

# EOF
