# coding: utf8
# kagami.py
# 10/10/2012 jichi
#
# Gospel, Gossip, Grimoire, Omajinai, and Mirage

import textwrap
from functools import partial
from PySide.QtCore import Signal, Slot, Property, QObject, QTimer
from cconv.cconv import wide2thin
from zhszht.zhszht import zhs2zht
from mecabjlp import mecabfmt
from sakurakit import skdatetime
from sakurakit.skclass import classproperty, staticproperty
from sakurakit.skdebug import dprint, dwarn
#from sakurakit.skqml import QmlObject
#from sakurakit.sktr import tr_
#from msime import msime
from mytr import my
import bbcode, config, cabochaman, dataman, ebdict, features, growl, mecabman, qmldialog, rc, settings

## Text popup ##

class OcrPopupBean(QObject):

  instance = None

  def __init__(self, parent=None):
    super(OcrPopupBean, self).__init__(parent)
    OcrPopupBean.instance = self
    dprint("pass")

  popupRequested = Signal(unicode, unicode, int, int)  # text, language, x, y

class OcrPopupController:

  def showPopup(self, text, language, x, y): # unicode, unicode, int, int
    if OcrPopupBean.instance:
      OcrPopupBean.instance.popupRequested.emit(text, language, x, y)

## Grimoire ##

class _GrimoireBean:

  def __init__(self):
    self.features = {} # {unicode text:(unicode feature, fmt)}, recent mecab features

  #def renderJapanese(self, text, feature, **kwargs):
  #  return mecabman.rendertable(text, features=self.features if feature else None, **kwargs);
    #return ''.join(self._renderLine(t, feature=feature, **kwargs)
    #    for t in self._wrapText(text, charPerLine))

  #def _renderLine(self, text, feature, **kwargs):
  #  return mecabman.rendertable(text, termEnabled=True, features=self.features if feature else None, **kwargs);
    #if not msimeEnabled or len(text) > msime.IME_MAX_SIZE:
    #  mecab = True
    #elif not msime.ja_valid():
    #  growl.warn("<br/>".join((
    #    my.tr("Missing Microsoft Japanese IME."),
    #    my.tr("Use MeCab Japanese parser instead."),
    #  )))
    #  mecab = True
    #else:
    #  mecab = False
    #if mecab:
    #  ret = mecabman.rendertable(text, termEnabled=True, features=self.features if feature else None, **kwargs);
    #else:
    #  ret = msimejlp.render(text, **kwargs);
    #  if not ret and not mecab:
    #    ret = mecabman.rendertable(text, termEnabled=True, features=self.features if feature else None, **kwargs);
    #return ret

  #@staticmethod
  #def _wrapText(text, charPerLine):
  #  if charPerLine > 4 and charPerLine < len(text):
  #    return textwrap.wrap(text, charPerLine)
  #  else:
  #    return [text]

#@QmlObject
class GrimoireBean(QObject):

  instance = None

  def __init__(self, parent=None):
    super(GrimoireBean, self).__init__(parent)
    self.__d = _GrimoireBean()
    GrimoireBean.instance = self
    dprint("pass")

  clear = Signal()
  pageBreak = Signal()
  showText = Signal(unicode, unicode, long)  # text, lang, timestamp
  showTranslation = Signal(unicode, unicode, unicode, long)  # text, lang, provider, timestamp
  showComment = Signal(QObject) # dataman.Comment

  showNameText = Signal(unicode, unicode)  # text, lang
  showNameTranslation = Signal(unicode, unicode, unicode)  # text, lang, provider

  @Slot(unicode, bool, unicode, unicode, int, unicode, bool, bool, result=unicode)
  def renderJapanese(self, text, caboChaEnabled, furiType, meCabDic, charPerLine, rubySize, colorize, center):
    """
    @return  unicode  html
    """
    d = self.__d
    feature = colorize # keep features when colorized
    if feature and d.features:
      d.features = {}
    fmt = mecabfmt.getfmt(meCabDic)
    render = cabochaman.rendertable if caboChaEnabled else mecabman.rendertable
    return ''.join(
        render(t, termEnabled=True, features=d.features if feature else None,
            fmt=fmt, furiType=furiType, charPerLine=charPerLine, rubySize=rubySize, colorize=colorize, center=center)
        for t in text.split('\n') if t)

  @Slot(unicode, result=unicode)
  def convertChinese(self, text): return zhs2zht(text)

  def lookupFeature(self, text):
    """
    @param  text  unicode
    @return  unicode  or None
    """
    return self.__d.features.get(text)

