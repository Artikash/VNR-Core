# coding: utf8
# ocrman.py
# 8/13/2014 jichi

import os
from functools import partial
from PySide.QtCore import QObject, Signal
from PySide.QtGui import QPixmap
from Qt5.QtWidgets import QApplication
from sakurakit import skwidgets, skfileio
from sakurakit.skclass import Q_Q, memoized, memoizedproperty
from sakurakit.skdebug import dprint, dwarn
from modiocr import modiocr
from mytr import my
import growl, rc, termman

IMAGE_FORMAT = 'png'

@memoized
def manager(): return OcrManager()

@Q_Q
class _OcrManager(object):

  def __init__(self):
    self.enabled = False # bool
    #self.language = 'ja'

  @memoizedproperty
  def mouseSelector(self):
    from mousesel import mousesel
    ret = mousesel.global_()
    ret.selected.connect(self._onRectSelected)
    return ret

  # Rubberband

  #@memoizedproperty
  #def rubberBand(self):
  #  from sakurakit.skrubberband import SkMouseRubberBand
  #  import windows
  #  parent = windows.top()
  #  #parent = None # this make rubberband as top window
  #  ret = SkMouseRubberBand(SkMouseRubberBand.Rectangle, parent)
  #  ret.setWindowFlags(ret.windowFlags()|Qt.Popup) # popup is needed to display the window out side of its parent
  #  ret.selected.connect(self._onRectSelected, Qt.QueuedConnection) # do it later
  #  return ret

  def _onRectSelected(self, x, y, width, height):
    """
    @param  x  int
    @param  y  int
    @param  width  int
    @param  height  int
    """
    #dprint(x, y, width, height)
    text = self.readScreen(x, y, width, height)
    if text:
      text = termman.manager().applyOcrTerms(text)
      if text:
        self.q.textReceived.emit(text, x, y, width, height)
        return
    growl.notify(my.tr("OCR did not recognize Japanese text"))

  # Mouse hook

  #@memoizedproperty
  #def mouseHook(self):
  #  from mousehook.screenselector import ScreenSelector
  #  ret = ScreenSelector()
  #  from sakurakit import skwin
  #  ret.setPressCondition(skwin.is_key_shift_pressed)
  #  ret.setSingleShot(False)
  #  # Use queued connection to avoid possible crash since it is on a different thread?
  #  rb = self.rubberBand
  #  ret.mousePressed.connect(rb.press, Qt.QueuedConnection)
  #  ret.mouseReleased.connect(rb.release, Qt.QueuedConnection)
  #  ret.mouseMoved.connect(rb.move, Qt.QueuedConnection)
  #  return ret

  # OCR

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
      qApp = QApplication.instance()
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
    return modiocr.readtext(path, modiocr.LANG_JA) # force Japanese

  def readScreen(self, x, y, width, height):
    """
    @param  x  int
    @param  y  int
    @param  width  int
    @param  height  int
    @param* async  bool  FIXME: why async would crash OCR?
    @return  unicode or None
    """
    dprint("enter")
    pm = self._capturePixmap(x, y, width, height)
    if not pm or pm.isNull():
      dwarn("leave: failed to capture image")
      return
    # TODO: process pixmap here
    path = self._randomPath()
    if not self._savePixmap(pm, path) or not os.path.exists(path):
      dwarn("leave: failed to save pixmap")
      return
    # FIXME: Async would crash
    #ret = skthreads.runsync(partial(self._readImageFile, path))
    #with SkProfiler(): # take around 3 seconds
    ret = self._readImageFile(path)
    #if not config.APP_DEBUG:
    #  skfileio.removefile(path) # do not remove file, in case two random paths are the same
    dprint("leave")
    return ret

class OcrManager(QObject):
  def __init__(self, parent=None):
    super(OcrManager, self).__init__(parent)
    self.__d = _OcrManager(self)

  textReceived = Signal(unicode, int, int, int, int) # text, x, y, width, height

  def isInstalled(self):
    return os.path.exists(modiocr.MODI_PATH) and modiocr.available() #and skwin.ADMIN

  def isEnabled(self): return self.__d.enabled

  def setEnabled(self, t):
    d = self.__d
    if d.enabled != t:
      d.enabled = t
      dprint(t)
      d.mouseSelector.setEnabled(t)
      if t:
        growl.msg(my.tr("Start OCR screen reader"))
      else:
        growl.msg(my.tr("Stop OCR screen reader"))

# EOF