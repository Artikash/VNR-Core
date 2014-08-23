# coding: utf8
# _trman.py
# 10/8/2012 jichi
#
# Terminology:
# - data: raw game text byte code
# - text: original game text
# - translation: machine translation
# - comment: user's subtitle or comment

import os, re
import requests
from functools import partial
from itertools import ifilter, imap
from time import time
from cconv.cconv import wide2thin, wide2thin_digit
from zhszht.zhszht import zhs2zht, zht2zhs
from sakurakit import skstr, skthreads, sktypes
from sakurakit.skclass import memoizedproperty
from sakurakit.skdebug import dwarn
from mytr import my, mytr_
import config, growl, mecabman, termman, trscriptman, textutil
import trman, trcache

from sakurakit.skprofiler import SkProfiler

__NO_DELIM = '' # no deliminators
_NO_SET = frozenset()
_NO_RE = re.compile('')

__PARAGRAPH_DELIM = u"【】「」♪" # machine translation of sentence deliminator
_PARAGRAPH_SET = frozenset(__PARAGRAPH_DELIM)
_PARAGRAPH_RE = re.compile(r"(%s)" % '|'.join(_PARAGRAPH_SET))

#__SENTENCE_DELIM = u"\n【】「」。♪" #…！？# machine translation of sentence deliminator
#_SENTENCE_SET = frozenset(__SENTENCE_DELIM)
_SENTENCE_RE = re.compile(ur"([。？！」\n])(?![。！？）」\n]|$)")

# All methods in this class are supposed to be thread-safe
# Though they are not orz
class TranslationCache:
  def __init__(self, maxSize=100, shrinkSize=30):
    """
    @param  maxSize  max data size
    @param  shrinkSize  data to delete when oversize
    """
    self.maxSize = maxSize
    self.shrinkSize = shrinkSize
    self.data = {} # {unicode text:[unicode sub, long timestamp]}

  def clear(self):
    if self.data:
      self.data = {}

  def get(self, key): # unicode -> unicode
    t = self.data.get(key)
    if t:
      t[1] = self._now()
      return t[0]

  def update(self, key, value): # unicode, unicode ->
    self.data[key] = [value, self._now()]
    if len(self.data) > self.maxSize:
      self._shrink()
    #return value

  @staticmethod
  def _now(): return long(time()) # -> long  msecs

  def _shrink(self):
    l = sorted(self.data.iteritems(), key=lambda it:it[1][1])
    self.data = {k:v for k,v in l[self.shrinkSize:]}

## Translators

class Translator(object):
  key = 'tr' # str

  def clearCache(self): pass

  def warmup(self): pass

  def translateTest(self, text, to='en', fr='ja', async=False):
    """
    @param  text  unicode
    @param  to  str
    @param* fr  str
    @param* async  bool
    @return  unicode sub
    """
    return ''

  def translate(self, text, to='en', fr='ja', async=False, emit=False, scriptEnabled=True):
    """
    @param  text  unicode
    @param  to  str
    @param* fr  str
    @param* async  bool
    @param* emit  bool
    @param* scriptEnabled  bool
    @return  (unicode sub or None, str lang or None, self.name or None)
    """
    return None, to, self.key

  # Emits

  def emitLanguages(self, fr, to):
    trman.manager().languagesReceived.emit(fr, to)
  def emitNormalizedText(self, t):
    trman.manager().normalizedTextReceived.emit(t)
  def emitSourceText(self, t):
    trman.manager().sourceTextReceived.emit(t)
  def emitEscapedText(self, t):
    trman.manager().escapedTextReceived.emit(t)
  def emitJointTranslation(self, t):
    trman.manager().jointTranslationReceived.emit(t)
  def emitEscapedTranslation(self, t):
    trman.manager().escapedTranslationReceived.emit(t)
  def emitTargetTranslation(self, t):
    trman.manager().targetTranslationReceived.emit(t)
  def emitSplitTexts(self, l):
    trman.manager().splitTextsReceived.emit(l)
  def emitSplitTranslations(self, l):
    trman.manager().splitTranslationsReceived.emit(l)

class LougoTranslator(Translator):
  key = 'lou' # override

  def translate(self, text, to='en', fr='ja', async=False, emit=False, scriptEnabled=True):
    """
    @param  text  unicode
    @param* fr  unicode
    @param* async  bool  ignored, always sync
    @param* emit  bool
    @param* scriptEnabled  bool
    @return  unicode sub, unicode lang, unicode provider
    """
    if emit:
      self.emitLanguages(fr='ja', to='en')
    if fr != 'ja':
      return None, None, None

    if scriptEnabled:
      t = text
      text = trscriptman.manager().normalizeText(text, fr=fr, to=to)
      if emit and text != t:
        self.emitNormalizedText(text)

    tm = termman.manager()
    t = text
    text = tm.applySourceTerms(text, 'en')
    if emit and text != t:
      self.emitSourceText(text)
    sub = mecabman.tolou(text)
    if sub:
      if emit:
        self.emitJointTranslation(text)
      sub = textutil.beautify_subtitle(sub)
    return sub, 'ja', self.key

  def translateTest(self, text, to='en', fr='ja', async=False):
    """
    @param  text  unicode
    @param* fr  unicode
    @param* async  bool  ignored, always sync
    @return  unicode sub, unicode lang, unicode provider
    """
    return mecabman.tolou(text)