class _GrimoireController:

  def __init__(self, q):
    self.timestamp = 0 # long
    self._queue = [] # list of functions
    self.nameClosure = None # function
    self.nameTrClosure = None # function
    self.nameMissing = False # bool
    self.nameTrMissing = False # bool

    t = self.flushTimer = QTimer(q)
    t.setSingleShot(True)
    #t.setInterval(100) # btw, ITH's split time is 200
    t.timeout.connect(self._flush)

  def append(self, func):
    self._queue.append(func)
    self.flushTimer.start(100) # TextHook interval is 200

  def clear(self):
    if self._queue:
      self._queue = []
    self.nameClosure = None
    self.nameTrClosure = None
    self.nameMissing = False
    self.nameTrMissing = False

  def _flush(self):
    if self._queue:
      #map(apply, self._queue)  # for loop is faster
      for it in self._queue: apply(it)
      #self._queue = []
    if self.nameClosure:
      self.nameClosure()
    if self.nameTrClosure:
      self.nameTrClosure()
    self.clear()

class GrimoireController(QObject):

  def __init__(self, parent=None):
    super(GrimoireController, self).__init__(parent)
    self.__d = _GrimoireController(self)

  def clear(self):
    self.__d.clear()
    self.__d.timestamp = 0
    GrimoireBean.instance.clear.emit()

  def pageBreak(self):
    ss = settings.global_()
    if (ss.isGrimoireTextVisible() or
        ss.isGrimoireNameVisible() or
        ss.isGrimoireTranslationVisible() or
        ss.isGrimoireSubtitleVisible()):
      self.__d.append(GrimoireBean.instance.pageBreak.emit)

  # Always send text
  def showText(self, text, language, timestamp):
    #if not settings.global_().isGrimoireTextVisible():
    #  return

    #if settings.global_().isMsimeCorrectionEnabled():
    #  text = msime.to_kanji(text) or text
    d = self.__d
    d.timestamp = timestamp
    d.append(partial(GrimoireBean.instance.showText.emit,
        text, language, timestamp))
    if d.nameClosure:
      d.append(d.nameClosure)
      d.nameClosure = None
      d.nameMissing = False
      if d.nameTrClosure:
        d.append(d.nameTrClosure)
        d.nameTrClosure = None
        d.nameTrMissing = False
      else:
        d.nameTrMissing = True
    else:
      d.nameMissing = True

  def showTranslation(self, text, language, provider, timestamp):
    if not settings.global_().isGrimoireTranslationVisible():
      return

    if self.__d.timestamp > timestamp:
      dprint("translation comes too late, ignored")
      return
    text = text.replace('\n', '<br/>')
    self.__d.append(partial(GrimoireBean.instance.showTranslation.emit,
        text, language, provider, timestamp))

  def showComment(self, c):
    if (settings.global_().isGrimoireSubtitleVisible() and
        c.type == 'subtitle' and not c.disabled and not c.deleted):
      self.__d.append(partial(GrimoireBean.instance.showComment.emit,
          c))

  # Always send name text, this could help flush the text
  def showNameText(self, text, language):
    #if not settings.global_().isGrimoireNameVisible(): #and not ss.isGrimoireTextVisible():
    #  return

    closure = partial(GrimoireBean.instance.showNameText.emit,
        text, language)
    d = self.__d
    d.nameClosure = None
    d.nameTrClosure = None
    if d.nameMissing:
      d.append(closure)
      d.nameMissing = False
      d.nameTrMissing = True
    else:
      d.nameClosure = closure
      d.nameTrMissing = False
      d.flushTimer.start(800)

  def showNameTranslation(self, text, language, provider):
    ss = settings.global_()
    if not ss.isGrimoireNameVisible() or not ss.isGrimoireTranslationVisible():
      return

    closure = partial(GrimoireBean.instance.showNameTranslation.emit,
        text, language, provider)
    d = self.__d
    d.nameTrClosure = None
    if d.nameTrMissing:
      d.append(closure)
      d.nameTrMissing = True
    else:
      d.nameTrClosure = closure
      d.flushTimer.start(800)

