# coding: utf8
# _trman.py
# 10/8/2012 jichi
#
# Terminology:
# - data: raw game text byte code
# - text: original game text
# - translation: machine translation
# - comment: user's subtitle or comment

#from sakurakit.skprof import SkProfiler

# TODO: Alignments are not cached to reduce memory usage

import os, re
import requests
from functools import partial
from itertools import ifilter, imap
from time import time
from PySide.QtCore import QMutex
from sakurakit import skstr, skthreads, sktypes
from sakurakit.skclass import memoizedproperty
from sakurakit.skdebug import dwarn
from opencc import opencc
from unitraits.uniconv import wide2thin_alnum
from mytr import my, mytr_
from unitraits import unichars, jpmacros
from convutil import wide2thin, zhs2zht, zht2zhs, zht2zhx
import config, defs, growl, mecabman, termman, textutil, trman, trcache, tahscript

_re_jitter = re.compile(jpmacros.applymacros(
    ur'{{boc}}({{kana}})[っッ]?(?=[、…]\1)'))

isspace = unichars.isspace
ispunct = textutil.ispunct

__NO_DELIM = '' # no deliminators
_NO_SET = frozenset()
_NO_RE = re.compile('')

__PARAGRAPH_DELIM = u"【】「」…♪" # machine translation of sentence deliminator
_PARAGRAPH_SET = frozenset(__PARAGRAPH_DELIM)
_PARAGRAPH_RE = re.compile(r"(%s|[%s])" %
    (defs.TERM_ESCAPE_EOS, ''.join(_PARAGRAPH_SET)))

#__SENTENCE_DELIM = u"\n【】「」。♪" #…！？# machine translation of sentence deliminator
#_SENTENCE_SET = frozenset(__SENTENCE_DELIM)
_SENTENCE_RE = re.compile(ur"([。？！…」\n])(?![。！？…）」\n]|$)")

_re_escape = re.compile(ur"^[0-9. 、。？！…]+$")
def is_escaped_text(text): # unicode -> bool
  return bool(_re_escape.match(text))

# All methods in this class are supposed to be thread-safe
# Though they are not orz
class TranslationCache:
  def __init__(self, maxSize=50, shrinkSize=20):
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
  asyncSupported = True # bool  whether threading is supported
  alignSupported = False # bool  whether support translation alignment
  onlineRequired = False # bool  whether translation requires Internet access

  def clearCache(self): pass

  def warmup(self, to='', fr=''): pass

  def translateTest(self, text, to='en', fr='ja', async=False):
    """
    @param  text  unicode
    @param  to  str
    @param* fr  str
    @param* async  bool
    @return  unicode sub
    """
    return ''

  def translate(self, text, to='en', fr='ja', async=False, emit=False, mark=None, scriptEnabled=False, align=None, **kwargs):
    """
    @param  text  unicode
    @param  to  str
    @param* fr  str
    @param* async  bool
    @param* emit  bool
    @param* align  list or None
    @param* mark  bool or None
    @param* scriptEnabled  bool
    @return  (unicode sub or None, str lang or None, str self.key or None)
    """
    return None, to, self.key

  @staticmethod
  def languageNeedsEscape(to, fr='ja'):
    """
    @param  to  str  language
    @param* fr  str  language
    @return  bool
    """
    return to[:2] in ('ko', 'zh') and fr == 'ja'

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

#class LougoTranslator(Translator):
#  key = 'lou' # override
#
#  def translate(self, text, to='en', fr='ja', emit=False, **kwargs):
#    """@reimp"""
#    if emit:
#      self.emitLanguages(fr='ja', to='en')
#    if fr != 'ja':
#      return None, None, None
#
#    #if scriptEnabled:
#    #  t = text
#    #  text = trscriptman.manager().normalizeText(text, fr=fr, to=to)
#    #  if emit and text != t:
#    #    self.emitNormalizedText(text)
#
#    tm = termman.manager()
#    t = text
#    text = tm.applySourceTerms(text, to='en', fr='ja', host=self.key)
#    if emit and text != t:
#      self.emitSourceText(text)
#    sub = mecabman.tolou(text)
#    if sub:
#      if emit:
#        self.emitJointTranslation(text)
#      sub = textutil.beautify_subtitle(sub)
#    return sub, 'ja', self.key
#
#  def translateTest(self, text, **kwargs):
#    """@reimp"""
#    return mecabman.tolou(text)


## Cascaded machine translator

class Retranslator(Translator):
  key = 'retr' # override

  def __init__(self, first=None, second=None, language=''):
    super(Retranslator, self).__init__()
    self.first = first # Translator
    self.second = second # Translator
    self.language = language # str

  @property
  def onlineRequired(self):
    """@reimp"""
    for it in (self.first, self.second):
      if it and it.onlineRequired:
        return True
    return False

  @property
  def asyncSupported(self):
    """@reimp"""
    for it in (self.first, self.second):
      if it and not it.asyncSupported:
        return False
    return True

  @property
  def alignSupported(self):
    """@reimp"""
    return bool(self.second) and self.second.alignSupported

  def translateTest(self, text, to='en', fr='ja', **kwargs):
    """@reimp"""
    if self.first:
      text = self.first.translateTest(text, to=self.language, fr=fr, **kwargs)
      if text and self.second:
        return self.second.translateTest(text, to=to, fr=self.language, **kwargs)
    return ''

  def translate(self, text, to='en', fr='ja', async=False, emit=False, mark=None, align=None, scriptEnabled1=False, scriptEnabled2=False, **kwargs):
    """@reimp"""
    if not self.first:
      return None, None, None
    if not self.second:
      return self.first.translateTest(text, to=to, fr=fr,
          mark=mark, align=align, emit=emit, scriptEnabled=scriptEnabled1, **kwargs)
    lang = self.language
    if lang == 'zht':
      lang = 'zhs' # force using Simplified Chinese as intermediate language
    text, lang, key = self.first.translate(text, to=lang, fr=fr,
        mark=False, align=None, emit=False, scriptEnabled=scriptEnabled1, **kwargs)
    if text:
      if fr == 'ja' and lang == 'zhs':
        text = opencc.ja2zhs(text)
      text2, lang2, key2 = self.second.translate(text, to=to, fr=lang,
          mark=mark, align=align, emit=emit, scriptEnabled=scriptEnabled2, **kwargs)
      if text2:
        text = text2
        lang = lang2
        key += ',' + key2
    return text, lang, key

## Text processing