class HanVietTranslator(Translator):
  key = 'hanviet' # override

  def translate(self, text, to='vi', fr='zhs', async=False, emit=False, scriptEnabled=True):
    """
    @param  text  unicode
    @param* fr  unicode
    @param* async  bool  ignored, always sync
    @param* emit  bool
    @param* scriptEnabled  bool  ignored
    @return  unicode sub, unicode lang, unicode provider
    """
    if emit:
      self.emitLanguages(fr=fr, to='vi')
    #if not fr.startswith('zh'):
    #  return None, None, None
    if fr == 'zht':
      text = zht2zhs(text)

    #if scriptEnabled:
    #  sm = trscriptman.manager()
    #  t = text
    #  text = sm.normalizeText(text, fr=fr, to=to)
    #  if emit and text != t:
    #    self.emitNormalizedText(text)

    tm = termman.manager()
    t = text
    text = tm.applySourceTerms(text, 'zhs')
    if emit and text != t:
      self.emitSourceText(text)

    from hanviet.hanviet import han2viet
    sub = han2viet(text)
    if sub:
      if emit:
        self.emitJointTranslation(sub)
      sub = textutil.beautify_subtitle(sub)
    return sub, 'vi', self.key

  def translateTest(self, text, to='vi', fr='zhs', async=False):
    """
    @param  text  unicode
    @param* fr  unicode
    @param* async  bool  ignored, always sync
    @return  unicode sub, unicode lang, unicode provider
    """
    from hanviet.hanviet import han2viet
    return han2viet(text)

# Machine translators

class MachineTranslator(Translator):

  splitsSentences = False # bool
  persistentCaching = False # bool  whether use sqlite to cache the translation

  #_CACHE_LENGTH = 10 # length of the translation to cache

  #_DELIM_SET = _PARAGRAPH_SET # set of deliminators
  #_DELIM_RE = _PARAGRAPH_RE   # rx of deliminators

  def __init__(self, parent=None, abortSignal=None):
    super(MachineTranslator, self).__init__()
    self.cache = TranslationCache()  # public overall translation cache
    self._cache = TranslationCache() # private translation cache for internal translation
    self.parent = parent  # QObject
    self.abortSignal = abortSignal # QtCore.Signal abort translation

  def clearCache(self):
    """@reimp"""
    self.cache.clear()
    self._cache.clear()

  def __cachedtr(self, text, async, tr, fr, to, **kwargs):
    """
    @param  text  unicode
    @param  async  bool
    @param  tr  function(unicode text, str to, str fr)
    @param  fr  str
    @param  to  str
    @return  unicode or None
    """
    #if len(text) > self._CACHE_LENGTH:
    #  return skthreads.runsync(partial(
    #       tr, text, **kwargs),
    #       abortSignal=self.abortSignal,
    #       parent=self.parent) if async else tr(text, **kwargs)
    ret = self._cache.get(text)
    if ret:
      return ret

    if self.persistentCaching:
      #with SkProfiler(): # takes about 0.03 to create, 0.02 to insert
      ret = trcache.get(key=self.key, fr=fr, to=to, text=text)
      if ret:
        self._cache.update(text, ret)
        return ret

    ret = skthreads.runsync(partial(
        tr, text, fr=fr, to=to, **kwargs),
        abortSignal=self.abortSignal,
        parent=self.parent) if async else tr(text, fr=fr, to=to, **kwargs)

    if ret:
      self._cache.update(text, ret)
      if self.persistentCaching:
        #with SkProfiler(): # takes about 0.003
        trcache.add(key=self.key, fr=fr, to=to, text=text, translation=ret)

    return ret

  def __tr(self, text, *args, **kwargs):
    """
    @param  t  unicode
    @return  unicode or None
    """
    # Current max length of escaped char is 12
    return ('' if not text else
        text if len(text) == 1 and text in _PARAGRAPH_SET or len(text) <= 12 and sktypes.is_float(text) else
        self.__cachedtr(text, *args, **kwargs))

  def _itertexts(self, text):
    """
    @param  text  unicode
    @yield  unicode
    """
    for line in ifilter(textutil.skip_empty_line,
        (it.strip() for it in _PARAGRAPH_RE.split(text))):
      if not self.splitsSentences:
        yield line
      else:
        for sentence in _SENTENCE_RE.sub(r"\1\n", line).split("\n"):
          yield sentence

  def _itertranslate(self, text, tr, async=False, **kwargs):
    """
    @param  text  unicode
    @param  tr  function(text, to, fr)
    @param  async  bool
    @param  kwargs  arguments passed to tr
    @yield  unicode
    """
    for line in self._itertexts(text):
      t = self.__tr(line,
          async, tr, # *args
          **kwargs)
      if t is None:
        dwarn("translation failed or aborted using '%s'" % self.key)
        break
      yield t # 10/10/2013: maybe, using generator instead would be faster?

  def _translate(self, emit, *args, **kwargs):
    """
    @param  emit  bool
    @param  text  unicode
    @param  tr  function(text, to, fr)
    @param  async  bool
    @param  kwargs  arguments passed to tr
    @return  unicode
    """
    #delim = ' ' if self.splitsSentences else ''
    if emit:
       l = list(self._itertranslate(*args, **kwargs))
       self.emitSplitTranslations(l)
       return ''.join(l)
    else:
      return ''.join(self._itertranslate(*args, **kwargs))

  def _translateTest(self, fn, text, async=False, **kwargs):
    """
    @param  fn  translator
    @param  text  unicode
    @param* fr  unicode
    @param* async  bool  ignored, always sync
    @return  unicode sub, unicode lang, unicode provider
    """
    return skthreads.runsync(partial(fn, text, **kwargs)) if async else fn(text, **kwargs)

  def _escapeText(self, text, to, fr, emit, scriptEnabled):
    """
    @param  text  unicode
    @param  to  str  language
    @param  fr  str  language
    @param  emit  bool
    @param  scriptEnabled  bool
    @return  unicode
    """
    if scriptEnabled:
      # 8/19/2014: Only test 0.007 second, with or without locks
      #with SkProfiler():
      t = text
      text = trscriptman.manager().normalizeText(text, fr=fr, to=to)
      if emit and text != t:
        self.emitNormalizedText(text)

    tm = termman.manager()
    t = text
    text = tm.applySourceTerms(text, to)
    if emit and text != t:
      self.emitSourceText(text)

    t = text
    #with SkProfiler():
    text = tm.prepareEscapeTerms(text, to)
    if emit and text != t:
      self.emitEscapedText(text)
    if emit:
      l = list(self._itertexts(text))
      self.emitSplitTexts(l)
    return text

  def _unescapeTranslation(self, text, to, emit):
    """
    @param  text  unicode
    @param  to  str  language
    @param* emit  bool
    @return  unicode
    """
    tm = termman.manager()
    if emit:
      self.emitJointTranslation(text)
    #text = self.__google_repl_after(text)
    t = text
    #with SkProfiler():
    text = tm.applyEscapeTerms(text, to)
    if emit and text != t:
      self.emitEscapedTranslation(text)
    t = text
    text = tm.applyTargetTerms(text, to)
    if emit and text != t:
      self.emitTargetTranslation(text)
    #text = text.replace("( ", '(')
    #text = text.replace(u"\n】", u"】\n")
    text = textutil.beautify_subtitle(text)
    return text

