# coding: utf8
# ocrman.py
# 8/13/2014 jichi

import os
from PySide.QtCore import QObject, Signal, Slot, Qt, QTimer
from PySide.QtGui import QColor
from sakurakit import skfileio, skwin
from sakurakit.skclass import Q_Q, memoized, memoizedproperty
from sakurakit.skdebug import dprint, dwarn
from modiocr import modiocr
from mytr import my
from _ocrman import OcrImageObject, OcrSettings
import _ocrman
import features, growl, rc, windows, winman

#from sakurakit.skprofiler import SkProfiler

@memoized
def manager(): return OcrManager()

@Q_Q
class _OcrManager(object):

  def __init__(self):
    self.enabled = False # bool

    self.settings = OcrSettings()
    self.pressedX = self.pressedY = 0

    self.selectionEnabled = False # bool  Allow emitting selected region using mouse
    self.selectedWindow = 0 # long  game window hwnd
    self.regionItems = [] # [QDeclarativeItem]
    self.regionOcrEnabled = False # bool  Allow applying OCR to the selected regions

  # Automatic sampling game window

  @memoizedproperty
  def ocrWindowTimer(self): # periodically check selected window
    ret = QTimer(self.q)
    ret.setSingleShot(False)
    #ret.setInterval(2000) # TODO: Allow change this value
    ret.timeout.connect(self.ocrWindow)
    return ret

  def setOcrWindowEnabled(self, t):
    t = self.ocrWindowTimer
    if t.isActive() != t:
      if t:
        t.start()
      else:
        t.stop()

  def ocrWindow(self):
    if self.enabled and self.regionOcrEnabled and self.selectedWindow and self.regionItems:
      texts = []
      for index, item in enumerate(self.regionItems):
        if item.property('visible') and item.property('enabled') and item.property('active') and not item.property('dragging'):
          width = item.property('width')
          height = item.property('height')
          x = item.property('x')
          y = item.property('y')
          text = self._ocrRegion(x, y, width, height, index=index, imageObject=item.property('imageObject'))
          if text and text != item.property('recognizedText'):
            item.setProperty('recognizedText', text)
            #text = termman.manager().applyOcrTerms(text)
            if text:
              texts.append(text)
      if texts:
        text = '\n'.join(texts)
        self.q.textRecognized.emit(text)

  def _ocrRegion(self, x, y, width, height, index=0, imageObject=None):
    """
    @param  x  int
    @param  y  int
    @param  width  int
    @param  height  int
    @param* index  int  an ID to distringuish the region
    @param* imageObject  OcrImageObject
    @return  unicode  recognized text
    """
    pm = self._capturePixmap(x, y, width, height)
    if pm:
      if imageObject:
        pm = imageObject.transformPixmap(pm)
      path = "%s/region.%s.%s" % (rc.DIR_TMP_OCR, index, _ocrman.OCR_IMAGE_FORMAT)
      if _ocrman.save_pixmap(pm, path):
        #with SkProfiler(): # 9/12/2014: 0.5 seconds
        text = self._ocrImageFile(path)
        #skfileio.removefile(path)

        # Crash with multi-threads
        #from functools import partial
        #from sakurakit import skthreads
        #text = skthreads.runsync(partial(self._ocrImageFile, path))
        return text
    return ''

  def _capturePixmap(self, x, y, width, height): # int, int, int, int -> QPixmap or None
    if x < 0:
      width += x
      x = 0
    if y < 0:
      height += y
      y = 0
    return _ocrman.capture_pixmap(x, y, width, height, hwnd=self.selectedWindow)

  def _ocrImageFile(self, path): # unicode ->
    delim = self.settings.deliminator
    if delim:
      return delim.join(modiocr.readtexts(path, self.settings.languageFlags))
    else:
      return modiocr.readtext(path, self.settings.languageFlags)

  # I/O hooks

  @memoizedproperty
  def mouseSelector(self):
    from mousesel import mousesel
    ret = mousesel.MouseSelector()
    ret.setRefreshInterval(5000) # refresh every 5 seconds
    ret.setRefreshEnabled(True)
    ret.pressed.connect(self._onMousePressed, Qt.QueuedConnection)
    ret.selected.connect(self._onMouseSelected) # already queued

    rb = ret.rubberBand() # QRubberBand
    rb.setParent(windows.top()) # must appear before setting window flags
    rb.setWindowFlags(rb.windowFlags()|Qt.Popup) # needed when parentWidget is not nullptr

    import win32con
    ret.setComboKey(win32con.VK_SHIFT)
    #ret.setComboKey(win32con.VK_MENU) # ALT
    return ret

  @memoizedproperty
  def rubberBand(self): # -> QRubberBand
    return self.mouseSelector.rubberBand()

  def setRubberBandColor(self, color): # QColor or str or Qt.color ->
    w = self.rubberBand
    if not color:
      w.setGraphicsEffect(None)
    else:
      if not isinstance(color, QColor):
        color = QColor(color)
      from Qt5.QtWidgets import QGraphicsColorizeEffect
      e = w.graphicsEffect()
      if not e or not isinstance(e, QGraphicsColorizeEffect):
        e = QGraphicsColorizeEffect(w)
        e.setStrength(1)
      e.setColor(color)
      w.setGraphicsEffect(e)

  #@memoizedproperty
  #def keyboardSignal(self):
  #  from kbsignal import kbsignal
  #  ret = kbsignal.KeyboardSignal()
  #  #ret.setRefreshInterval(5000) # refresh every 5 seconds
  #  #ret.setRefreshEnabled(True)
  #  ret.pressed.connect(self._onKeyPressed, Qt.QueuedConnection)

  #  import win32con
  #  ret.setKeyEnabled(win32con.VK_F2, True)
  #  return ret

  #@memoizedproperty
  #def rubberBand(self):
  #  from sakurakit.skrubberband import SkMouseRubberBand
  #  import windows
  #  parent = windows.top()
  #  #parent = None # this make rubberband as top window
  #  ret = SkMouseRubberBand(SkMouseRubberBand.Rectangle, parent)
  #  ret.setWindowFlags(ret.windowFlags()|Qt.Popup) # popup is needed to display the window out side of its parent
  #  ret.selected.connect(self._onMouseSelected, Qt.QueuedConnection) # do it later
  #  return ret

  # Key event

  def _onKeyPressed(self, vk):
    dprint(vk)

  # Mouse event

  def _onMousePressed(self, x, y):
    """
    @param  x  int
    @param  y  int
    """
    self.pressedX = x
    self.pressedY = y
    windows.raise_top_window()

  #def _onMouseSelected(self, x, y, width, height):
  def _onMouseSelected(self, *args):
    """
    @param  x  int
    @param  y  int
    @param  width  int
    @param  height  int
    """
    if self.selectionEnabled:
      self._selectRegion(*args)
    else:
      self._selectImage(*args)

  def createImageObject(self, x, y, width, height, hwnd=0): # int, int, int, int, int -> QObject
    return OcrImageObject.create(x, y, width, height,
        hwnd=hwnd,
        settings=self.settings, parent=self.q)

  def _selectImage(self, x, y, width, height):
    """
    @param  x  int
    @param  y  int
    @param  width  int
    @param  height  int
    """
    imgobj = self.createImageObject(x, y, width, height)
    if not imgobj:
      #growl.notify(my.tr("OCR did not recognize any texts in the image"))
      return
    hwnd = skwin.get_window_at(self.pressedX, self.pressedY)
    text = imgobj.ocr()
    lang = imgobj.language()
    winobj = winman.manager().createWindowObject(hwnd) if hwnd else None #and hwnd != self.DESKTOP_HWND else None
    self.q.imageSelected.emit(x, y, width, height, imgobj, winobj, text, lang)

  def _selectRegion(self, x, y, width, height):
    """
    @param  x  int
    @param  y  int
    @param  width  int
    @param  height  int
    """
    self.q.regionSelected.emit(x, y, width, height)