class YueTranslator(Translator):

  def __init__(self, session=None, abortSignal=None):
    super(YueTranslator, self).__init__()
    self.abortSignal = abortSignal

    from baidu import baidufanyi
    baidufanyi.session = session or requests.Session()
    self.engine = baidufanyi

  def translate(self, text, fr, to='yue', async=False):
    """@reimp"""
    if text:
      ret = skthreads.runsync(partial(
        self.engne.translate, text, to=to, fr=fr),
        abortSignal=self.abortSignal,
      ) if async else self.engine.translate(text, to=to, fr=fr)
      if ret:
        if fr != 'zhs':
          ret = zhs2zht(ret)
        return ret
    return text

# Basic machine translators

class MachineTranslator(Translator):

  splitsSentences = False # bool
  persistentCaching = False # bool  whether use sqlite to cache the translation

  #_CACHE_LENGTH = 10 # length of the translation to cache

  #_DELIM_SET = _PARAGRAPH_SET # set of deliminators
  #_DELIM_RE = _PARAGRAPH_RE   # rx of deliminators

  def __init__(self, abortSignal=None, preprocess=None, postprocess=None):
    super(MachineTranslator, self).__init__()
    self.cache = TranslationCache()  # public overall translation cache
    self._cache = TranslationCache() # private translation cache for internal translation
    self.abortSignal = abortSignal # QtCore.Signal abort translation
    self.preprocess = preprocess # function unicode text, str fr -> text
    self.postprocess = postprocess # function unicode text, str to -> text

  def clearCache(self):
    """@reimp"""
    self.cache.clear()
    self._cache.clear()

  def __tr(self, text, tr, to, fr, async):
    """
    @param  text  unicode
    @param  async  bool
    @param  tr  function(unicode text, str to, str fr)
    @param  to  str
    @param  fr  str
    @return  unicode or None
    """
    #if len(text) > self._CACHE_LENGTH:
    #  return skthreads.runsync(partial(
    #    tr, text, **kwargs),
    #    abortSignal=self.abortSignal,
    #  ) if async else tr(text, **kwargs)
    ret = self._cache.get(text)
    if ret:
      return ret

    if self.persistentCaching:
      #with SkProfiler(): # takes about 0.03 to create, 0.02 to insert
      ret = trcache.get(key=self.key, fr=fr, to=to, text=text)
      if ret:
        self._cache.update(text, ret)
        return ret

    #with SkProfiler():
    ret = skthreads.runsync(partial(
      tr, text, to=to, fr=fr),
      abortSignal=self.abortSignal,
    ) if async and self.asyncSupported else tr(text, to=to, fr=fr)

    if ret:
      if isinstance(ret, str):
        ret = ret.decode('utf8', errors='ignore')
      if ret:
        self._cache.update(text, ret)
        if self.persistentCaching:
          #with SkProfiler(): # takes about 0.003
          trcache.add(key=self.key, fr=fr, to=to, text=text, translation=ret)
    return ret

  def _itertexts(self, text):
    """
    @param  text  unicode
    @yield  unicode
    """
    for line in _PARAGRAPH_RE.split(text):
      line = line.strip()
      if line:
        if not self.splitsSentences or len(line) == 1 or line == defs.TERM_ESCAPE_EOS:
          yield line
        else:
          for sentence in _SENTENCE_RE.sub(r"\1\n", line).split("\n"):
            if sentence:
              yield sentence

  def _splitTranslate(self, text, tr, to, fr, async):
    """
    @param  text  unicode
    @param  tr  function(text, to, fr)
    @param  async  bool
    @return  [unicode] not None
    """
    ret = []
    for text in self._itertexts(text):
      if len(text) == 1 and text in _PARAGRAPH_SET or is_escaped_text(text) or text == defs.TERM_ESCAPE_EOS:
        ret.append(text)
      else:
        text = self.__tr(text, tr, to, fr, async)
        if text is None:
          dwarn("translation failed or aborted using '%s'" % self.key)
          return []
        if text:
          ret.append(text)
    return ret

  def _translate(self, emit, text, tr, to, fr, async, align=None):
    """
    @param  emit  bool
    @param  text  unicode
    @param  tr  function(text, to, fr)
    @param  to  str
    @param  fr  str
    @param  async  bool
    @param* align  list or None
    @param  kwargs  arguments passed to tr
    @return  unicode
    """
    tr = self.__partialTranslate(tr, to, fr, align)
    l = self._splitTranslate(text, tr, to, fr, async)
    if emit:
      self.emitSplitTranslations(l)
    #delim = ' ' if self.splitsSentences else ''
    return ''.join(l) if l else ''

  def __partialTranslate(self, tr, to, fr, align):
    """
    @param  tr  function
    @param  fr  str
    @param  to  str
    @param  align  list or None
    @return  function
    """
    if align is not None and self.alignSupported:
      ret = partial(tr, align=align)
    else:
      ret = tr
    # 1/25/2015: temporarily disabled
    #if fr == 'ja':
    #  mt = termman.manager().getRuleBasedTranslator(to)
    #  if mt:
    #    ret = partial(mt.translate, tr=ret)
    return ret

  def _translateTest(self, fn, text, async=False, **kwargs):
    """
    @param  fn  translator
    @param  text  unicode
    @param* fr  unicode
    @param* async  bool  ignored, always sync
    @return  unicode sub, unicode lang, unicode provider
    """
    return skthreads.runsync(partial(fn, text, **kwargs)) if async and self.asyncSupported else fn(text, **kwargs)

  def _escapeText(self, text, to, fr, emit, scriptEnabled=False):
    """
    @param  text  unicode
    @param  to  str  language
    @param  fr  str  language
    @param  emit  bool
    @param* scriptEnabled  bool
    @return  unicode
    """
    #if scriptEnabled and to == 'en' and fr == 'ja':
    if scriptEnabled and fr == 'ja' and to not in ('ko', 'zhs', 'zht'):
      # 8/19/2014: Only test 0.007 second, with or without locks
      #with SkProfiler():
      t = text
      text = tahscript.manager().apply(text, self.key)
      if emit and text != t:
        self.emitNormalizedText(text)

    #if scriptEnabled:
    #  # 8/19/2014: Only test 0.007 second, with or without locks
    #  #with SkProfiler():
    #  t = text
    #  text = trscriptman.manager().normalizeText(text, fr=fr, to=to)
    #  if emit and text != t:
    #    self.emitNormalizedText(text)
    if self.preprocess:
      text = self.preprocess(text, fr)

    tm = termman.manager()
    t = text
    #with SkProfiler(): # 9/26/2014: 0.0005 seconds, Python: 0.04 seconds
    text = tm.applySourceTerms(text, to=to, fr=fr, host=self.key)
    if emit and text != t:
      self.emitSourceText(text)

    t = text
    #with SkProfiler(): # 9/26/2014: C++ 0.015 seconds, Python: 0.05 seconds
    #if self._needsJitter(to, fr):
    if fr == 'ja':
      text = self._escapeJitter(text, to, fr)
    text = tm.prepareEscapeTerms(text, to=to, fr=fr, host=self.key)
    if emit and text != t:
      self.emitEscapedText(text)

    if emit:
      l = list(self._itertexts(text))
      self.emitSplitTexts(l)
    return text

  def _unescapeTranslation(self, text, to, fr, mark, emit):
    """
    @param  text  unicode
    @param  to  str  language
    @param  fr  str  language
    @param  mark  bool or None
    @param  emit  bool
    @return  unicode
    """
    tm = termman.manager()
    if emit:
      self.emitJointTranslation(text)

    if to == 'zht':
      text = zht2zhx(text)

    #text = self.__google_repl_after(text)
    t = text
    #with SkProfiler(): # 9/26/2014: 0.08 seconds, Python: 0.06 seconds
    text = tm.applyEscapeTerms(text, to=to, fr=fr, mark=mark, host=self.key)
    if emit and text != t:
      self.emitEscapedTranslation(text)
    t = text
    #with SkProfiler(): # 9/26/2014: 0.0005 seconds, Python: 0.04 seconds
    text = tm.applyTargetTerms(text, to=to, fr=fr, mark=mark, host=self.key)
    text = text.replace(defs.TERM_ESCAPE_EOS, '')
    #if self._needsJitter(to, fr):
    if fr == 'ja':
      text = self._unescapeJitter(text, to, fr)
    if emit and text != t:
      self.emitTargetTranslation(text)
    #text = text.replace("( ", '(')
    #text = text.replace(u"\n】", u"】\n")
    if text:
      if self.postprocess:
        text = self.postprocess(text, to)
      if config.is_latin_language(to):
        text = textutil.capitalize_html_sentence(text)
      if not (to.startswith('zh') and to in ('ja', 'ar')):
        text = textutil.remove_html_punct_space(text)
      text = textutil.beautify_subtitle(text)
    return text.strip() # escape could produce trailing " "

  # Jitter for CJK
  #def _needsJitter(self, to, fr):
  #  """
  #  @param  to  str  language
  #  @param  fr  str  language
  #  @return  bool
  #  """
  #  return fr == 'ja' and (to.startswith('zh') or to == 'ko')

  def _escapeJitter(self, text, to, fr):
    """
    @param  text  unicode
    @param  to  str  unused
    @param  fr  str  unused
    @return  unicode
    """
    esc = defs.JITTER_ESCAPE_KANJI if config.is_kanji_language(to) else defs.JITTER_ESCAPE_LATIN
    return _re_jitter.sub(esc, text)

  def _unescapeJitter(self, text, to, fr):
    """
    @param  text  unicode
    @param  to  str  unused
    @param  fr  str  unused
    @return  unicode
    """
    esc = defs.JITTER_ESCAPE_KANJI if config.is_kanji_language(to) else defs.JITTER_ESCAPE_LATIN
    i = text.rfind(esc)
    while i >= 0:
      jitter = '' # jittered char
      j = i + len(esc)
      while j < len(text):
        ch = text[j]
        if ch == '<':
          while j < len(text) and text[j] != '>':
            j += 1
        elif not isspace(ch) and not ispunct(ch):
          jitter = ch
          break
        j += 1
      text = text[:i] + jitter + text[i+len(esc):]
      i = text.rfind(esc)
    return text