class OfflineMachineTranslator(MachineTranslator):
  persistentCaching = False # bool  disable sqlite
  def __init__(self, *args, **kwargs):
    super(OfflineMachineTranslator, self).__init__(*args, **kwargs)

class OnlineMachineTranslator(MachineTranslator):
  persistentCaching = True # bool  enable sqlite
  def __init__(self, *args, **kwargs):
    super(OnlineMachineTranslator, self).__init__(*args, **kwargs)

## Offline

# Note:
# There are trailing spaces after each translation for Atlas.
# But there are no trailing spaces for LEC.
class AtlasTranslator(OfflineMachineTranslator):
  key = 'atlas' # override
  splitsSentences = True
  #_DELIM_SET = _SENTENCE_SET # override
  #_DELIM_RE = _SENTENCE_RE # override

  def __init__(self, **kwargs):
    super(AtlasTranslator, self).__init__(**kwargs)
    self.engine = self.createengine()
    self._warned = False # bool

  @staticmethod
  def createengine():
    from atlas import atlas
    ret = atlas.create_engine()
    ok = ret.load()
    #import atexit
    #atexit.register(ret.destroy)
    if ok:
      growl.msg(my.tr("ATLAS translator is loaded"))
    else:
      growl.error(my.tr("Cannot load {0} for machine translation. Please check Preferences/Location").format(mytr_("ATLAS")))
    return ret

  # This would cause issue?
  def warmup(self):
    """@reimp"""
    self.engine.warmup()

  def translateTest(self, text, to='en', fr='ja', async=False):
    """
    @param  text  unicode
    @param* fr  unicode
    @param* async  bool  ignored, always sync
    @return  unicode sub, unicode lang, unicode provider
    """
    try: return self._translateTest(self.engine.translate, text, async=async)
    except Exception, e:
      dwarn(e)
      growl.error(my.tr("Cannot load {0} for machine translation. Please check Preferences/Location").format(mytr_("ATLAS")),
          async=async)
      return ''

  def translate(self, text, to='en', fr='ja', async=False, emit=False, scriptEnabled=True):
    """@reimp"""
    to = 'en'
    if emit:
      self.emitLanguages(fr='ja', to=to)
    if fr != 'ja':
      return None, None, None
    if not emit:
      repl = self.cache.get(text)
      if repl:
        return repl, to, self.key
    repl = self._escapeText(text, to, fr, emit, scriptEnabled)
    if repl:
      try:
        repl = self._translate(emit, repl, self.engine.translate, async=async)
        if repl:
          # ATLAS always try to append period at the end
          repl = wide2thin(repl) #.replace(u". 。", ". ").replace(u"。", ". ").replace(u" 」", u"」").rstrip()
          repl = self._unescapeTranslation(repl, to=to, emit=emit)
          repl = repl.replace(u" 」", u"」") # remove the trailing space
          self.cache.update(text, repl)
          return repl, to, self.key
      #except RuntimeError, e:
      except Exception, e:
        if not self._warned:
          self._warned = True
          dwarn(e)
          if not async:
            growl.error(my.tr("Cannot load {0} for machine translation. Please check Preferences/Location").format(mytr_("ATLAS")),
                async=async)
    return None, None, None

