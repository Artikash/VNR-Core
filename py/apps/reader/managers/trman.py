# coding: utf8
# trman.py
# 10/8/2012 jichi
#
# Terminology:
# - data: raw game text byte code
# - text: original game text
# - translation: machine translation
# - comment: user's subtitle or comment

import os, itertools
from functools import partial
from PySide.QtCore import QObject, Signal, Slot, Qt
from sakurakit import skevents
from sakurakit.skclass import Q_Q, memoized, memoizedproperty
from sakurakit.skdebug import dprint, dwarn
import features, textutil
import _trman

@Q_Q
class _TranslatorManager(object):

  def __init__(self, q):
    #self.convertsChinese = False
    self.online = False
    self.language = 'en' # str, user language

    self.infoseekEnabled = \
    self.exciteEnabled = \
    self.bingEnabled = \
    self.googleEnabled = \
    self.baiduEnabled = \
    self.lecOnlineEnabled = \
    self.lougoEnabled = \
    self.jbeijingEnabled = \
    self.dreyeEnabled = \
    self.ezTransEnabled = \
    self.atlasEnabled = \
    self.lecEnabled = \
    True # bool

  normalizeText = staticmethod(textutil.normalize_punct)

  @memoizedproperty
  def lougoTranslator(self):
    return _trman.LougoTranslator()

  @memoizedproperty
  def atlasTranslator(self):
    return _trman.AtlasTranslator(parent=self.q)

  @memoizedproperty
  def lecTranslator(self):
    return _trman.LecTranslator(parent=self.q)

  @memoizedproperty
  def ezTranslator(self):
    return _trman.EzTranslator(parent=self.q)

  @memoizedproperty
  def dreyeTranslator(self):
    return _trman.DreyeTranslator(parent=self.q)

  @memoizedproperty
  def jbeijingTranslator(self):
    return _trman.JBeijingTranslator(parent=self.q)

  @memoizedproperty
  def baiduTranslator(self):
    return _trman.BaiduTranslator(parent=self.q, abortSignal=self.q.onlineAbortionRequested)

  @memoizedproperty
  def googleTranslator(self):
    return _trman.GoogleTranslator(parent=self.q, abortSignal=self.q.onlineAbortionRequested)

  @memoizedproperty
  def bingTranslator(self):
    return _trman.BingTranslator(parent=self.q, abortSignal=self.q.onlineAbortionRequested)

  @memoizedproperty
  def lecOnlineTranslator(self):
    return _trman.LecOnlineTranslator(parent=self.q, abortSignal=self.q.onlineAbortionRequested)

  @memoizedproperty
  def infoseekTranslator(self):
    return _trman.InfoseekTranslator(parent=self.q, abortSignal=self.q.onlineAbortionRequested)

  @memoizedproperty
  def exciteTranslator(self):
    return _trman.ExciteTranslator(parent=self.q, abortSignal=self.q.onlineAbortionRequested)

  @staticmethod
  def translateAndApply(func, tr, *args, **kwargs):
    """
    @param  func  function to apply
    @param  tr  function to translate
    @param  *args  passed to tr
    @param  **kwargs  passed to tr
    """
    r = tr(*args, **kwargs)
    if r[0]: func(*r)

  def getTranslator(self, key):
    """
    @param  key  str
    @return  TranslatorEngine or None
    """
    if key == 'eztrans':
      return self.ezTranslator
    if key == 'lecol':
      return self.lecOnlineTranslator
    if key == 'lou':
      return self.lougoTranslator
    try: return getattr(self, key + 'Translator')
    except AttributeError: pass

  def iterOfflineTranslators(self):
    """
    @yield  Translator
    """
    #if self.lougoEnabled: yield self.lougoTranslator
    if self.jbeijingEnabled: yield self.jbeijingTranslator
    if self.dreyeEnabled: yield self.dreyeTranslator
    if self.ezTransEnabled: yield self.ezTranslator
    if self.lecEnabled: yield self.lecTranslator
    if self.atlasEnabled: yield self.atlasTranslator

  def iterOnlineTranslators(self):
    """Iterate reversely
    @yield  Translator
    """
    if self.online:
      if self.infoseekEnabled: yield self.infoseekTranslator
      if self.exciteEnabled: yield self.exciteTranslator
      if self.lecOnlineEnabled: yield self.lecOnlineTranslator
      if self.bingEnabled: yield self.bingTranslator
      if self.googleEnabled: yield self.googleTranslator
      if self.baiduEnabled: yield self.baiduTranslator

  def iterTranslators(self):
    """
    @yield  Translator
    """
    return itertools.chain(self.iterOfflineTranslators(), self.iterOnlineTranslators())