class OcrManager(QObject):
  def __init__(self, parent=None):
    super(OcrManager, self).__init__(parent)
    self.__d = _OcrManager(self)

  imageSelected = Signal(int, int, int, int, QObject, QObject, unicode, unicode) # x, y, width, height, OcrImageObject, WindowObject, text, language
  regionSelected = Signal(int, int, int, int) # x, y, width, height, OcrImageObject, WindowObject, text, language
  textRecognized = Signal(unicode) # text in the selected region is recognized

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
      #d.keyboardSignal.setEnabled(t)
      if t:
        growl.msg(my.tr("Start OCR screen reader"))
      else:
        growl.msg(my.tr("Stop OCR screen reader"))

  # Selection

  def isRegionSelectionEnabled(self): return self.__d.selectionEnabled
  def setRegionSelectionEnabled(self, t):
    dprint(t)
    d = self.__d
    if d.selectionEnabled != t:
      d.selectionEnabled = t
      #d.setRubberBandColor('red' if t else None)
      d.setRubberBandColor('magenta' if t else None)

  def isRegionOcrEnabled(self): return self.__d.regionOcrEnabled
  def setRegionOcrEnabled(self, t):
    dprint(t)
    self.__d.regionOcrEnabled = t

  def regionOcrInterval(self): return self.__d.ocrWindowTimer.interval()
  def setRegionOcrInterval(self, v):
    dprint(v)
    self.__d.ocrWindowTimer.setInterval(v)

  def selectedWindow(self): return self.__d.selectedWindow # long
  def setSelectedWindow(self, hwnd):
    dprint(hwnd)
    d = self.__d
    d.selectedWindow = hwnd
    d.setOcrWindowEnabled(bool(hwnd))

  def addRegionItem(self, item): # QDeclarativeItem ->  the item in ocrregion.qml
    self.__d.regionItems.append(item)
    #dprint(item) # too verbose

  def clearRegionItems(self):
    for item in self.__d.regionItems:
      if item.property('active'):
        item.setProperty('active', False)
      if item.property('visible'):
        item.setProperty('visible', False)

  def createImageObject(self, x, y, width, height): # int, int, int, int -> QObject or None
    return self.__d.createImageObject(x, y, width, height, self.__d.selectedWindow)


# EOF

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
