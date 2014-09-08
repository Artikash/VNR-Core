# coding: utf8
# _ocrman.py
# 9/7/2014 jichi

__all__ = ['OcrImageObject', 'OcrSettings']

import os
from PySide.QtCore import QObject, Slot
from sakurakit import skfileio
from sakurakit.skdebug import dprint, dwarn
from modiocr import modiocr
import termman

OCR_MIN_WIDTH = 3
OCR_MIN_HEIGHT = 3

OCR_IMAGE_FORMAT = 'png'
OCR_IMAGE_QUALITY = 100 # -1 or [0, 100], 100 is lossless quality

def capture_pixmap(x, y, width, height, hwnd=None):
  """
  @param  x  int
  @param  y  int
  @param  width  int
  @param  height  int
  @param  hwnd  int
  @return  QPixmap
  """
  from PySide.QtGui import QPixmap
  if hwnd:
    from sakurakit import skwidgets
    return QPixmap.grabWindow(skwidgets.to_wid(hwnd), x, y, width, height)
  else:
    from PySide.QtCore import QCoreApplication
    qApp = QCoreApplication.instance()
    wid = qApp.desktop().winId()
    return QPixmap.grabWindow(wid, x, y, width, height)

class OcrSettings(object):
  def __init__(self):
    self.deliminator = '' # str
    self.languages = [] # [str lang]
    self.languageFlags = modiocr.LANG_JA # int

  def isSpaceEnabled(self): return bool(self.deliminator)
  def setSpaceEnabled(self, t): self.deliminator = ' ' if t else ''

  def language(self): return self.languages[0] if self.languages else 'ja' # -> str

  def setLanguages(self, v): # [str]
    self.languageFlags = modiocr.locales2lang(v) or modiocr.LANG_JA # Japanese by default

class _OcrImageObject:

  def __init__(self, pixmap, settings):
    self.settings = settings # OcrSettings
    self.pixmap = pixmap # QPixmap
    self.path = self._randomPath() # str

  @staticmethod
  def _randomPath():
    """
    @return  unicode
    """
    import rc
    from time import time
    return "%s/%f.%s" % (rc.DIR_TMP_OCR, time(), OCR_IMAGE_FORMAT)

  # OCR

  def ocr(self):
    """
    @return  unicode
    """
    if not self._savePixmap(self.pixmap):
      return ''
    # FIXME: Async would crash
    #ret = skthreads.runsync(partial(self._readImageFile, path))
    #with SkProfiler(): # take around 3 seconds
    text = self._readImage()
    if text:
      text = termman.manager().applyOcrTerms(text)
    return text

  def _readImage(self):
    """
    @return  unicode  text
    """
    delim = self.settings.deliminator
    if delim:
      return delim.join(modiocr.readtexts(self.path, self.settings.languageFlags))
    else:
      return modiocr.readtext(self.path, self.settings.languageFlags)

  # Screenshot

  def _savePixmap(self, pm):
    """
    @param  pm  QPixmap
    @param  path  unicode
    @return  bool
    """
    return pm.save(self.path, OCR_IMAGE_FORMAT, OCR_IMAGE_QUALITY) and os.path.exists(self.path)

# Passed to QML
class OcrImageObject(QObject):
  def __init__(self, parent=None, **kwargs):
    """
    @param  parent  QObject
    @param  pixmap  QPixmap
    @param  settings  OcrSettings
    """
    super(OcrImageObject, self).__init__(parent)
    self.__d = _OcrImageObject(**kwargs)

  @Slot(result=unicode)
  def ocr(self):
    """
    @return  unicode
    """
    return self.__d.ocr()

  def language(self):
    """
    @return  str
    """
    return self.__d.settings.language()

  @Slot()
  def release(self):
    self.setParent(None)
    skfileio.removefile(self.__d.path)
    dprint("pass")

  @classmethod
  def create(cls, x, y, width, height, hwnd=0, **kwargs): # -> cls or None
    """
    @param  x  int
    @param  y  int
    @param  width  int
    @param  height  int
    @param* hwnd  int
    @param* kwargs  parameters to create OcrImageObject
    """
    if width < OCR_MIN_WIDTH or height < OCR_MIN_HEIGHT:
      dwarn("skip image that is too small")
      return
    pm = capture_pixmap(x, y, width, height, hwnd)
    if not pm or pm.isNull():
      dwarn("failed to capture image")
      return
    return cls(pixmap=pm, **kwargs)

# EOF