class OfflineMachineTranslator(MachineTranslator):
  onlineRequired = False # overrie
  persistentCaching = False # bool  disable sqlite
  def __init__(self, *args, **kwargs):
    super(OfflineMachineTranslator, self).__init__(*args, **kwargs)

class OnlineMachineTranslator(MachineTranslator):
  onlineRequired = True # override
  #persistentCaching = True # bool  enable sqlite
  persistentCaching = False # bool  temporarily disabled, or it will break the syntax translator
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
  def warmup(self, to='', fr=''):
    """@reimp"""
    self.engine.warmup()

  def translateTest(self, text, to='en', fr='ja', async=False):
    """@reimp"""
    try: return self._translateTest(self.engine.translate, text, async=async)
    except Exception, e:
      dwarn(e)
      growl.error(my.tr("Cannot load {0} for machine translation. Please check Preferences/Location").format(mytr_("ATLAS")),
          async=async)
      return ''

  def _translateApi(self, text, fr='', to=''): # unicode -> unicode
    return self.engine.translate(text)

  def translate(self, text, to='en', fr='ja', async=False, emit=False, scriptEnabled=False, mark=None, **kwargs):
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
    repl = self._escapeText(text, to=to, fr=fr, emit=emit, scriptEnabled=scriptEnabled)
    if repl:
      try:
        repl = self._translate(emit, repl,
            self._translateApi,
            to, fr, async)
        if repl:
          # ATLAS always try to append period at the end
          repl = wide2thin(repl) #.replace(u". 。", ". ").replace(u"。", ". ").replace(u" 」", u"」").rstrip()
          repl = self._unescapeTranslation(repl, to=to, fr=fr, mark=mark, emit=emit)
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
    ok = ret.load() # no language is given here
    #import atexit
    #atexit.register(ret.destroy)
    if ok:
      growl.msg(my.tr("LEC translator is loaded"))
    else:
      growl.error(my.tr("Cannot load {0} for machine translation. Please check Preferences/Location").format(mytr_("LEC")))
    return ret

  # This would cause issue?
  def warmup(self, to='', fr=''):
    """@reimp"""
    if fr + to in ('jaen', 'jaru', 'enru'):
      self.engine.warmup(to=to, fr=fr)
    else:
      self.engine.warmup()

  def translateTest(self, text, to='en', fr='ja', async=False):
    """@reimp"""
    to, fr = self._checkLanguages(to, fr)
    try: return self._translateTest(self.engine.translate, text, to=to, fr=fr, async=async)
    except Exception, e:
      dwarn(e)
      growl.error(my.tr("Cannot load {0} for machine translation. Please check Preferences/Location").format(mytr_("LEC")),
          async=async)
      return ''

  #def _translateApi(self, text, fr='', to=''): # unicode -> unicode
  #  return self.engine.translate(text)

  #@staticmethod
  def _checkLanguages(self, to, fr):
    """
    @param  to  str
    @param  fr  str
    @return  (str to, str fr)
    """
    if to not in ('en', 'ru'):
      to = 'en'
    if fr not in ('en', 'ja'):
      fr = 'ja'
    return to, fr

  def translate(self, text, to='en', fr='ja', async=False, emit=False, mark=None, scriptEnabled=False, **kwargs):
    """@reimp"""
    to, fr = self._checkLanguages(to, fr)
    if emit:
      self.emitLanguages(fr=fr, to=to)
    #if fr != 'ja':
    #  return None, None, None
    if not emit:
      repl = self.cache.get(text)
      if repl:
        return repl, to, self.key
    repl = self._escapeText(text, to=to, fr=fr, emit=emit, scriptEnabled=scriptEnabled)
    if repl:
      try:
        repl = self._translate(emit, repl,
            self.engine.translate,
            to, fr, async)
        if repl:
          repl = wide2thin(repl) #.replace(u"。", ". ").replace(u" 」", u"」").rstrip()
          repl = self._unescapeTranslation(repl, to=to, fr=fr, mark=mark, emit=emit)
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

  def warmup(self, to='', fr=''):
    """@reimp"""
    self.engine.warmup()

  def translateTest(self, text, to='en', fr='ja', async=False):
    """@reimp"""
    try: return self._translateTest(self.engine.translate, text, async=async)
    except Exception, e:
      dwarn(e)
      growl.error(my.tr("Cannot load {0} for machine translation. Please check Preferences/Location").format(mytr_("ezTrans XP")),
          async=async)
      return ''

  def _translateApi(self, text, fr='', to=''): # unicode -> unicode
    return self.engine.translate(text)

  __ez_repl_before = staticmethod(skstr.multireplacer({
    u'『': u'『"『', # a double quote in the middle
    u'』': u'』"』',

  }))
  __ez_repl_after = staticmethod(skstr.multireplacer({
    u'「"「': u'『',
    u'」"」': u'』',
  }))
  # Example: GPS97351.678 => GPS97351. 678
  __re_term_fix = re.compile(r'(?<=\d\.) (?=\d{2})')
  def translate(self, text, to='ko', fr='ja', async=False, emit=False, mark=None, **kwargs):
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
    repl = self._escapeText(text, to=to, fr=fr, emit=emit)
    if repl:
      try:
        repl = self.__ez_repl_before(repl)
        repl = self._translate(emit, repl,
            self._translateApi,
            to, fr, async)
        if repl:
          repl = self.__ez_repl_after(repl)
          repl = self.__re_term_fix.sub('', repl)
          repl = self._unescapeTranslation(repl, to=to, fr=fr, mark=mark, emit=emit)
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