## Gospel ##

#@QmlObject
class GospelBean(QObject):

  instance = None

  def __init__(self, parent=None):
    super(GospelBean, self).__init__(parent)
    GospelBean.instance = self
    dprint("pass")

  clear = Signal()
  hide = Signal()
  #showMessage = Signal(unicode, unicode)  # text, lang
  showComment = Signal(QObject) # compatible with dataman.Comment

  @Slot(unicode, result=unicode)
  def convertChinese(self, text): return zhs2zht(text)

class GospelController:

  @staticmethod
  def clear(): GospelBean.instance.clear.emit()
  @staticmethod
  def hide(): GospelBean.instance.hide.emit()
  @staticmethod
  def showComment(c):
    if (features.USER_COMMENT and settings.global_().isGrimoireCommentVisible() and
        c.type == 'popup' and not c.disabled and not c.deleted):
      GospelBean.instance.showComment.emit(c)

## Gossip ##

#@QmlObject
class GossipBean(QObject):

  instance = None

  def __init__(self, parent=None):
    super(GossipBean, self).__init__(parent)
    GossipBean.instance = self
    dprint("pass")

  clear = Signal()
  #showMessage = Signal(unicode, unicode)  # text, lang
  showComment = Signal(QObject) # compatible with dataman.Comment

  @Slot(unicode, result=unicode)
  def convertChinese(self, text): return zhs2zht(text)

class GossipController:

  @staticmethod
  def clear(): GossipBean.instance.clear.emit()
  @staticmethod
  def showComment(c):
    if (features.USER_COMMENT and settings.global_().isGrimoireCommentVisible() and
        c.type == 'comment' and not c.disabled and not c.deleted):
      GossipBean.instance.showComment.emit(c)

## Omajinai ##

#@QmlObject
class OmajinaiBean(QObject):

  instance = None

  def __init__(self, parent=None):
    super(OmajinaiBean, self).__init__(parent)
    OmajinaiBean.instance = self
    dprint("pass")

  clear = Signal()
  showComment = Signal(QObject) # dataman.Comment

  @Slot(unicode, result=unicode)
  def convertChinese(self, text): return zhs2zht(text)

class _OmajinaiController:
  def __init__(self, q):
    self._queue = [] # [dataman.Comment]

    t = self._shiftTimer = QTimer(q)
    t.setSingleShot(True)
    t.setInterval(500)
    t.timeout.connect(self._shift)

  def _shift(self):
    if self._queue:
      c = self._queue.pop(0)
      OmajinaiBean.instance.showComment.emit(c)
      if self._queue:
        self._shiftTimer.start()

  def append(self, c):
    self._queue.append(c)
    if len(self._queue) == 1:
      self._shiftTimer.start()

  def clear(self):
    if self._queue:
      self._queue = []

class OmajinaiController(QObject):

  def __init__(self, parent=None):
    super(OmajinaiController, self).__init__(parent)
    self.__d = _OmajinaiController(self)

  def clear(self):
    self.__d.clear()
    OmajinaiBean.instance.clear.emit()

  def showComment(self, c):
    if (features.USER_COMMENT and settings.global_().isGrimoireDanmakuVisible() and
        c.type == 'danmaku' and not c.disabled and not c.deleted):
      self.__d.append(c)

## Mirage ##