class LecTranslator(OfflineMachineTranslator):
  key = 'lec' # override
  splitsSentences = True
  #_DELIM_SET = _SENTENCE_SET # override
  #_DELIM_RE = _SENTENCE_RE # override

  def __init__(self, **kwargs):
    super(LecTranslator, self).__init__(**kwargs)
    self.engine = self.createengine()
    self._warned = False # bool

  @staticmethod
  def createengine():
    from lec import powertrans
    ret = powertrans.create_engine()
    ok = ret.load()
    #import atexit
    #atexit.register(ret.destroy)
    if ok:
      growl.msg(my.tr("LEC translator is loaded"))
    else:
      growl.error(my.tr("Cannot load {0} for machine translation. Please check Preferences/Location").format(mytr_("LEC")))
    return ret

  # This would cause issue?
  def warmup(self):
    """@reimp"""
    self.engine.warmup()

  def translateTest(self, text, to='en', fr='ja', async=False):
    """
    @param  text  unicode
    @param* fr  unicode
    @param* async  bool  ignored, always sync
    @return  unicode sub, unicode lang, unicode provider
    """
    try: return self._translateTest(self.engine.translate, text, async=async)
    except Exception, e:
      dwarn(e)
      growl.error(my.tr("Cannot load {0} for machine translation. Please check Preferences/Location").format(mytr_("LEC")),
          async=async)
      return ''

  def translate(self, text, to='en', fr='ja', async=False, emit=False, scriptEnabled=True):
    """@reimp"""
    to = 'en'
    if emit:
      self.emitLanguages(fr='ja', to=to)
    if fr != 'ja':
      return None, None, None
    if not emit:
      repl = self.cache.get(text)
      if repl:
        return repl, to, self.key
    repl = self._escapeText(text, to, fr, emit, scriptEnabled)
    if repl:
      try:
        repl = self._translate(emit, repl, self.engine.translate, async=async)
        if repl:
          repl = wide2thin(repl) #.replace(u"。", ". ").replace(u" 」", u"」").rstrip()
          repl = self._unescapeTranslation(repl, to=to, emit=emit)
          self.cache.update(text, repl)
          return repl, to, self.key
      #except RuntimeError, e:
      except Exception, e:
        if not self._warned:
          self._warned = True
          dwarn(e)
          if not async:
            growl.error(my.tr("Cannot load {0} for machine translation. Please check Preferences/Location").format(mytr_("LEC")),
                async=async)
    return None, None, None

class EzTranslator(OfflineMachineTranslator):
  key = 'eztrans' # override

  def __init__(self, **kwargs):
    super(EzTranslator, self).__init__(**kwargs)
    self.engine = self.createengine()
    self._warned = False # bool

  @staticmethod
  def createengine():
    from eztrans import eztrans
    ret = eztrans.create_engine()
    ok = ret.load()
    #import atexit
    #atexit.register(ret.destroy) # not needed by eztrans
    if ok:
      growl.msg(my.tr("ezTrans XP is loaded"))
    else:
      growl.error(my.tr("Cannot load {0} for machine translation. Please check Preferences/Location").format(mytr_("ezTrans XP")))
    return ret

  def warmup(self):
    """@reimp"""
    self.engine.warmup()

  def translateTest(self, text, to='en', fr='ja', async=False):
    """
    @param  text  unicode
    @param* fr  unicode
    @param* async  bool  ignored, always sync
    @return  unicode sub, unicode lang, unicode provider
    """
    try: return self._translateTest(self.engine.translate, text, async=async)
    except Exception, e:
      dwarn(e)
      growl.error(my.tr("Cannot load {0} for machine translation. Please check Preferences/Location").format(mytr_("ezTrans XP")),
          async=async)
      return ''

  def translate(self, text, to='ko', fr='ja', async=False, emit=False, scriptEnabled=True):
    """@reimp"""
    to = 'ko'
    if emit:
      self.emitLanguages(fr='ja', to=to)
    if fr != 'ja':
      return None, None, None
    if not emit:
      repl = self.cache.get(text)
      if repl:
        return repl, to, self.key
    repl = self._escapeText(text, to, fr, emit, scriptEnabled)
    if repl:
      try:
        repl = self._translate(emit, repl, self.engine.translate, async=async)
        if repl:
          repl = self._unescapeTranslation(repl, to=to, emit=emit)
          self.cache.update(text, repl)
          return repl, to, self.key
      #except RuntimeError, e:
      except Exception, e:
        if not self._warned:
          self._warned = True
          dwarn(e)
          if not async:
            growl.error(my.tr("Cannot load {0} for machine translation. Please check Preferences/Location").format(mytr_("ezTrans XP")),
                async=async)
    return None, None, None