class HanVietTranslator(OfflineMachineTranslator):
  key = 'hanviet' # override
  alignSupported = True # override

  def __init__(self, **kwargs):
    super(HanVietTranslator, self).__init__(**kwargs)

    from hanviet import hanviet
    self.engine = hanviet

  def _translateApi(self, text, fr='', to='', mark=False, align=None): # unicode, bol -> unicode
    # Wide to thin and padding space as well
    text = text.replace(u'，', ", ")
    text = text.replace(u'。', ". ")
    text = text.replace(u'、', ", ")
    text = text.replace(u'？', "? ")
    text = text.replace(u'！', "! ")
    text = text.replace(u'（', " (")
    text = text.replace(u'）', ") ")
    text = text.replace(u'”', ' "')
    text = text.replace(u'”', '" ')
    text = text.replace(u'‘', " '")
    text = text.replace(u'’', "' ")
    text = self.engine.translate(text, mark=bool(mark), align=align)
    #text = wide2thin(text) # not needed
    return text

  def translate(self, text, to='vi', fr='zhs', emit=False, mark=None, align=None, **kwargs):
    """@reimp"""
    async = False # disable async
    to = 'vi'
    if fr not in ('zhs', 'zht', 'ja', 'vi'):
      if emit:
        self.emitLanguages(fr=fr, to=to)
      return None, None, None
    #if fr != 'zhs':
    #  text = zht2zhs(text)
    #  fr = 'zhs'
    fr = 'zhs'
    if emit:
      self.emitLanguages(fr=fr, to=to)
    if not emit:
      repl = self.cache.get(text)
      if repl:
        return repl, to, self.key
    text = opencc.ja2zhs(text)
    #with SkProfiler():
    repl = self._escapeText(text, to=to, fr=fr, emit=emit)
    if repl:
      repl = self._translate(emit, repl,
          partial(self._translateApi, mark=mark),
          to, fr, async, align=align)
      if repl:
        repl = self._unescapeTranslation(repl, to=to, fr=fr, mark=mark, emit=emit) # 0.1 seconds
        self.cache.update(text, repl)
        return repl, to, self.key
    return None, None, None

  def translateTest(self, text, **kwargs):
    """@reimp"""
    text = opencc.ja2zhs(text)
    return self._translateApi(text, mark=False)

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

  def warmup(self, to='', fr=''):
    """@reimp"""
    self.engine.warmup()

  def translateTest(self, text, to='en', fr='ja', async=False):
    """@reimp"""
    simplified = to == 'zhs'
    try: return self._translateTest(self.engine.translate, text, async=async, simplified=simplified)
    except Exception, e:
      dwarn(e)
      growl.error(my.tr("Cannot load {0} for machine translation. Please check Preferences/Location").format(mytr_("JBeijing")),
          async=async)
      return ''

  def _translateApi(self, text, fr='', to='', simplified=False): # unicode -> unicode
    return self.engine.translate(text, simplified=simplified)

  def translate(self, text, to='zhs', fr='ja', async=False, emit=False, mark=None, **kwargs):
    """@reimp"""
    # Profiler: 1e-5 seconds
    #with SkProfiler():
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
    repl = self._escapeText(text, to=to, fr=fr, emit=emit) # 0.1 seconds
    if repl:
      repl = repl.replace('\n', ' ') # JBeijing cannot handle multiple lines
      try:
        repl = self._translate(emit, repl,
            partial(self._translateApi, simplified=simplified),
            to, fr, async) # 0.1 seconds
        if repl:
          #with SkProfiler():
          #repl = wide2thin_digit(repl) # convert wide digits to thin digits
          repl = wide2thin_alnum(repl) # convert all wide characters to thin
          repl = self._unescapeTranslation(repl, to=to, fr=fr, mark=mark, emit=emit) # 0.1 seconds
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