class TranslatorManager(QObject):

  ## Construction ##

  def __init__(self, parent=None):
    super(TranslatorManager, self).__init__(parent)
    self.__d = _TranslatorManager(self)

    self.clearCacheRequested.connect(self.clearCache, Qt.QueuedConnection)

  ## Signals ##

  onlineAbortionRequested = Signal()
  #infoseekAbortionRequested = Signal()
  #bingAbortionRequested = Signal()
  #baiduAbortionRequested = Signal()

  # Logs
  languagesReceived = Signal(unicode, unicode) # fr, to
  sourceTextReceived = Signal(unicode)      # text after applying source terms
  escapedTextReceived = Signal(unicode)    # text after preparing escaped terms
  splitTextsReceived = Signal(list)  # texts after splitting
  splitTranslationsReceived = Signal(list)  # translations after applying translation
  jointTranslationReceived = Signal(unicode)  # translation before applying terms
  escapedTranslationReceived = Signal(unicode)  # translation after unescaping terms
  targetTranslationReceived = Signal(unicode)  # translation after applying target terms

  clearCacheRequested = Signal() # async

  def abortOnline(self):
    self.onlineAbortionRequested.emit()
    #for sig in self.infoseekAbortionRequested, self.bingAbortionRequested, self.baiduAbortionRequested:
    #  sig.emit()

  def clearCache(self):
    for it in self.__d.iterTranslators():
      it.reset()
    dprint("pass")

  ## Properties ##

  def language(self): return self.__d.language
  def setLanguage(self, value): self.__d.language = value

  def isOnline(self): return self.__d.online
  def setOnline(self, value): self.__d.online = value

  def isInfoseekEnabled(self): return self.__d.infoseekEnabled
  def setInfoseekEnabled(self, value): self.__d.infoseekEnabled = value

  def isExciteEnabled(self): return self.__d.exciteEnabled
  def setExciteEnabled(self, value): self.__d.exciteEnabled = value

  def isGoogleEnabled(self): return self.__d.googleEnabled
  def setGoogleEnabled(self, value): self.__d.googleEnabled = value

  def isBingEnabled(self): return self.__d.bingEnabled
  def setBingEnabled(self, value): self.__d.bingEnabled = value

  def isBaiduEnabled(self): return self.__d.baiduEnabled
  def setBaiduEnabled(self, value): self.__d.baiduEnabled = value

  def isLecOnlineEnabled(self): return self.__d.lecOnlineEnabled
  def setLecOnlineEnabled(self, value): self.__d.lecOnlineEnabled = value

  def isLougoEnabled(self): return self.__d.lougoEnabled
  def setLougoEnabled(self, value): self.__d.lougoEnabled = value

  def isJBeijingEnabled(self): return self.__d.jbeijingEnabled
  def setJBeijingEnabled(self, value): self.__d.jbeijingEnabled = value

  def isDreyeEnabled(self): return self.__d.dreyeEnabled
  def setDreyeEnabled(self, value): self.__d.dreyeEnabled = value

  def isEzTransEnabled(self): return self.__d.ezTransEnabled
  def setEzTransEnabled(self, value): self.__d.ezTransEnabled = value

  def isLecEnabled(self): return self.__d.lecEnabled
  def setLecEnabled(self, value): self.__d.lecEnabled = value

  def isAtlasEnabled(self): return self.__d.atlasEnabled
  def setAtlasEnabled(self, value): self.__d.atlasEnabled = value

  ## Queries ##

  def warmup(self):
    if features.MACHINE_TRANSLATION:
      for it in self.__d.iterOfflineTranslators():
        dprint("warm up %s" % it.key)
        it.warmup()

  def hasOnlineTranslators(self):
    """
    @return  bool
    """
    d = self.__d
    return any((
      d.baiduEnabled,
      d.googleEnabled,
      d.bingEnabled,
      d.lecOnlineEnabled,
      d.infoseekEnabled,
      d.exciteEnabled,
    ))

  def hasOfflineTranslators(self):
    """
    @return  bool
    """
    d = self.__d
    return any((
      #d.lougoEnabled,
      d.jbeijingEnabled,
      d.dreyeEnabled,
      d.ezTransEnabled,
      d.lecEnabled,
      d.atlasEnabled,
    ))

  def hasTranslators(self):
    """
    @return  bool
    """
    return self.hasOnlineTranslators() or self.hasOfflineTranslators()

  def isEnabled(self):
    """
    @return  bool
    """
    return features.MACHINE_TRANSLATION and self.hasTranslators()

  def translate(self, *args, **kwargs):
    """
    @return  unicode
    """
    return self.translateOne(*args, **kwargs)[0] or ''

  def translateDirect(self, text, fr='ja', engine='', async=False):
    """
   Test @param  text  unicode
    @param  fr  unicode  language
    @param  to  unicode  language
    @param  async  bool
    @return  unicode sub
    """
    #if not features.MACHINE_TRANSLATION or not text:
    if not text:
      return ''
    d = self.__d
    e = d.getTranslator(engine)
    if e:
      return e.translateTest(text, fr=fr, to=d.language, async=async)
    dwarn("invalid translator: %s" % engine)
    return ''

  def translateOne(self, text, fr='ja', engine='', online=True, async=False, cached=True, emit=False):
    """Translate using any translator
    @param  text  unicode
    @param  fr  unicode  language
    @param  to  unicode  language
    @param  async  bool
    @param  online  bool
    @param  emit  bool  whether emit intermediate results
    @param  cached  bool  NOT USED, always cached
    @return  unicode sub or None, unicode lang, unicode provider
    """
    if not features.MACHINE_TRANSLATION or not text:
      return None, None, None
    d = self.__d
    text = d.normalizeText(text)
    if engine:
      e = d.getTranslator(engine)
      if e:
        return e.translate(text, fr=fr, to=d.language, async=async, emit=emit)
      dwarn("invalid translator: %s" % engine)
    for it in d.iterOfflineTranslators():
      return it.translate(text, fr=fr, to=d.language, async=async, emit=emit)
    for it in d.iterOnlineTranslators():
      return it.translate(text, fr=fr, to=d.language, async=True, emit=emit)
    return None, None, None

  def translateApply(self, func, text, fr='ja'):
    """Specialized for textman
    @param  text  unicode
    @param  fr  unicode  language
    @param  to  unicode  language
    @param  func  function(unicode sub, unicode lang, unicode provider)
    """
    if not features.MACHINE_TRANSLATION or not text:
      return
    d = self.__d
    text = d.normalizeText(text)

    for it in d.iterOfflineTranslators():
      r = it.translate(text, fr=fr, to=d.language, async=False)
      if r[0]: func(*r)

    for it in d.iterOnlineTranslators():
      skevents.runlater(partial(d.translateAndApply,
          func, it.translate, text, fr=fr, to=d.language, async=True))