class JBeijingTranslator(OfflineMachineTranslator):
  key = 'jbeijing' # override

  def __init__(self, **kwargs):
    super(JBeijingTranslator, self).__init__(**kwargs)
    self.engine = self.createengine()
    self._warned = False # bool

  @staticmethod
  def createengine():
    from jbeijing import jbeijing
    ret = jbeijing.create_engine()
    ok = ret.load()
    #import atexit
    #atexit.register(ret.destroy)
    if ok:
      growl.msg(my.tr("JBeijing translator is loaded"))

      def _filter(path): return bool(path) and os.path.exists(path + '.dic')
      paths = jbeijing.userdic()
      paths.extend(config.JCUSERDIC_LOCATIONS)
      paths = filter(_filter, paths)
      paths = paths[:3] # at most 3 dictionaries

      ok = ret.setUserDic(paths)

      def _tidy(path):
        path = os.path.dirname(path)
        d = os.path.basename(path)
        path = os.path.dirname(path)
        dd = os.path.basename(path)
        path = os.path.dirname(path)
        ddd = os.path.basename(path)
        return "* " + '/'.join((ddd, dd))
      if ok:
        dics = '<br/>'.join(imap(_tidy, paths))
        #dics = dics.replace('@Goodboyye','@goodboyye').replace('@Hdgdyl','@好大個的鴨梨').replace('@Zhugeqiu', '@諸葛秋')
        growl.msg('<br/>'.join((
          my.tr("Load user-defined dictionaries") + ":",
          dics
        )))
      else:
        dics = '<br/>'.join(imap(_tidy, jbeijing.userdic()))
        growl.error('<br/>'.join((
          my.tr("Failed to load user-defined dictionaries") + ":",
          dics
        )))
    else:
      growl.error(my.tr("Cannot load {0} for machine translation. Please check Preferences/Location").format(mytr_("JBeijing")))
    return ret

  def warmup(self):
    """@reimp"""
    self.engine.warmup()

  def translateTest(self, text, to='en', fr='ja', async=False):
    """
    @param  text  unicode
    @param* fr  unicode
    @param* async  bool  ignored, always sync
    @return  unicode sub, unicode lang, unicode provider
    """
    simplified = to == 'zhs'
    try: return self._translateTest(self.engine.translate, text, async=async, simplified=simplified)
    except Exception, e:
      dwarn(e)
      growl.error(my.tr("Cannot load {0} for machine translation. Please check Preferences/Location").format(mytr_("JBeijing")),
          async=async)
      return ''

  def translate(self, text, to='zhs', fr='ja', async=False, emit=False, scriptEnabled=True):
    """@reimp"""
    if fr != 'ja':
      if emit:
        self.emitLanguages(fr='ja', to=to)
      return None, None, None
    simplified = to == 'zhs'
    to = 'zhs' if simplified else 'zht'
    if not emit:
      repl = self.cache.get(text)
      if repl:
        return repl, to, self.key
    #with SkProfiler():
    repl = self._escapeText(text, to, fr, emit, scriptEnabled)
    if repl:
      repl = repl.replace('\n', ' ') # JBeijing cannot handle multiple lines
      try:
        #with SkProfiler():
        repl = self._translate(emit, repl, self.engine.translate, async=async, simplified=simplified)
        if repl:
          repl = wide2thin_digit(repl) # convert wide digits to thin digits
          #with SkProfiler():
          repl = self._unescapeTranslation(repl, to=to, emit=emit)
          self.cache.update(text, repl)
          return repl, to, self.key
      #except RuntimeError, e:
      except Exception, e:
        if self._warned:
          self._warned = True
          dwarn(e)
          if not async:
            growl.error(my.tr("Cannot load {0} for machine translation. Please check Preferences/Location").format(mytr_("JBeijing")),
                async=async)
    return None, None, None

class DreyeTranslator(OfflineMachineTranslator):
  key = 'dreye' # override

  def __init__(self, **kwargs):
    super(DreyeTranslator, self).__init__(**kwargs)
    self._warned = False # bool

  @memoizedproperty
  def jcEngine(self):
    from dreye import dreye
    ret = dreye.create_engine('ja')
    ok = ret.load()
    #import atexit
    #atexit.register(ret.destroy)
    if ok:
      growl.msg(my.tr("Dr.eye Japanese-Chinese translator is loaded"))
    else:
      growl.error(my.tr("Cannot load {0} for machine translation. Please check Preferences/Location").format(mytr_("Dr.eye")))
    return ret

  @memoizedproperty
  def ecEngine(self):
    from dreye import dreye
    ret = dreye.create_engine('en')
    ok = ret.load()
    #import atexit
    #atexit.register(ret.destroy)
    if ok:
      growl.msg(my.tr("Dr.eye English-Chinese translator is loaded"))
    else:
      growl.error(my.tr("Cannot load {0} for machine translation. Please check Preferences/Location").format(mytr_("Dr.eye")))
    return ret

  def translateTest(self, text, to='en', fr='ja', async=False):
    """
    @param  text  unicode
    @param* fr  unicode
    @param* async  bool  ignored, always sync
    @return  unicode sub, unicode lang, unicode provider
    """
    try:
      engine = self.jcEngine if fr == 'ja' else self.ecEngine
      return self._translateTest(engine.translate, text, to=to, fr=fr, async=async)
    except Exception, e:
      dwarn(e)
      growl.error(my.tr("Cannot load {0} for machine translation. Please check Preferences/Location").format(mytr_("Dr.eye")),
          async=async)
      return ''

  # Ignored
  #def warmup(self):
  #  """@reimp"""
  #  self.ecEngine.warmup()
  #  self.jcEngine.warmup()

  #__dreye_repl_after = staticmethod(skstr.multireplacer({
  #  '[': u'【',
  #  ' ]': u'】',
  #}))
  def translate(self, text, to='zhs', fr='ja', async=False, emit=False, scriptEnabled=True):
    """@reimp"""
    if fr == 'zht':
      text = zht2zhs(text)
    simplified = to == 'zhs'
    engine = self.jcEngine if fr == 'ja' else self.ecEngine
    if emit:
      self.emitLanguages(fr=fr, to=to)
    else:
      repl = self.cache.get(text)
      if repl:
        return repl, to, self.key
    repl = self._escapeText(text, to, fr, emit, scriptEnabled)
    if repl:
      try:
        repl = self._translate(emit, repl, engine.translate, async=async, to=to, fr=fr)
        if repl:
          if to != 'zhs':
            repl = zhs2zht(repl)
          #sub = self._applySentenceTransformation(sub)
          #sub = self.__dreye_repl_after(sub)
          #sub = sub.replace(']', u'】')
          repl = self._unescapeTranslation(repl, to=to, emit=emit)
          self.cache.update(text, repl)
          return repl, to, self.key
      #except RuntimeError, e:
      except Exception, e:
        if not self._warned:
          self._warned = True
          dwarn(e) # This might crash colorama TT
          if not async:
            growl.error(my.tr("Cannot load {0} for machine translation. Please check Preferences/Location").format(mytr_("Dr.eye")),
                async=async)
    return None, None, None