class FastAITTranslator(OfflineMachineTranslator):
  key = 'fastait' # override

  def __init__(self, **kwargs):
    super(FastAITTranslator, self).__init__(**kwargs)
    self._warned = False # bool
    self._mutex = QMutex()

  @memoizedproperty
  def jazhsEngine(self):
    from kingsoft import fastait
    ret = fastait.create_engine(fr='ja', to='zhs')
    ok = ret.load()
    #import atexit
    #atexit.register(ret.destroy)
    if ok:
      growl.msg(my.tr("FastAIT Japanese-Chinese translator is loaded"))
    else:
      growl.error(my.tr("Cannot load {0} for machine translation. Please check Preferences/Location").format(mytr_("FastAIT")))
    return ret

  @memoizedproperty
  def jazhtEngine(self):
    from kingsoft import fastait
    ret = fastait.create_engine(fr='ja', to='zht')
    ok = ret.load()
    #import atexit
    #atexit.register(ret.destroy)
    if ok:
      growl.msg(my.tr("FastAIT Japanese-Chinese translator is loaded"))
    else:
      growl.error(my.tr("Cannot load {0} for machine translation. Please check Preferences/Location").format(mytr_("FastAIT")))
    return ret

  @memoizedproperty
  def enzhsEngine(self):
    from kingsoft import fastait
    ret = fastait.create_engine(fr='en', to='zhs')
    ok = ret.load()
    #import atexit
    #atexit.register(ret.destroy)
    if ok:
      growl.msg(my.tr("FastAIT English-Chinese translator is loaded"))
    else:
      growl.error(my.tr("Cannot load {0} for machine translation. Please check Preferences/Location").format(mytr_("FastAIT")))
    return ret

  @memoizedproperty
  def enzhtEngine(self):
    from kingsoft import fastait
    ret = fastait.create_engine(fr='en', to='zht')
    ok = ret.load()
    #import atexit
    #atexit.register(ret.destroy)
    if ok:
      growl.msg(my.tr("FastAIT English-Chinese translator is loaded"))
    else:
      growl.error(my.tr("Cannot load {0} for machine translation. Please check Preferences/Location").format(mytr_("FastAIT")))
    return ret

  @memoizedproperty
  def zhsenEngine(self):
    from kingsoft import fastait
    ret = fastait.create_engine(fr='zhs', to='en')
    ok = ret.load()
    #import atexit
    #atexit.register(ret.destroy)
    if ok:
      growl.msg(my.tr("FastAIT Chinese-English translator is loaded"))
    else:
      growl.error(my.tr("Cannot load {0} for machine translation. Please check Preferences/Location").format(mytr_("FastAIT")))
    return ret

  @memoizedproperty
  def zhtenEngine(self):
    from kingsoft import fastait
    ret = fastait.create_engine(fr='zht', to='en')
    ok = ret.load()
    #import atexit
    #atexit.register(ret.destroy)
    if ok:
      growl.msg(my.tr("FastAIT Chinese-English translator is loaded"))
    else:
      growl.error(my.tr("Cannot load {0} for machine translation. Please check Preferences/Location").format(mytr_("FastAIT")))
    return ret

  def getEngine(self, fr, to):
    langs = fr + to
    if langs == 'jazhs':
      return self.jazhsEngine
    elif langs == 'jazht':
      return self.jazhtEngine
    elif langs == 'enzhs':
      return self.enzhsEngine
    elif langs == 'enzht':
      return self.enzhtEngine
    elif langs == 'zhsen':
      return self.zhsenEngine
    elif langs == 'zhten':
      return self.zhtenEngine

  #@staticmethod
  def _checkLanguages(self, to, fr):
    """
    @param  to  str
    @param  fr  str
    @return  (str to, str fr)
    """
    if to not in ('en', 'ja') and not to.startswith('zh'):
      to = 'en'
      if to == fr:
        to = 'zht'
    return to, fr

  def translateTest(self, text, to='en', fr='ja', async=False):
    """@reimp"""
    to, fr = self._checkLanguages(to, fr)
    try:
      engine = self.getEngine(fr=fr, to=to)
      if engine:
        return self._translateTest(engine.translate, text, to=to, fr=fr, async=async)
    except Exception, e:
      dwarn(e)
      growl.error(my.tr("Cannot load {0} for machine translation. Please check Preferences/Location").format(mytr_("Dr.eye")),
          async=async)
    return ''

  # Ignored
  #def warmup(self, to='', fr=''):
  #  """@reimp"""
  #  self.ecEngine.warmup()
  #  self.jcEngine.warmup()

  # Prevent Fastait from turning 『』 to 「」
  __ja_repl_before = staticmethod(skstr.multireplacer({
    u'『': u'“‘', # open double single quote
    u'』': u'’”', # close single double quote

  }))
  __ja_repl_after = staticmethod(skstr.multireplacer({
    u'“‘': u'『', # open double single quote
    u'’”': u'』', # close single double quote
  }))
  def translate(self, text, to='zhs', fr='ja', async=False, emit=False, mark=None, **kwargs):
    """@reimp"""
    #async = True # force async since FastAIT is randomly slow. but async would cause synchronization error
    async = True
    to, fr = self._checkLanguages(to, fr)
    engine = self.getEngine(to=to, fr=fr)
    if engine:
      if emit:
        self.emitLanguages(fr=fr, to=to)
      else:
        repl = self.cache.get(text)
        if repl:
          return repl, to, self.key
      repl = self._escapeText(text, to=to, fr=fr, emit=emit)
      if repl:
        try:
          if fr == 'ja':
            repl = self.__ja_repl_before(repl)
          repl = self._translate(emit, repl,
              partial(self._synchronizedTranslate, engine.translate),
              to, fr, async)
          if repl:
            if fr == 'ja':
              repl = self.__ja_repl_after(repl)
            repl = self._unescapeTranslation(repl, to=to, fr=fr, mark=mark, emit=emit)
            self.cache.update(text, repl)
            return repl, to, self.key
        #except RuntimeError, e:
        except Exception, e:
          if not self._warned:
            self._warned = True
            dwarn(e) # This might crash colorama TT
            if not async:
              growl.error(my.tr("Cannot load {0} for machine translation. Please check Preferences/Location").format(mytr_("FastAIT")),
                  async=async)
    return None, None, None

  def _synchronizedTranslate(self, tr, *args, **kwargs):
    self._mutex.lock()
    ret = tr(*args, **kwargs)
    self._mutex.unlock()
    return ret

