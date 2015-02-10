# coding: utf8
# trman.py
# 10/8/2012 jichi
#
# Terminology:
# - data: raw game text byte code
# - text: original game text
# - translation: machine translation
# - comment: user's subtitle or comment

#from sakurakit.skprof import SkProfiler

import itertools
from functools import partial
from PySide.QtCore import QObject, Signal, Slot, Qt
from sakurakit import skevents, skthreads
from sakurakit.skclass import  memoized, memoizedproperty, hasmemoizedproperty
from sakurakit.skdebug import dprint, dwarn
import features, textutil
import _trman

#@Q_Q
class _TranslatorManager(object):

  def __init__(self, abortSignal):
    self.abortSignal = abortSignal # signal

    self.online = False
    self.language = 'en' # str, user language

    self.yueEnabled = False # translate zh to yue

    self.allTranslators = [] # all created translators
    self.marked = False

    self.infoseekEnabled = \
    self.exciteEnabled = \
    self.bingEnabled = \
    self.googleEnabled = \
    self.naverEnabled = \
    self.baiduEnabled = \
    self.lecOnlineEnabled = \
    self.transruEnabled = \
    self.hanVietEnabled = \
    self.jbeijingEnabled = \
    self.fastaitEnabled = \
    self.dreyeEnabled = \
    self.ezTransEnabled = \
    self.atlasEnabled = \
    self.lecEnabled = \
    True # bool

    self.alignEnabled = {} # {str key:bool t}
    self.scriptEnabled = {} # {str key:bool t}

    from PySide.QtNetwork import QNetworkAccessManager
    nam = QNetworkAccessManager() # parent is not assigned
    from qtrequests import qtrequests
    self.session = qtrequests.Session(nam, abortSignal=self.abortSignal)

  normalizeText = staticmethod(textutil.normalize_punct)

  def getAlignEnabled(self, key): return self.alignEnabled.get(key) or False # str -> bool
  def setAlignEnabled(self, key, t): self.alignEnabled[key] = t # str, bool ->

  def getScriptEnabled(self, key): return self.scriptEnabled.get(key) or False # str -> bool
  def setScriptEnabled(self, key, t): # str, bool ->
    if self.scriptEnabled.get(key) != t:
      self.scriptEnabled[key] = t
      if self.hasTranslator(key):
        mt = self.getTranslator(key)
        dprint("clear cache for %s" % key)
        mt.clearCache()

  def postprocess(self, text, language):
    if self.yueEnabled and language.startswith('zh') and self.online:
      ret = self.yueTranslator.translate(text, fr=language)
      if ret:
        return ret
    return text

  def _newtr(self, tr):
    """
    @param  tr  _trman.Translator
    """
    self.allTranslators.append(tr)
    return tr

  @memoizedproperty
  def yueTranslator(self): # no an independent machine translator
    return _trman.YueTranslator(
          abortSignal=self.abortSignal, session=self.session)

  @memoizedproperty
  def atlasTranslator(self): return self._newtr(_trman.AtlasTranslator())

  @memoizedproperty
  def lecTranslator(self): return self._newtr(_trman.LecTranslator())

  @memoizedproperty
  def ezTranslator(self): return self._newtr(_trman.EzTranslator())

  @memoizedproperty
  def fastaitTranslator(self): return self._newtr(_trman.FastAITTranslator(
      postprocess=self.postprocess))

  @memoizedproperty
  def dreyeTranslator(self): return self._newtr(self._newtr(_trman.DreyeTranslator(
      postprocess=self.postprocess)))

  @memoizedproperty
  def jbeijingTranslator(self): return self._newtr(_trman.JBeijingTranslator(
      postprocess=self.postprocess))

  @memoizedproperty
  def hanVietTranslator(self): return self._newtr(_trman.HanVietTranslator())

  @memoizedproperty
  def googleTranslator(self):
    return self._newtr(_trman.GoogleTranslator(
        abortSignal=self.abortSignal,
        session=self.session,
        postprocess=self.postprocess))

  @memoizedproperty
  def bingTranslator(self):
    return self._newtr(_trman.BingTranslator(
        abortSignal=self.abortSignal,
        session=self.session,
        postprocess=self.postprocess))

  @memoizedproperty
  def baiduTranslator(self):
    return self._newtr(_trman.BaiduTranslator(
        abortSignal=self.abortSignal,
        session=self.session,
        postprocess=self.postprocess))

  @memoizedproperty
  def naverTranslator(self):
    return self._newtr(_trman.NaverTranslator(
        abortSignal=self.abortSignal,
        session=self.session,
        postprocess=self.postprocess))

  @memoizedproperty
  def lecOnlineTranslator(self):
    return self._newtr(_trman.LecOnlineTranslator(
        abortSignal=self.abortSignal,
        session=self.session))

  @memoizedproperty
  def transruTranslator(self):
    return self._newtr(_trman.TransruTranslator(
        abortSignal=self.abortSignal,
        session=self.session))

  @memoizedproperty
  def infoseekTranslator(self):
    return self._newtr(_trman.InfoseekTranslator(
        abortSignal=self.abortSignal,
        session=self.session))

  @memoizedproperty
  def exciteTranslator(self):
    return self._newtr(_trman.ExciteTranslator(
        abortSignal=self.abortSignal,
        session=self.session))

  @staticmethod
  def translateAndApply(func, kw, tr, text, align=None, **kwargs):
    """
    @param  func  function to apply
    @param  kw  arguments passed to func
    @param  tr  function to translate
    @param  text  unicode  passed to tr
    @param* align  list or None
    @param  **kwargs  passed to tr
    """
    # TODO: I might be able to do runsync here instead of within tr
    #async = kwargs.get('async')
    #if async:
    #  kwargs['async'] = False
    #  r = skthreads.runsync(partial(tr, *args, **kwargs),
    #      abortSignal=self.abortSignal)
    #else:
    r = tr(text, align=align, **kwargs)
    if r and r[0]:
      func(r[0], r[1], r[2], align, **kw)

  def _getTranslatorPropertyName(self, key):
    """
    @param  key  str
    @return  str
    """
    if key == 'eztrans':
      return 'ezTranslator'
    if key == 'lecol':
      return 'lecOnlineTranslator'
    if key == 'transru':
      return 'transruTranslator'
    if key == 'hanviet':
      return 'hanVietTranslator'
    return key + 'Translator'

  def getTranslator(self, key):
    """
    @param  key  str
    @return  TranslatorEngine or None
    """
    v = self._getTranslatorPropertyName(key)
    try: return getattr(self, v)
    except AttributeError: pass

  def hasTranslator(self, key):
    """
    @param  key  str
    @return  TranslatorEngine or None
    """
    v = self._getTranslatorPropertyName(key)
    return hasmemoizedproperty(self, v)

  def iterOfflineTranslators(self):
    """
    @yield  Translator
    """
    if self.jbeijingEnabled: yield self.jbeijingTranslator
    if self.fastaitEnabled: yield self.fastaitTranslator
    if self.dreyeEnabled: yield self.dreyeTranslator
    if self.ezTransEnabled: yield self.ezTranslator
    if self.hanVietEnabled: yield self.hanVietTranslator
    if self.lecEnabled: yield self.lecTranslator
    if self.atlasEnabled: yield self.atlasTranslator

  def iterOnlineTranslators(self, reverse=False):
    """
    @param* reverse  bool
    @yield  Translator
    """
    if self.online:
      if reverse:
        if self.infoseekEnabled: yield self.infoseekTranslator
        if self.exciteEnabled: yield self.exciteTranslator
        if self.lecOnlineEnabled: yield self.lecOnlineTranslator
        if self.transruEnabled: yield self.transruTranslator
        if self.googleEnabled: yield self.googleTranslator
        if self.bingEnabled: yield self.bingTranslator
        if self.naverEnabled: yield self.naverTranslator
        if self.baiduEnabled: yield self.baiduTranslator
      else:
        if self.baiduEnabled: yield self.baiduTranslator
        if self.naverEnabled: yield self.naverTranslator
        if self.bingEnabled: yield self.bingTranslator
        if self.googleEnabled: yield self.googleTranslator
        if self.transruEnabled: yield self.transruTranslator
        if self.lecOnlineEnabled: yield self.lecOnlineTranslator
        if self.exciteEnabled: yield self.exciteTranslator
        if self.infoseekEnabled: yield self.infoseekTranslator

  def iterTranslators(self):
    """
    @yield  Translator
    """
    return itertools.chain(self.iterOfflineTranslators(), self.iterOnlineTranslators())