## Online

class InfoseekTranslator(OnlineMachineTranslator):
  key = 'infoseek' # override

  def __init__(self, **kwargs):
    super(InfoseekTranslator, self).__init__(**kwargs)
    from transer import infoseek
    infoseek.session = requests.Session()
    self.engine = infoseek

  #__infoseek_repl_after = staticmethod(skstr.multireplacer({
  #  '[': u'【',
  #  ']\n': u'】',
  #}))
  def translate(self, text, to='en', fr='ja', async=False, emit=False, scriptEnabled=True):
    """@reimp"""
    if fr != 'ja':
      return None, None, None
    to = 'en' if to in ('ms', 'ar', 'ru', 'nl', 'pl') else to
    if emit:
      self.emitLanguages(fr=fr, to=to)
    else:
      repl = self.cache.get(text)
      if repl:
        return repl, to, self.key
    repl = self._escapeText(text, to, fr, emit, scriptEnabled)
    if repl:
      repl = self._translate(emit, repl, self.engine.translate,
          to=to, fr=fr, async=async)
      if repl:
        repl = self._unescapeTranslation(repl, to=to, emit=emit)
        self.cache.update(text, repl)
    return repl, to, self.key

  def translateTest(self, text, to='en', fr='ja', async=False):
    """
    @param  text  unicode
    @param* fr  unicode
    @param* async  bool  ignored, always sync
    @return  unicode sub, unicode lang, unicode provider
    """
    try: return self._translateTest(self.engine.translate,
            text, to=to, fr=fr, async=async)
    except Exception, e: dwarn(e); return ''

class ExciteTranslator(OnlineMachineTranslator):
  key = 'excite' # override

  def __init__(self, **kwargs):
    super(ExciteTranslator, self).__init__(**kwargs)
    from excite import worldtrans
    worldtrans.session = requests.Session()
    self.engine = worldtrans

  #__excite_repl_after = staticmethod(skstr.multireplacer({
  #  '[': u'【',
  #  ']\n': u'】',
  #}))
  def translate(self, text, to='en', fr='ja', async=False, emit=False, scriptEnabled=True):
    """@reimp"""
    if fr != 'ja':
      return None, None, None
    to = 'en' if to in ('ms', 'id', 'th', 'vi', 'ar', 'nl', 'pl') else to
    if emit:
      self.emitLanguages(fr=fr, to=to)
    else:
      repl = self.cache.get(text)
      if repl:
        return repl, to, self.key
    repl = self._escapeText(text, to, fr, emit, scriptEnabled)
    if repl:
      repl = self._translate(emit, repl, self.engine.translate,
          to=to, fr=fr, async=async)
      if repl:
        repl = self._unescapeTranslation(repl, to=to, emit=emit)
        self.cache.update(text, repl)
    return repl, to, self.key

  def translateTest(self, text, to='en', fr='ja', async=False):
    """
    @param  text  unicode
    @param* fr  unicode
    @param* async  bool  ignored, always sync
    @return  unicode sub, unicode lang, unicode provider
    """
    try: return self._translateTest(self.engine.translate,
            text, to=to, fr=fr, async=async)
    except Exception, e: dwarn(e); return ''

class LecOnlineTranslator(OnlineMachineTranslator):
  key = 'lecol' # override

  def __init__(self, **kwargs):
    super(LecOnlineTranslator, self).__init__(**kwargs)

    from lec import leconline
    leconline.session = requests.Session()
    self.engine = leconline

  def translate(self, text, to='en', fr='ja', async=False, emit=False, scriptEnabled=True):
    """@reimp"""
    to = 'en' if to in ('ms', 'th', 'vi') else to
    if emit:
      self.emitLanguages(fr=fr, to=to)
    else:
      repl = self.cache.get(text)
      if repl:
        return repl, to, self.key
    repl = self._escapeText(text, to, fr, emit, scriptEnabled)
    if repl:
      repl = self._translate(emit, repl, self.engine.translate,
          to=to, fr=fr, async=async)
      if repl:
        repl = self._unescapeTranslation(repl, to=to, emit=emit)
        self.cache.update(text, repl)
    return repl, to, self.key

  def translateTest(self, text, to='en', fr='ja', async=False):
    """
    @param  text  unicode
    @param* fr  unicode
    @param* async  bool  ignored, always sync
    @return  unicode sub, unicode lang, unicode provider
    """
    try: return self._translateTest(self.engine.translate,
            text, to=to, fr=fr, async=async)
    except Exception, e: dwarn(e); return ''