class DreyeTranslator(OfflineMachineTranslator):
  key = 'dreye' # override

  def __init__(self, **kwargs):
    super(DreyeTranslator, self).__init__(**kwargs)
    self._warned = False # bool

  @memoizedproperty
  def jcEngine(self):
    from dreye import dreyemt
    ret = dreyemt.create_engine('ja')
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
    from dreye import dreyemt
    ret = dreyemt.create_engine('en')
    ok = ret.load()
    #import atexit
    #atexit.register(ret.destroy)
    if ok:
      growl.msg(my.tr("Dr.eye English-Chinese translator is loaded"))
    else:
      growl.error(my.tr("Cannot load {0} for machine translation. Please check Preferences/Location").format(mytr_("Dr.eye")))
    return ret

  def translateTest(self, text, to='en', fr='ja', async=False):
    """@reimp"""
    try:
      engine = self.jcEngine if fr == 'ja' else self.ecEngine
      return self._translateTest(engine.translate, text, to=to, fr=fr, async=async)
    except Exception, e:
      dwarn(e)
      growl.error(my.tr("Cannot load {0} for machine translation. Please check Preferences/Location").format(mytr_("Dr.eye")),
          async=async)
      return ''

  # Ignored
  #def warmup(self, to='', fr=''):
  #  """@reimp"""
  #  self.ecEngine.warmup()
  #  self.jcEngine.warmup()

  #__dreye_repl_after = staticmethod(skstr.multireplacer({
  #  '[': u'【',
  #  ' ]': u'】',
  #}))
  def translate(self, text, to='zhs', fr='ja', async=False, emit=False, mark=None, **kwargs):
    """@reimp"""
    if fr == 'zht':
      text = zht2zhs(text)
    engine = self.jcEngine if fr == 'ja' else self.ecEngine
    if emit:
      self.emitLanguages(fr=fr, to=to)
    else:
      repl = self.cache.get(text)
      if repl:
        return repl, to, self.key
    repl = self._escapeText(text, to=to, fr=fr, emit=emit)
    if repl:
      try:
        repl = self._translate(emit, repl,
            engine.translate,
            to, fr, async)
        if repl:
          if to != 'zhs':
            repl = zhs2zht(repl)
          #sub = self._applySentenceTransformation(sub)
          #sub = self.__dreye_repl_after(sub)
          #sub = sub.replace(']', u'】')
          repl = self._unescapeTranslation(repl, to=to, fr=fr, mark=mark, emit=emit)
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
  asyncSupported = False # override  disable async
  alignSupported = True # override

  NOT_SUPPORTED_LANGUAGES = frozenset(('ms', 'ar', 'ru', 'nl', 'pl'))

  def _checkLanguages(self, to, fr):
    """
    @param  to  str
    @param  fr  str
    @return  (str to, str fr)
    """
    if to in self.NOT_SUPPORTED_LANGUAGES:
      to = 'en'
    if fr in self.NOT_SUPPORTED_LANGUAGES:
      fr = 'en'
    return to, fr

  def __init__(self, session=None, **kwargs):
    super(InfoseekTranslator, self).__init__(**kwargs)

    from transer import infoseek
    infoseek.session = session or requests.Session()
    self.engine = infoseek

  #__infoseek_repl_after = staticmethod(skstr.multireplacer({
  #  '[': u'【',
  #  ']\n': u'】',
  #}))
  __fix_escape = re.compile(r'(?<=[0-9]) .(?=[0-9])') # replace ' .' between digits with '.'
  def translate(self, text, to='en', fr='ja', async=False, emit=False, mark=None, align=None, scriptEnabled=False, **kwargs):
    """@reimp"""
    to, fr = self._checkLanguages(to, fr)
    if emit:
      self.emitLanguages(fr=fr, to=to)
    else:
      repl = self.cache.get(text)
      if repl:
        return repl, to, self.key
    repl = self._escapeText(text, to=to, fr=fr, emit=emit, scriptEnabled=scriptEnabled)
    if repl:
      repl = self._translate(emit, repl,
          self.engine.translate,
          to, fr, async, align=align)
      if repl:
        repl = self.__fix_escape.sub('.', repl)
        repl = self._unescapeTranslation(repl, to=to, fr=fr, mark=mark, emit=emit)
        self.cache.update(text, repl)
    return repl, to, self.key

  def translateTest(self, text, to='en', fr='ja', async=False):
    """@reimp"""
    to, fr = self._checkLanguages(to, fr)
    try: return self._translateTest(self.engine.translate,
            text, to=to, fr=fr, async=async)
    except Exception, e: dwarn(e); return ''

class ExciteTranslator(OnlineMachineTranslator):
  key = 'excite' # override
  asyncSupported = False # override  disable async

  def __init__(self, session=None, **kwargs):
    super(ExciteTranslator, self).__init__(**kwargs)
    from excite import worldtrans
    #worldtrans.session = session or requests.Session() # Session is disabled otherwise it will get blocked
    self.engine = worldtrans

  NOT_SUPPORTED_LANGUAGES = frozenset(('ms', 'id', 'th', 'vi', 'ar', 'nl', 'pl'))

  def _checkLanguages(self, to, fr):
    """
    @param  to  str
    @param  fr  str
    @return  (str to, str fr)
    """
    if to in self.NOT_SUPPORTED_LANGUAGES:
      to = 'en'
    if fr in self.NOT_SUPPORTED_LANGUAGES:
      fr = 'en'
    return to, fr

  #__excite_repl_after = staticmethod(skstr.multireplacer({
  #  '[': u'【',
  #  ']\n': u'】',
  #}))
  def translate(self, text, to='en', fr='ja', async=False, emit=False, mark=None, scriptEnabled=False, **kwargs):
    """@reimp"""
    to, fr = self._checkLanguages(to, fr)
    if emit:
      self.emitLanguages(fr=fr, to=to)
    else:
      repl = self.cache.get(text)
      if repl:
        return repl, to, self.key
    repl = self._escapeText(text, to=to, fr=fr, emit=emit, scriptEnabled=scriptEnabled)
    if repl:
      repl = self._translate(emit, repl,
          self.engine.translate,
          to, fr, async)
      if repl:
        repl = self._unescapeTranslation(repl, to=to, fr=fr, mark=mark, emit=emit)
        self.cache.update(text, repl)
    return repl, to, self.key

  def translateTest(self, text, to='en', fr='ja', async=False):
    """@reimp"""
    to, fr = self._checkLanguages(to, fr)
    try: return self._translateTest(self.engine.translate,
            text, to=to, fr=fr, async=async)
    except Exception, e: dwarn(e); return ''

