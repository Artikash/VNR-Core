# coding: utf8
# ocrman.py
# 8/13/2014 jichi

import os
from PySide.QtCore import QObject, Signal, Qt
from sakurakit import skwin
from sakurakit.skclass import Q_Q, memoized, memoizedproperty
from sakurakit.skdebug import dprint, dwarn
from modiocr import modiocr
from mytr import my
import growl, windows, winman
from _ocrman import OcrImageObject, OcrSettings

@memoized
def manager(): return OcrManager()

@Q_Q
class _OcrManager(object):

  def __init__(self):
    self.enabled = False # bool
    self.settings = OcrSettings()
    self.pressedX = self.pressedY = 0

  @memoizedproperty
  def mouseSelector(self):
    from mousesel import mousesel
    ret = mousesel.global_()
    ret.setParentWidget(windows.top())
    ret.setRefreshInterval(5000) # refresh every 5 seconds
    ret.setRefreshEnabled(True)
    ret.pressed.connect(self._onPressed, Qt.QueuedConnection)
    ret.selected.connect(self._onRectSelected) # already queued

    import win32con
    ret.setComboKey(win32con.VK_SHIFT)
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

  def _onPressed(self, x, y):
    """
    @param  x  int
    @param  y  int
    """
    self.pressedX = x
    self.pressedY = y
    windows.raise_top_window()

  def _onRectSelected(self, x, y, width, height):
    """
    @param  x  int
    @param  y  int
    @param  width  int
    @param  height  int
    """
    imgobj = OcrImageObject.create(x, y, width, height,
        settings=self.settings, parent=self.q)
    if not imgobj:
      growl.notify(my.tr("OCR did not recognize any texts in the image"))
      return
    text = imgobj.ocr()
    lang = imgobj.language()
    winobj = self._getWindowObject(self.pressedX, self.pressedY)
    self.q.textReceived.emit(text, lang, x, y, width, height, imgobj, winobj)

  def _getWindowObject(self, x, y): # int, int -> QObject or None
    hwnd = skwin.get_window_at(x, y)
    return winman.manager().createWindowObject(hwnd) if hwnd else None #and hwnd != self.DESKTOP_HWND else None

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

class OcrManager(QObject):
  def __init__(self, parent=None):
    super(OcrManager, self).__init__(parent)
    self.__d = _OcrManager(self)

  textReceived = Signal(unicode, unicode, int, int, int, int, QObject, QObject) # text, language, x, y, width, height, OcrImageObject, WindowObject

  def languages(self): return self.__d.settings.languages
  def setLanguages(self, v):
    dprint(','.join(v))
    self.__d.settings.setLanguages(v)

  def isInstalled(self):
    return os.path.exists(modiocr.MODI_PATH) and modiocr.available() #and skwin.ADMIN

  def isSpaceEnabled(self): return self.__d.settings.isSpaceEnabled()
  def setSpaceEnabled(self, t): self.__d.settings.setSpaceEnabled(t)

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