class TransruTranslator(OnlineMachineTranslator):
  key = 'transru' # override

  def __init__(self, **kwargs):
    super(TransruTranslator, self).__init__(**kwargs)

    from promt import transru
    transru.session = requests.Session()
    self.engine = transru

  def translate(self, text, to='en', fr='ja', async=False, emit=False, scriptEnabled=True):
    """@reimp"""
    to = 'en' if to not in ('ja', 'en', 'ru', 'it', 'fr', 'de', 'pt', 'es') else to
    if emit:
      self.emitLanguages(fr=fr, to=to)
    else:
      repl = self.cache.get(text)
      if repl:
        return repl, to, self.key
    repl = self._escapeText(text, to, fr, emit, scriptEnabled)
    if repl:
      repl = self._translate(emit, repl, self.engine.translate,
          to=to, fr=fr, async=async)
      if repl:
        repl = self._unescapeTranslation(repl, to=to, emit=emit)
        self.cache.update(text, repl)
    return repl, to, self.key

  def translateTest(self, text, to='en', fr='ja', async=False):
    """
    @param  text  unicode
    @param* fr  unicode
    @param* async  bool  ignored, always sync
    @return  unicode sub, unicode lang, unicode provider
    """
    try: return self._translateTest(self.engine.translate,
            text, to=to, fr=fr, async=async)
    except Exception, e: dwarn(e); return ''

class GoogleTranslator(OnlineMachineTranslator):
  key = 'google' # override

  def __init__(self, **kwargs):
    super(GoogleTranslator, self).__init__(**kwargs)

    from google import googletrans
    googletrans.session = requests.Session()
    self.engine = googletrans

  #__google_repl_after = staticmethod(skstr.multireplacer({
  #  '...': u'…',
  #}))
  def translate(self, text, to='en', fr='ja', async=False, emit=False, scriptEnabled=True):
    """@reimp"""
    if emit:
      self.emitLanguages(fr=fr, to=to)
    else:
      repl = self.cache.get(text)
      if repl:
        return repl, to, self.key
    repl = self._escapeText(text, to, fr, emit, scriptEnabled)
    if repl:
      repl = self._translate(emit, repl, self.engine.translate,
          to=to, fr=fr, async=async)
      if repl:
        repl = self._unescapeTranslation(repl, to=to, emit=emit)
        if to.startswith('zh'):
          repl = repl.replace("...", u'…')
        self.cache.update(text, repl)
    return repl, to, self.key

  def translateTest(self, text, to='en', fr='ja', async=False):
    """
    @param  text  unicode
    @param* fr  unicode
    @param* async  bool  ignored, always sync
    @return  unicode sub, unicode lang, unicode provider
    """
    try: return self._translateTest(self.engine.translate,
            text, to=to, fr=fr, async=async) #.decode('utf8', errors='ignore')
    except Exception, e: dwarn(e); return ''

class BingTranslator(OnlineMachineTranslator):
  key = 'bing' # override

  def __init__(self, **kwargs):
    super(BingTranslator, self).__init__(**kwargs)

    # It is dangerous to create engine here, which is async
    from microsoft import bingtrans
    bingtrans.session = requests.Session()
    self.engine = bingtrans.create_engine() # time-limited

  #@memoizedproperty
  #def engine(self)I:
  #  from microsoft import bingtrans
  #  bingtrans.session = requests.Session()
  #  self.engine = bingtrans.create_engine() # time-limited

  #__bing_repl_after = staticmethod(skstr.multireplacer({
  #  '[': u'【',
  #  ']\n': u'】',
  #}))
  def translate(self, text, to='en', fr='ja', async=False, emit=False, scriptEnabled=True):
    """@reimp"""
    #if fr != 'ja':
    #  return None, None, None
    if emit:
      self.emitLanguages(fr=fr, to=to)
    else:
      repl = self.cache.get(text)
      if repl:
        return repl, to, self.key
    repl = self._escapeText(text, to, fr, emit, scriptEnabled)
    if repl:
      repl = self._translate(emit, repl, self.engine.translate,
          to=to, fr=fr, async=async)
      if repl:
        repl = self._unescapeTranslation(repl, to=to, emit=emit)
        self.cache.update(text, repl)
    return repl, to, self.key

  def translateTest(self, text, to='en', fr='ja', async=False):
    """
    @param  text  unicode
    @param* fr  unicode
    @param* async  bool  ignored, always sync
    @return  unicode sub, unicode lang, unicode provider
    """
    try: return self._translateTest(self.engine.translate, text, to=to, fr=fr, async=async)
    except Exception, e: dwarn(e); return ''