class LecOnlineTranslator(OnlineMachineTranslator):
  key = 'lecol' # override
  asyncSupported = False # override  disable async

  def __init__(self, session=None, **kwargs):
    super(LecOnlineTranslator, self).__init__(**kwargs)

    from lec import leconline
    leconline.session = session or requests.Session()
    self.engine = leconline

  NOT_SUPPORTED_LANGUAGES = frozenset(('ms', 'th', 'vi'))

  def _checkLanguages(self, to, fr):
    """
    @param  to  str
    @param  fr  str
    @return  (str to, str fr)
    """
    if to in self.NOT_SUPPORTED_LANGUAGES:
      to = 'en'
    if fr in self.NOT_SUPPORTED_LANGUAGES:
      fr = 'en'
    return to, fr

  def translate(self, text, to='en', fr='ja', async=False, emit=False, mark=None, scriptEnabled=False, **kwargs):
    """@reimp"""
    to, fr = self._checkLanguages(to, fr)
    if emit:
      self.emitLanguages(fr=fr, to=to)
    else:
      repl = self.cache.get(text)
      if repl:
        return repl, to, self.key
    repl = self._escapeText(text, to=to, fr=fr, emit=emit, scriptEnabled=scriptEnabled)
    if repl:
      repl = self._translate(emit, repl,
          self.engine.translate,
          to, fr, async)
      if repl:
        repl = self._unescapeTranslation(repl, to=to, fr=fr, mark=mark, emit=emit)
        self.cache.update(text, repl)
    return repl, to, self.key

  def translateTest(self, text, to='en', fr='ja', async=False):
    """@reimp"""
    to, fr = self._checkLanguages(to, fr)
    try: return self._translateTest(self.engine.translate,
            text, to=to, fr=fr, async=async)
    except Exception, e: dwarn(e); return ''

class TransruTranslator(OnlineMachineTranslator):
  key = 'transru' # override
  asyncSupported = False # override  disable async

  SUPPORTED_LANGUAGES = frozenset(('ja', 'en', 'ru', 'it', 'fr', 'de', 'pt', 'es'))

  def __init__(self, session=None, **kwargs):
    super(TransruTranslator, self).__init__(**kwargs)

    from promt import transru
    transru.session = session or requests.Session()
    self.engine = transru

  def _checkLanguages(self, to, fr):
    """
    @param  to  str
    @param  fr  str
    @return  (str to, str fr)
    """
    if to not in self.SUPPORTED_LANGUAGES:
      to = 'en'
    if fr not in self.SUPPORTED_LANGUAGES:
      fr = 'en'
    return to, fr

  def translate(self, text, to='en', fr='ja', async=False, emit=False, mark=None, scriptEnabled=False, **kwargs):
    """@reimp"""
    to, fr = self._checkLanguages(to, fr)
    if emit:
      self.emitLanguages(fr=fr, to=to)
    else:
      repl = self.cache.get(text)
      if repl:
        return repl, to, self.key
    repl = self._escapeText(text, to=to, fr=fr, emit=emit, scriptEnabled=scriptEnabled)
    if repl:
      repl = self._translate(emit, repl,
          self.engine.translate,
          to, fr, async)
      if repl:
        repl = self._unescapeTranslation(repl, to=to, fr=fr, mark=mark, emit=emit)
        self.cache.update(text, repl)
    return repl, to, self.key

  def translateTest(self, text, to='en', fr='ja', async=False):
    """@reimp"""
    to, fr = self._checkLanguages(to, fr)
    try: return self._translateTest(self.engine.translate,
            text, to=to, fr=fr, async=async)
    except Exception, e: dwarn(e); return ''

class VTranslator(OnlineMachineTranslator):
  key = 'vtrans' # override
  #asyncSupported = True # override  enable async
  asyncSupported = False # override  disable async
  alignSupported = True # override

  def __init__(self, session=None, **kwargs):
    super(VTranslator, self).__init__(**kwargs)

    from vtrans import vtrans
    vtrans.session = session or requests.Session()
    self.engine = vtrans

  def translate(self, text, to='zhs', fr='ja', async=False, emit=False, mark=None, **kwargs):
    """@reimp"""
    if fr != 'ja':
      return None, None, None
    if not to.startswith('zh'):
      to = 'zhs'
    if emit:
      self.emitLanguages(fr=fr, to=to)
    else:
      repl = self.cache.get(text)
      if repl:
        return repl, to, self.key
    repl = self._escapeText(text, to=to, fr=fr, emit=emit)
    if repl:
      repl = self._translate(emit, repl,
          self.engine.translate,
          to, fr, async)
      if repl:
        repl = self._unescapeTranslation(repl, to=to, fr=fr, mark=mark, emit=emit)
        if to == 'zht':
          repl = zhs2zht(repl)
        self.cache.update(text, repl)
    return repl, to, self.key

  def translateTest(self, text, to='en', fr='ja', async=False):
    """@reimp"""
    #async = True # force enable async
    try: return self._translateTest(self.engine.translate,
            text, to=to, fr=fr, async=async) #.decode('utf8', errors='ignore')
    except Exception, e: dwarn(e); return ''

class GoogleTranslator(OnlineMachineTranslator):
  key = 'google' # override
  #asyncSupported = True # override  enable async
  asyncSupported = False # override  disable async
  alignSupported = True # override

  def __init__(self, session=None, **kwargs):
    super(GoogleTranslator, self).__init__(**kwargs)

    import googleman
    googleman.setsession(session or requests.Session())

    # Session is not used, or it could get blocked by Google
    #googletrans.session = requests.Session()
    self.engine = googleman.manager()

  #__google_repl_after = staticmethod(skstr.multireplacer({
  #  '...': u'…',
  #}))
  # Fix numbers such as 929,005.678。
  __re_term_fix = re.compile(r'(?<=\d),(?=\d{2})')
  def translate(self, text, to='en', fr='ja', async=False, emit=False, mark=None, align=None, scriptEnabled=False, **kwargs):
    """@reimp"""
    #async = True # force enable async
    #to = 'vi'
    if emit:
      self.emitLanguages(fr=fr, to=to)
    else:
      repl = self.cache.get(text)
      if repl:
        return repl, to, self.key
    repl = self._escapeText(text, to=to, fr=fr, emit=emit, scriptEnabled=scriptEnabled)
    if repl:
      repl = self._translate(emit, repl,
          self.engine.translate,
          to, fr, async, align=align)
      if repl:
        #if self.languageNeedsEscape(to, fr):
        repl = self.__re_term_fix.sub('', repl)
        repl = self._unescapeTranslation(repl, to=to, fr=fr, mark=mark, emit=emit)
        if to.startswith('zh'):
          repl = repl.replace("...", u'…')
        self.cache.update(text, repl)
    return repl, to, self.key

  def translateTest(self, text, to='en', fr='ja', async=False):
    """@reimp"""
    #async = True # force enable async
    try: return self._translateTest(self.engine.translate,
            text, to=to, fr=fr, async=async) #.decode('utf8', errors='ignore')
    except Exception, e: dwarn(e); return ''