class _MirageBean:

  def __init__(self):
    self.features = {} # {unicode text:(unicode feature, fmt)}, recent mecab features

  #def renderJapanese(self, text, feature, **kwargs):
  #  return mecabman.rendertable(text, features=self.features if feature else None, **kwargs);
    #return ''.join(cls._renderLine(t, **kwargs)
    #    for t in cls._wrapText(text, charPerLine))

  #@staticmethod
  #def _renderLine(text, **kwargs):
  #  return mecabman.rendertable(text, **kwargs) # termEnabled = False
    #if not msimeEnabled or len(text) > msime.IME_MAX_SIZE:
    #  jlp = mecabman
    #elif not msime.ja_valid():
    #  growl.warn("<br/>".join((
    #    my.tr("Missing Microsoft Japanese IME."),
    #    my.tr("Use MeCab Japanese parser instead."),
    #  )))
    #  jlp = mecabman
    #else:
    #  jlp = msimejlp
    #ret = jlp.render(text, furiType=furiType, rubySize=rubySize, colorize=colorize, center=center);
    #if not ret and jlp is msimejlp:
    #  ret = mecabman.rendertable(text, furiType=furiType, rubySize=rubySize, colorize=colorize, center=center);
    #return ret

  #@staticmethod
  #def _wrapText(text, charPerLine):
  #  if charPerLine > 4 and charPerLine < len(text):
  #    return textwrap.wrap(text, charPerLine)
  #  else:
  #    return [text]

#@QmlObject
class MirageBean(QObject):

  instance = None

  def __init__(self, parent=None):
    super(MirageBean, self).__init__(parent)
    self.__d = _MirageBean()
    MirageBean.instance = self
    dprint("pass")

  show = Signal()
  clear = Signal()
  pageBreak = Signal()
  showText = Signal(unicode, unicode, long)  # text, lang, timestamp
  showTranslation = Signal(unicode, unicode, unicode, long)  # text, lang, provider, timestamp

  @Slot(unicode, bool, unicode, unicode, int, unicode, bool, bool, result=unicode)
  def renderJapanese(self, text, caboChaEnabled, furiType, meCabDic, charPerLine, rubySize, colorize, center):
    """
    @return  unicode  html
    """
    d = self.__d
    feature = colorize
    if feature and d.features:
      d.features = {}
    render = cabochaman.rendertable if caboChaEnabled else mecabman.rendertable
    fmt = mecabfmt.getfmt(meCabDic)
    return ''.join( # disable term by default
        render(t, termEnabled=False, features=d.features if feature else None,
            furiType=furiType, charPerLine=charPerLine, rubySize=rubySize, colorize=colorize, center=center)
        for t in text.split('\n') if t) or text # return the original text if failed

  @Slot(unicode, result=unicode)
  def convertChinese(self, text): return zhs2zht(text)

class _MirageController:

  def __init__(self, q):
    self._queue = [] # list of functions

    t = self.flushTimer = QTimer(q)
    t.setSingleShot(True)
    #t.setInterval(100) # btw, ITH's split time is 200
    t.timeout.connect(self._flush)

  def append(self, func):
    self._queue.append(func)
    self.flushTimer.start(100) # TextHook interval is 200

  def clear(self):
    if self._queue:
      self._queue = []

  def _flush(self):
    if self._queue:
      #map(apply, self._queue)  # for loop is faster
      for it in self._queue: apply(it)
      self._queue = []

class MirageController(QObject):

  def __init__(self, parent=None):
    super(MirageController, self).__init__(parent)
    self.__d = _MirageController(self)

  def show(self): MirageBean.instance.show.emit()

  def clear(self):
    self.__d.clear()
    MirageBean.instance.clear.emit()

  def pageBreak(self):
    self.__d.append(MirageBean.instance.pageBreak.emit)

  def showText(self, text, language, timestamp):
    #if settings.global_().isMsimeCorrectionEnabled():
    #  text = msime.to_kanji(text) or text
    self.__d.append(partial(MirageBean.instance.showText.emit,
        text, language, timestamp))

  def showTranslation(self, text, language, provider, timestamp):
    text = text.replace('\n', '<br/>')
    self.__d.append(partial(MirageBean.instance.showTranslation.emit,
        text, language, provider, timestamp))

# EOF