class BaiduTranslator(OnlineMachineTranslator):
  key = 'baidu' # override

  def __init__(self, **kwargs):
    super(BaiduTranslator, self).__init__(**kwargs)

    from kingsoft import iciba
    iciba.session = requests.Session()
    self.iciba = iciba

    from baidu import baidufanyi
    baidufanyi.session = requests.Session()
    self.baidufanyi = baidufanyi

  def getEngine(self, fr, to):
    """
    @param  fr  str
    @param  to  str
    @return baidu.baidufanyi or kingsoft.iciba
    """
    if fr == 'ja' and to.startswith('zh'):
      return self.iciba
    else:
      return self.baidufanyi

  __baidu_repl_before = staticmethod(skstr.multireplacer({
    #u'【': u'‘', # open single quote
    #u'】': u'’：', # close single quote
    #u'「': u'‘“', # open single double quote
    #u'」': u'”’', # close double single quote
    u'『': u'“‘', # open double single quote
    u'』': u'’”', # close single double quote

  }))
  __baidu_repl_after = staticmethod(skstr.multireplacer({
    #u'‘': u'【', # open single quote
    #u'’：': u'】', # close single quote
    #u'‘“': u'「', # open single double quote
    #u'”’': u'」', # close double single quote
    u'“‘': u'『', # open double single quote
    u'’”': u'』', # close single double quote
  }))
  def translate(self, text, to='zhs', fr='ja', async=False, emit=False, scriptEnabled=True):
    """@reimp"""
    #if fr not in ('ja', 'en', 'zhs', 'zht'):
    #  return None, None, None
    if emit:
      self.emitLanguages(fr=fr, to=to)
    #if lang not in ('zhs', 'zht', 'ja', 'en'):
    #  return None, None, None
    else:
      repl = self.cache.get(text)
      if repl:
        return repl, to, self.key
    repl = self._escapeText(text, to, fr, emit, scriptEnabled)
    if repl:
      engine = self.getEngine(fr=fr, to=to)
      repl = self.__baidu_repl_before(repl)
      repl = self._translate(emit, repl, engine.translate,
          to=to, fr=fr, async=async)
      if repl:
        if to == 'zht':
          repl = zhs2zht(repl)
        repl = self.__baidu_repl_after(repl)
        repl = self._unescapeTranslation(repl, to=to, emit=emit)
        self.cache.update(text, repl)
    return repl, to, self.key

  def translateTest(self, text, to='en', fr='ja', async=False):
    """
    @param  text  unicode
    @param* fr  unicode
    @param* async  bool  ignored, always sync
    @return  unicode sub, unicode lang, unicode provider
    """
    engine = self.getEngine(fr=fr, to=to)
    try: return self._translateTest(engine.translate,
            text, to=to, fr=fr, async=async)
    except Exception, e: dwarn(e); return ''

# EOF

#from youdao import youdaofanyi
#
#class YoudaoTranslator(OnlineMachineTranslator):
#  key = 'youdao' # override
#
#  def __init__(self, **kwargs):
#    super(YoudaoTranslator, self).__init__(**kwargs)
#
#  __youdao_repl_before = staticmethod(skstr.multireplacer({
#    #u'【': u'‘', # open single quote
#    #u'】': u'’：', # close single quote
#    #u'「': u'“', # open double quote
#    #u'」': u'”', # close double quote
#    u'『': u'“‘', # open double single quote
#    u'』': u'’”', # close single double quote
#
#  }))
#  __youdao_repl_after = staticmethod(skstr.multireplacer({
#    #u'‘': u'【', # open single quote
#    #u'’：': u'】', # close single quote
#    #u'“': u'「', # open double quote
#    #u'”': u'」', # close double quote
#    u'“‘': u'『', # open double single quote
#    u'’”': u'』', # close single double quote
#  }))
#  def translate(self, text, to='zhs', fr='ja', async=False, emit=False):
#    """@reimp"""
#    #if fr not in ('ja', 'en', 'zhs', 'zht'):
#    #  return None, None, None
#    lang = to
#    #if lang not in ('zhs', 'zht', 'ja', 'en', 'fr', 'ko', 'ru', 'es'):
#    #  return None, None, None
#    tm = termman.manager()
#    text = tm.applySourceTerms(text, lang)
#    text = tm.prepareEscapeTerms(text, lang)
#    #text = self._prepareSentenceTransformation(text)
#    text = self.__youdao_repl_before(text)
#    sub = self._translateByParagraph(text, youdaofanyi.translate,
#        async=async, abortSignal=self.q.onlineAbortionRequested,
#        to=lang, fr=fr)
#    #sub = skthreads.runsync(partial(
#    #    youdaofanyi.translate, text, to=lang, fr=fr),
#    #    parent=self.q)
#    if sub:
#      sub = sub.replace(" . ", ".") # Fix escaped number
#      if lang == 'zht':
#        sub = zhs2zht(sub)
#      sub = self.__youdao_repl_after(sub)
#      sub = tm.applyEscapeTerms(sub, lang)
#      sub = tm.applyTargetTerms(sub, lang)
#      sub = textutil.beautify_subtitle(sub)
#    return sub, lang, self.key
#
#  def translateTest(self, text, to='en', fr='ja', async=False):
#    """
#    @param  text  unicode
#    @param* fr  unicode
#    @param* async  bool  ignored, always sync
#    @return  unicode sub, unicode lang, unicode provider
#    """
#    try: return self._translateTest(youdaofanyi.translate, text, to=to, fr=fr, async=async)
#    except Exception, e: dwarn(e); return ''