class BingTranslator(OnlineMachineTranslator):
  key = 'bing' # override
  asyncSupported = False # override  disable async
  alignSupported = True # override

  def __init__(self, session=None, **kwargs):
    super(BingTranslator, self).__init__(**kwargs)

    from microsoft import bingtrans
    bingtrans.session = session or requests.Session()

    import bingman
    self.engine = bingman.manager()

  #__bing_repl_after = staticmethod(skstr.multireplacer({
  #  '[': u'【',
  #  ']\n': u'】',
  #}))
  __fix_escape = re.compile(r'(?<=[0-9]),(?=[0-9])') # replace ',' between digits with '.'
  def translate(self, text, to='en', fr='ja', async=False, emit=False, mark=None, align=None, scriptEnabled=False, **kwargs):
    """@reimp"""
    #if fr != 'ja':
    #  return None, None, None
    #to = 'en'
    if emit:
      self.emitLanguages(fr=fr, to=to)
    else:
      repl = self.cache.get(text)
      if repl:
        return repl, to, self.key
    repl = self._escapeText(text, to=to, fr=fr, emit=emit, scriptEnabled=scriptEnabled)
    if repl:
      repl = self._translate(emit, repl,
          self.engine.translate,
          to, fr, async, align=align)
      if repl:
        repl = self.__fix_escape.sub('.', repl)
        repl = self._unescapeTranslation(repl, to=to, fr=fr, mark=mark, emit=emit)
        self.cache.update(text, repl)
    return repl, to, self.key

  def translateTest(self, text, to='en', fr='ja', async=False):
    """@reimp"""
    try: return self._translateTest(self.engine.translate, text, to=to, fr=fr, async=async)
    except Exception, e: dwarn(e); return ''

class NaverTranslator(OnlineMachineTranslator):
  key = 'naver' # override
  asyncSupported = False # override  disable async
  alignSupported = True # override  enable translation alignment

  def __init__(self, session=None, **kwargs):
    super(NaverTranslator, self).__init__(**kwargs)

    from naver import navertrans
    navertrans.session = session or requests.Session()

    self.engine = navertrans

  #@staticmethod
  def _checkLanguages(self, to, fr):
    """
    @param  to  str
    @param  fr  str
    @return  (str to, str fr)
    """
    if fr == 'ja':
      return 'ko', 'ja'
    elif fr == 'en':
      if to not in ('ko', 'es', 'pt', 'id', 'th'):
        return 'ko', 'en'
    elif fr == 'ko':
      if to not in ('en', 'ko', 'zhs', 'zht'):
        return 'en', 'ko'
    elif fr.startswith('zh'):
      return 'ko', fr
    elif fr in ('es', 'pt', 'id', 'th'):
      return 'en', fr
    else:
      return 'ko', 'en'
    return to, fr

  def translate(self, text, to='ko', fr='ja', async=False, emit=False, mark=None, align=None, **kwargs):
    """@reimp"""
    to, fr = self._checkLanguages(to, fr)
    if emit:
      self.emitLanguages(fr=fr, to=to)
    else:
      repl = self.cache.get(text)
      if repl:
        return repl, to, self.key
    if fr == 'zht': # traditional Chinese translation is not good
      text = zht2zhs(text)
      fr = 'zhs'
    repl = self._escapeText(text, to=to, fr=fr, emit=emit)
    if repl:
      alignCount = len(align) if align else 0
      repl = self._translate(emit, repl,
          self.engine.translate,
          to, fr, async, align=align)
      if repl:
        # Sometimes naver translation result contains <>
        # Example: るみちゃん、めでたい結婚を機にさ、名前変えたら
        # => 루미,<결혼을 계기로, 이름 바꾸면
        repl = repl.replace('<', '').replace('>', '')
        if to == 'zht':
          repl = zhs2zht(repl)
          if align:
            for i,(k,v) in enumerate(align[aligntSize:]):
              if v:
                align[alignCount + i] = (k, zhs2zht(v))
        repl = self._unescapeTranslation(repl, to=to, fr=fr, mark=mark, emit=emit)
        self.cache.update(text, repl)
    return repl, to, self.key

  def translateTest(self, text, to='en', fr='ja', async=False):
    """@reimp"""
    to, fr = self._checkLanguages(to, fr)
    try: return self._translateTest(self.engine.translate,
            text, to=to, fr=fr, async=async)
    except Exception, e: dwarn(e); return ''

class BaiduTranslator(OnlineMachineTranslator):
  key = 'baidu' # override
  asyncSupported = False # override  disable async
  alignSupported = True # override  enable translation alignment

  def __init__(self, session=None, **kwargs):
    super(BaiduTranslator, self).__init__(**kwargs)

    from kingsoft import iciba, icibadef
    iciba.session = session or requests.Session()
    self.iciba = iciba

    self.ICIBA_LANGUAGES = icibadef.MT_LANGUAGES

    from baidu import baidufanyi
    baidufanyi.session = session or requests.Session()
    self.baidufanyi = baidufanyi

  def getEngine(self, fr, to):
    """
    @param  fr  str
    @param  to  str
    @return baidu.baidufanyi or kingsoft.iciba
    """
    if fr in self.ICIBA_LANGUAGES and to in self.ICIBA_LANGUAGES:
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
  def translate(self, text, to='zhs', fr='ja', async=False, emit=False, mark=None, align=None, **kwargs):
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
    repl = self._escapeText(text, to=to, fr=fr, emit=emit)
    if repl:
      alignCount = len(align) if align else 0
      engine = self.baidufanyi if align is not None else self.getEngine(fr=fr, to=to)
      repl = self.__baidu_repl_before(repl)
      repl = self._translate(emit, repl,
          engine.translate,
          to, fr, async, align=align)
      if repl:
        if to == 'zht':
          #with SkProfiler(): # 10/19/2014: 1.34e-05 with python, 2.06-e5 with opencc
          repl = zhs2zht(repl)
          if align:
            for i,(k,v) in enumerate(align[alignCount:]):
              if v:
                align[alignCount + i] = (k, zhs2zht(v))
        repl = self.__baidu_repl_after(repl)
        repl = self._unescapeTranslation(repl, to=to, fr=fr, mark=mark, emit=emit)
        self.cache.update(text, repl)
    return repl, to, self.key

  def translateTest(self, text, to='en', fr='ja', async=False):
    """@reimp"""
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
#    text = tm.applySourceTerms(text, lang, host=self.key)
#    text = tm.prepareEscapeTerms(text, lang, host=self.key)
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
#      sub = tm.applyEscapeTerms(sub, lang, host=self.key)
#      sub = tm.applyTargetTerms(sub, lang, host=self.key)
#      sub = textutil.beautify_subtitle(sub)
#    return sub, lang, self.key
#
#  def translateTest(self, text, to='en', fr='ja', async=False):
#    """
#    @param  text  unicode
#    @param* fr  unicode
#    @param* async  bool  ignored, always sync
#    @return  unicode sub
#    """
#    try: return self._translateTest(youdaofanyi.translate, text, to=to, fr=fr, async=async)
#    except Exception, e: dwarn(e); return ''