@memoized
def manager(): return TranslatorManager()

#def translate(*args, **kwargs):
#  return manager().translate(*args, **kwargs)

class YakuCoffeeBean(QObject):
  def __init__(self, parent=None):
    super(YakuCoffeeBean, self).__init__(parent)

  @Slot(result=bool)
  def enabled(self):
    return manager().isEnabled()

  @Slot(unicode, result=unicode)
  def yaku(self, t):
    """
    @param  t  unicode
    @return  unicode
    """
    return manager().translate(t, async=True)

# EOF

#  def _translateOnlineByBing(self, text):
#    """
#    @param  text  unicode
#    @return  unicode sub, unicode lang, unicode provider
#    """
#    lang = self.language
#    text = self._applySourceTerms(text, lang)
#    text = self._prepareEscapeTerms(text, lang)
#    text = self._prepareSentenceTransformation(text)
#    sub = skthreads.runsync(partial(
#        bingtrans.translate, text, to=lang),
#        parent=self.q)
#    if sub:
#      if bingtrans.bad_translation(sub):
#        growl.error(self.q.tr(
#"""So many users are using Microsoft translator this month that
#VNR has run out of free monthly quota TT
#Please try a different translator in Preferences instead.
#"""))
#        sub = ""
#      else:
#        sub = self._applyEscapeTerms(sub, lang)
#        sub = self._applyTargetTerms(sub, lang)
#    return sub, lang, mytr_("Bing")