class TranslatorManager(QObject):

  ## Construction ##

  def __init__(self, parent=None):
    super(TranslatorManager, self).__init__(parent)
    self.__d = _TranslatorManager(abortSignal=self.onlineAbortionRequested)

    self.clearCacheRequested.connect(self.clearCache, Qt.QueuedConnection)

  ## Signals ##

  onlineAbortionRequested = Signal()
  #infoseekAbortionRequested = Signal()
  #bingAbortionRequested = Signal()
  #baiduAbortionRequested = Signal()

  # Logs
  languagesReceived = Signal(unicode, unicode) # fr, to
  normalizedTextReceived = Signal(unicode) # text after applying translation replacement scripts
  sourceTextReceived = Signal(unicode) # text after applying source terms
  escapedTextReceived = Signal(unicode) # text after preparing escaped terms
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
    for it in self.__d.allTranslators:
      it.clearCache()
    dprint("pass")

  ## Properties ##

  def isMarked(self): return self.__d.marked
  def setMarked(self, t): self.__d.marked = t

  def language(self): return self.__d.language
  def setLanguage(self, value): self.__d.language = value

  def isOnline(self): return self.__d.online
  def setOnline(self, value): self.__d.online = value

  def isYueEnabled(self): return self.__d.yueEnabled
  def setYueEnabled(self, value): self.__d.yueEnabled = value

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

  def isNaverEnabled(self): return self.__d.naverEnabled
  def setNaverEnabled(self, value): self.__d.naverEnabled = value

  def isLecOnlineEnabled(self): return self.__d.lecOnlineEnabled
  def setLecOnlineEnabled(self, value): self.__d.lecOnlineEnabled = value

  def isTransruEnabled(self): return self.__d.transruEnabled
  def setTransruEnabled(self, value): self.__d.transruEnabled = value

  def isHanVietEnabled(self): return self.__d.hanVietEnabled
  def setHanVietEnabled(self, value): self.__d.hanVietEnabled = value

  def isJBeijingEnabled(self): return self.__d.jbeijingEnabled
  def setJBeijingEnabled(self, value): self.__d.jbeijingEnabled = value

  def isFastaitEnabled(self): return self.__d.fastaitEnabled
  def setFastaitEnabled(self, value): self.__d.fastaitEnabled = value

  def isDreyeEnabled(self): return self.__d.dreyeEnabled
  def setDreyeEnabled(self, value): self.__d.dreyeEnabled = value

  def isEzTransEnabled(self): return self.__d.ezTransEnabled
  def setEzTransEnabled(self, value): self.__d.ezTransEnabled = value

  def isLecEnabled(self): return self.__d.lecEnabled
  def setLecEnabled(self, value): self.__d.lecEnabled = value

  def isAtlasEnabled(self): return self.__d.atlasEnabled
  def setAtlasEnabled(self, value): self.__d.atlasEnabled = value

  # Script

  def isAtlasScriptEnabled(self): return self.__d.getScriptEnabled('atlas')
  def setAtlasScriptEnabled(self, t): self.__d.setScriptEnabled('atlas', t)

  def isLecScriptEnabled(self): return self.__d.getScriptEnabled('lec')
  def setLecScriptEnabled(self, t): self.__d.setScriptEnabled('lec', t)

  def isLecOnlineScriptEnabled(self): return self.__d.getScriptEnabled('lecol')
  def setLecOnlineScriptEnabled(self, t): self.__d.setScriptEnabled('lecol', t)

  def isBingScriptEnabled(self): return self.__d.getScriptEnabled('bing')
  def setBingScriptEnabled(self, t): self.__d.setScriptEnabled('bing', t)

  def isGoogleScriptEnabled(self): return self.__d.getScriptEnabled('google')
  def setGoogleScriptEnabled(self, t): self.__d.setScriptEnabled('google', t)

  def isInfoseekScriptEnabled(self): return self.__d.getScriptEnabled('infoseek')
  def setInfoseekScriptEnabled(self, t): self.__d.setScriptEnabled('infoseek', t)

  def isExciteScriptEnabled(self): return self.__d.getScriptEnabled('excite')
  def setExciteScriptEnabled(self, t): self.__d.setScriptEnabled('excite', t)

  def isTransruScriptEnabled(self): return self.__d.getScriptEnabled('transru')
  def setTransruScriptEnabled(self, t): self.__d.setScriptEnabled('transru', t)

  # Alignment

  def isBingAlignEnabled(self): return self.__d.getAlignEnabled('bing')
  def setBingAlignEnabled(self, t): self.__d.setAlignEnabled('bing', t)

  def isGoogleAlignEnabled(self): return self.__d.getAlignEnabled('google')
  def setGoogleAlignEnabled(self, t): self.__d.setAlignEnabled('google', t)

  def isBaiduAlignEnabled(self): return self.__d.getAlignEnabled('baidu')
  def setBaiduAlignEnabled(self, t): self.__d.setAlignEnabled('baidu', t)

  def isNaverAlignEnabled(self): return self.__d.getAlignEnabled('naver')
  def setNaverAlignEnabled(self, t): self.__d.setAlignEnabled('naver', t)

  def isInfoseekAlignEnabled(self): return self.__d.getAlignEnabled('infoseek')
  def setInfoseekAlignEnabled(self, t): self.__d.setAlignEnabled('infoseek', t)

  def isHanVietAlignEnabled(self): return self.__d.getAlignEnabled('hanviet')
  def setHanVietAlignEnabled(self, t): self.__d.setAlignEnabled('hanviet', t)

  ## Queries ##

  def warmup(self, to='', fr='ja'):
    if features.MACHINE_TRANSLATION:
      for it in self.__d.iterOfflineTranslators():
        dprint("warm up %s" % it.key)
        it.warmup(to=to, fr=fr)

  def hasOnlineTranslators(self):
    """
    @return  bool
    """
    d = self.__d
    return any((
      d.baiduEnabled,
      d.naverEnabled,
      d.googleEnabled,
      d.bingEnabled,
      d.lecOnlineEnabled,
      d.transruEnabled,
      d.infoseekEnabled,
      d.exciteEnabled,
    ))

  def hasOfflineTranslators(self):
    """
    @return  bool
    """
    d = self.__d
    return any((
      d.hanVietEnabled,
      d.jbeijingEnabled,
      d.fastaitEnabled,
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

  def enabledEngines(self, fr=''): # str -> [str]
    d = self.__d
    r = []
    if d.hanVietEnabled and (not fr or fr.startswith('zh')):
      r.append('hanviet')
    if d.jbeijingEnabled and (not fr or fr in ('ja', 'zhs', 'zht')):
      r.append('jbeijing')

    if d.fastaitEnabled: r.append('fastait')
    if d.dreyeEnabled: r.append('dreye')
    if d.ezTransEnabled: r.append('eztrans')
    if d.lecEnabled: r.append('lec')
    if d.atlasEnabled: r.append('atlas')

    if d.baiduEnabled: r.append('baidu')
    if d.naverEnabled: r.append('naver')
    if d.googleEnabled: r.append('google')
    if d.bingEnabled: r.append('bing')
    if d.lecOnlineEnabled: r.append('lecol')
    if d.transruEnabled: r.append('transru')
    if d.infoseekEnabled: r.append('infoseek')
    if d.exciteEnabled: r.append('excite')
    return r

  def isEnabled(self):
    """
    @return  bool
    """
    return features.MACHINE_TRANSLATION and self.hasTranslators()

  def guessTranslationLanguage(self): # -> str
    if not self.isEnabled():
      return ''
    d = self.__d
    if d.hanVietEnabled:
      return 'vi'
    if d.jbeijingEnabled or d.baiduEnabled or d.fastaitEnabled or d.dreyeEnabled:
      return 'zhs' if d.language == 'zhs' else 'zht'
    if d.ezTransEnabled or d.naverEnabled:
      return 'ko'
    if (d.atlasEnabled or d.lecEnabled) and not any((
        d.infoseekEnabled,
        d.transruEnabled,
        d.exciteEnabled,
        d.bingEnabled,
        d.googleEnabled,
        d.lecOnlineEnabled,
      )):
      return 'en'
    return d.language

  def translate(self, *args, **kwargs):
    """
    @return  unicode
    """
    return self.translateOne(*args, **kwargs)[0]

  def translateTest(self, text, fr='ja', engine='', async=False):
    """
    @param  text  unicode
    @param* fr  unicode  language
    @param* to  unicode  language
    @param* async  bool
    @return  unicode or None
    """
    #if not features.MACHINE_TRANSLATION or not text:
    if not text:
      return ''
    d = self.__d
    kw = {
      'fr': fr,
      'to': d.language,
      'async': async,
    }
    #text = d.normalizeText(text)
    e = d.getTranslator(engine)
    if e:
      return e.translateTest(text, **kw)
    #dwarn("invalid translator: %s" % engine)
    for it in d.iterOfflineTranslators():
      return it.translateTest(text, **kw)
    for it in d.iterOnlineTranslators():
      return it.translateTest(text, **kw)

  def translateOne(self, text, fr='ja', engine='', mark=None, online=True, async=False, cached=True, emit=False, scriptEnabled=None):
    """Translate using any translator
    @param  text  unicode
    @param* fr  unicode  language
    @param* to  unicode  language
    @param* mark  bool or None
    @param* async  bool
    @param* online  bool
    @param* emit  bool  whether emit intermediate results
    @param* scriptEnabled  bool or None  whether enable the translation script
    @param* cached  bool  NOT USED, always cached
    @return  unicode sub or None, unicode lang, unicode provider
    """
    if not features.MACHINE_TRANSLATION or not text:
      return None, None, None
    d = self.__d
    to = d.language
    kw = {
      'fr': fr,
      'to': to,
      'mark': mark,
      'async': async,
      'emit': emit,
    }
    text = d.normalizeText(text)
    if engine:
      e = d.getTranslator(engine)
      if e:
        kw['scriptEnabled'] = d.getScriptEnabled(e.key) if scriptEnabled is None else scriptEnabled
        return e.translate(text, **kw)
      #dwarn("invalid translator: %s" % engine)
    for it in d.iterOfflineTranslators():
      kw['scriptEnabled'] = d.getScriptEnabled(it.key) if scriptEnabled is None else scriptEnabled
      return it.translate(text, **kw)
    for it in d.iterOnlineTranslators():
      kw['scriptEnabled'] = d.getScriptEnabled(it.key) if scriptEnabled is None else scriptEnabled
      if emit or not it.asyncSupported:
        return it.translate(text, **kw)
      else: # not emit and asyncSupported
        kw['async'] = False # force using single thread
        return skthreads.runsync(partial(it.translate, text, **kw),
          abortSignal=self.onlineAbortionRequested,
        ) or (None, None, None)
    return None, None, None

  def translateApply(self, func, text, fr='ja', mark=None, scriptEnabled=None, **kwargs):
    """Specialized for textman
    @param  func  function(unicode sub, unicode lang, unicode provider)
    @param  text  unicode
    @param* fr  unicode  language
    @param* mark  bool or None
    @param* kwargs  pass to func
    """
    if not features.MACHINE_TRANSLATION or not text:
      return
    d = self.__d
    to = d.language

    text = d.normalizeText(text)
    if mark is None:
      mark = d.marked

    for it in d.iterOfflineTranslators():
      align = [] if it.alignSupported and d.getAlignEnabled(it.key) else None
      script = d.getScriptEnabled(it.key) if scriptEnabled is None else scriptEnabled
      #with SkProfiler(): # 0.3 seconds
      r = it.translate(text, fr=fr, to=to, mark=mark, align=align, scriptEnabled=script, async=False)
      #with SkProfiler(): # 0.0004 seconds
      if r and r[0]:
        func(r[0], r[1], r[2], align, **kwargs)

    # Always disable async
    for it in d.iterOnlineTranslators(reverse=True): # need reverse since skevents is used
      align = [] if it.alignSupported and d.getAlignEnabled(it.key) else None
      script = d.getScriptEnabled(it.key) if scriptEnabled is None else scriptEnabled
      skevents.runlater(partial(d.translateAndApply,
          func, kwargs, it.translate, text, fr=fr, to=to, mark=mark, align=align, scriptEnabled=script, async=False))

@memoized
def manager(): return TranslatorManager()

#def translate(*args, **kwargs):
#  return manager().translate(*args, **kwargs)

class TranslatorCoffeeBean(QObject):
  def __init__(self, parent=None):
    super(TranslatorCoffeeBean, self).__init__(parent)

  @Slot(result=unicode)
  def translators(self): # -> [str translator_name]
    return ','.join(manager().enabledEngines())

  @Slot(unicode, unicode, result=unicode)
  def translate(self, text, engine):
    # I should not hardcode fr = 'ja' here
    # Force async
    # Translate direct to disable Shared Dictionary
    return manager().translateTest(text, engine=engine, async=True) or ''

class TranslatorQmlBean(QObject):
  def __init__(self, parent=None):
    super(TranslatorQmlBean, self).__init__(parent)

  @Slot(unicode, result=unicode)
  def translators(self, language): # str -> [str translator_name]
    return ','.join(manager().enabledEngines(language))

  @Slot(unicode, unicode, unicode, result=unicode)
  def translate(self, text, language, engine):
    # I should not hardcode fr = 'ja' here
    # Force async
    return manager().translate(text, engine=engine, fr=language, mark=False, async=True, scriptEnabled=False) or ''

# EOF
