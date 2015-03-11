# coding: utf8
# termman.py
# 10/8/2012 jichi
#
# Terminology:
# - data: raw game text byte code
# - text: original game text
# - translation: machine translation
# - comment: user's subtitle or comment

#from sakurakit.skprof import SkProfiler

import os, string, re
#from collections import OrderedDict
from functools import partial
from time import time
from PySide.QtCore import Signal, QObject, QTimer, QMutex, Qt
#from rbmt import api as rbmt
from sakurakit import skfileio, skthreads
from sakurakit.skclass import memoized, Q_Q
from sakurakit.skdebug import dprint, dwarn
from mytr import my
import dataman, growl, rc
import _termman

@memoized
def manager(): return TermManager()

def _make_script_interpreter(type):
  if type == 'trans':
    from pytrscript import TranslationScriptPerformer
    return TranslationScriptPerformer()
  else:
    from pytrcodec import TranslationCoder
    return TranslationCoder()

@Q_Q
class _TermManager:

  instance = None # _TermManager  needed for updateTime

  def __init__(self, q):
    _TermManager.instance = self

    #self.convertsChinese = False
    self.enabled = True # bool
    self.hentai = False # bool
    self.marked = False # bool
    #self.syntax = False # bool

    # For saving terms
    self.updateTime = 0 # float

    self.targetLanguage = 'ja' # str  targetLanguage

    self.saveMutex = QMutex()

    self.scripts = {} # {(str type, str fr, str to):TranslationScriptPerformer or ScriptCoder}
    self.scriptLocks = {} #  {(str lang, str fr, str to):bool}
    self.scriptTimes = {} # [(str lang, str fr, str to):float time]

    #self.rbmt = {} # {str language:rbmt.api.Translator}
    #self.rbmtTimes = {} # [str language:float time]

    t = self.saveTimer = QTimer(q)
    t.setSingleShot(True)
    t.setInterval(2000) # wait for 2 seconds for rebuilding
    t.timeout.connect(self.saveTerms)

    q.invalidateCacheRequested.connect(t.start, Qt.QueuedConnection)
    q.cacheChangedRequested.connect(q.cacheChanged, Qt.QueuedConnection)

  def rebuildCacheLater(self, queued=False):
    if queued:
      self.q.invalidateCacheRequested.emit()
    else:
      self.saveTimer.start()

  def saveTerms(self):
    if not self.scriptTimes:
      return
    if not self.saveMutex.tryLock():
      dwarn("retry later due to thread contention")
      self.rebuildCacheLater(queued=True)
      return

    saveTime = time()
    skthreads.runsync(partial(
        self._saveTerms, saveTime))
    self.saveMutex.unlock()

  def _saveTerms(self, createTime):
    """Invoked async
    @param  createTime  float
    """
    #for lang,ts in self.targetLanguages.iteritems():
    scriptTimes = self.scriptTimes
    #rbmtTimes = self.rbmtTimes
    if not scriptTimes or createTime < self.updateTime:
      return

    dprint("enter")

    if scriptTimes and createTime >= self.updateTime:
      self._saveScriptTerms(createTime=createTime, times=scriptTimes)

    if createTime >= self.updateTime:
      dprint("cache changed")
      self.q.cacheChangedRequested.emit()
    dprint("leave")

  def _saveScriptTerms(self, createTime, times):
    """
    @param  createTime  float
    @param  times  {str key:float time}
    """
    dprint("enter")

    dm = dataman.manager()
    gameIds = dm.currentGameIds()
    dprint("current series gameIds = %s" % gameIds)
    if gameIds:
      gameIds = set(gameIds) # in case it is changed during iteration

    l = [t.d for t in dm.terms() if not t.d.disabled and not t.d.deleted and t.d.pattern] # filtered
    _termman.sort_terms(l)
    w = _termman.TermWriter(
      termData=l,
      gameIds=gameIds,
      hentai=self.hentai,
      createTime=createTime,
      parent=self,
    )

    #for scriptKey,ts in times.iteritems():
    for scriptKey,ts in times.items(): # back up items
      if ts < self.updateTime: # skip language that does not out of date
        type, to, fr = scriptKey
        macros = w.queryMacros(to, fr)

        if w.isOutdated():
          dwarn("leave: cancel saving out-of-date terms")
          return

        path = rc.term_path(type, to=to, fr=fr) # unicode
        dir = os.path.dirname(path) # unicode path
        if not os.path.exists(dir):
          skfileio.makedirs(dir)

        scriptKey = type, to, fr
        if self.scriptLocks.get(scriptKey):
          raise Exception("cancel saving locked terms")
        self.scriptLocks[scriptKey] = True

        man = self.scripts.get(scriptKey)
        if not man:
          man = self.scripts[scriptKey] = _make_script_interpreter(type)
        elif not man.isEmpty():
          man.clear()
        try:
          if w.saveTerms(path, type, to, fr, macros) and man.loadScript(path):
            dprint("type = %s, to = %s, fr = %s, count = %s" % (type, to, fr, man.size()))
        except:
          self.scriptLocks[scriptKey] = False
          raise

        self.scriptLocks[scriptKey] = False
        times[scriptKey] = createTime
    dprint("leave")

  def applyTerms(self, text, type, to, fr, host='', mark=False, ignoreIfNotReady=False):
    """
    @param  text  unicode
    @param  type  str
    @param  to  str  language
    @param* fr  str  language
    @param* key  str
    @param* mark  bool or None
    @param* ignoreIfNotReady bool
    """
    if not text:
      return text
    if mark is None:
      mark = self.marked
    if type in ('encode', 'decode'):
      key = 'trans', to, fr # share the same manager
    else:
      key = type, to, fr
    man = self.scripts.get(key)
    if man is None:
      self.scriptTimes[key] = 0
      self.rebuildCacheLater()
      if ignoreIfNotReady:
        dwarn("ignore text while processing shared dictionary")
        growl.notify(my.tr("Processing Shared Dictionary") + "...")
        return ''
    if self.scriptLocks.get(key):
      dwarn("skip applying locked script")
      if ignoreIfNotReady:
        dwarn("ignore text while processing shared dictionary")
        growl.notify(my.tr("Processing Shared Dictionary") + "...")
        return ''
      return text

    if not man or man.isEmpty():
      return text

    category = _termman.host_category(host)
    if type == 'encode':
      return man.encode(text, category)
    elif type == 'decode':
      return man.decode(text, category, mark)
    else
      return man.transform(text, category, mark)

class TermManager(QObject):

  ## Construction ##

  def __init__(self, parent=None):
    super(TermManager, self).__init__(parent)
    self.__d = _TermManager(self)

  cacheChanged = Signal()

  cacheChangedRequested = Signal() # private euse
  invalidateCacheRequested = Signal() # private use

  ## Properties ##

  #def isLocked(self): return self.__d.locked

  #def getRuleBasedTranslator(self, language):
  #  """
  #  @param  language  str
  #  @return rbmt.api.Translator or None
  #  """
  #  d = self.__d
  #  if not d.syntax or not d.enabled or not language:
  #    return
  #  ret = d.rbmt.get(language)
  #  if ret:
  #    return ret if ret.ruleCount() else None
  #  cabocha = cabochaman.cabochaparser()
  #  if not cabocha:
  #    dwarn("failed to create cabocha parser")
  #  else:
  #    ret = d.rbmt[language] = TermTranslator(cabocha, language, underline=d.marked)
  #    d.rbmtTimes[language] = 0
  #    d.rebuildCacheLater()

  def setTargetLanguage(self, v):
    d = self.__d
    if v and v != d.targetLanguage:
      d.targetLanguage = v
      # Reset translation scripts
      d.scripts = {}
      d.scriptTimes = {}
      # Reset rule-based translator
      #d.rbmt = {}
      #d.rbmtTimes = {}

  def isEnabled(self): return self.__d.enabled
  def setEnabled(self, value): self.__d.enabled = value

  def isHentaiEnabled(self): return self.__d.hentai
  def setHentaiEnabled(self, value):
    dprint(value)
    self.__d.hentai = value

  #def isSyntaxEnabled(self): return self.__d.syntax
  #def setSyntaxEnabled(self, value):
  #  dprint(value)
  #  self.__d.syntax = value

  def isMarked(self): return self.__d.marked
  def setMarked(self, t): self.__d.marked = t
    #d = self.__d
    #if d.marked != t:
    #  d.marked = t
    #  for key,man in d.scripts.iteritems():
    #    type = key[0]
    #    marked = t and type in ('output', 'trans_output')
    #    man.setLinkEnabled(marked)

    #  for it in d.rbmt.itervalues():
    #    it.setUnderline(t and it.isEscape())

  ## Marks ##

  #def clearMarkCache(self): # invoked on escapeMarked changes in settings
  #  for term in dataman.manager().iterEscapeTerms():
  #    term.applyReplace = None

  #def markEscapeText(self, text): # unicode -> unicode
  #  return _mark_text(text) if text and self.__d.marked else text

  #def removeMarks(self, text): # unicode -> unicode
  #  return textutil.remove_html_tags(text) if self.__d.marked else text

  #def convertsChinese(self): return self.__d.convertsChinese
  #def setConvertsChinese(self, value): self.__d.convertsChinese = value

  ## Cache ##

  def invalidateCache(self):
    d = self.__d
    d.updateTime = time()
    d.rebuildCacheLater()

  #def warmup(self, async=True, interval=0): # bool, int
  #  d = self.__d
  #  if not d.enabled or d.locked:
  #    return
  #  dprint("enter")
  #  dm = dataman.manager()

  #  task = partial(d.warmup,
  #      terms=dm.terms(),
  #      hasTitles=dm.hasTermTitles(),
  #      hentai=d.hentai,
  #      language=d.language)

  #  if not async:
  #    apply(task)
  #  else:
  #    d.locked = True
  #    if interval:
  #      skthreads.runasynclater(task, interval)
  #    else:
  #      skthreads.runasync(task)
  #  dprint("leave")

  ## Queries ##

  #def filterTerms(self, terms, language):
  #  """
  #  @param  terms  iterable dataman.Term
  #  @param  language  unicode
  #  @yield  Term
  #  """
  #  return self.__d.iterTerms(terms, language)

  def applyGameTerms(self, text, to=None, fr=None, ignoreIfNotReady=False):
    """
    @param  text  unicode
    @param* to  str
    @param* fr  str
    @param* ignoreIfNotReady  bool
    @return  unicode
    """
    d = self.__d
    # 9/25/2014: Qt 3e-05 seconds
    # 9/26/2014: Boost 4e-05 seconds
    #with SkProfiler():
    return d.applyTerms(text, 'game', to or d.targetLanguage, fr or 'ja', ignoreIfNotReady=ignoreIfNotReady) if d.enabled else text
    #return self.__d.applyTerms(dataman.manager().iterOriginTerms(), text, language)

  #def applyNameTerms(self, text, language):
  #  """
  #  @param  text  unicode
  #  @param  language  unicode
  #  @return  unicode
  #  """
  #  return self.__d.applyTerms(dataman.manager().iterNameTerms(), text, language)

  def applyTtsTerms(self, text, language=None):
    """
    @param  text  unicode
    @param* language  str
    @return  unicode
    """
    d = self.__d
    return d.applyTerms(text, 'tts', 'ja', language or d.targetLanguage) if d.enabled else text

  def applyOcrTerms(self, text, language=None):
    """
    @param  text  unicode
    @param* language  str
    @return  unicode
    """
    d = self.__d
    return d.applyTerms(text, 'ocr', 'ja', language or 'ja') if d.enabled else text

  def applyOutputTerms(self, text, to, fr, host='', mark=None):
    """
    @param  text  unicode
    @param  to  str  language
    @param  fr  str  language
    @param* host  str
    @param* mark  bool or None
    @return  unicode
    """
    d = self.__d
    # 9/25/2014: Qt 0.0003 seconds
    # 9/26/2014: Boost 0.0005 seconds, underline = True
    #with SkProfiler():
    return d.applyTerms(text, 'output', to, fr, host=host, mark=mark) if d.enabled else text
    #if d.marked and language.startswith('zh'):
    #  ret = ret.replace('> ', '>')
    #return self.__d.applyTerms(dataman.manager().iterTargetTerms(),
    #    text, language, convertsChinese=True, marksChanges=self.__d.marked)

  def applyInputTerms(self, text, to, fr, host=''):
    """
    @param  text  unicode
    @param  to  str  language
    @param  fr  str  language
    @param* host  str
    @return  unicode
    """
    d = self.__d
    # 9/25/2014: Qt 0.0005 seconds
    # 9/26/2014: Boost 0.001 seconds
    #with SkProfiler():
    return d.applyTerms(text, 'input', to, fr, host=host) if d.enabled else text
    #dm = dataman.manager()
    #d = self.__d
    #text = d.applyTerms(dm.iterSourceTerms(), text, language)
    #if text and dm.hasNameItems() and config.is_latin_language(d.targetLanguage):
    #  try:
    #    for name in dm.iterNameItems():
    #      if name.translation:
    #        text = name.replace(text)
    #  except Exception, e: dwarn(e)
    #  text = text.rstrip() # remove trailing spaces

  def encodeTranslation(self, text, to, fr, host=''):
    """
    @param  text  unicode
    @param  to  str  language
    @param  fr  str  language
    @param* host  str
    @return  unicode
    """
    d = self.__d
    if not d.enabled: #or not ESCAPE_ALL and not config.is_kanji_language(language):
      return text
    # 9/25/2014: Qt 0.01 seconds
    # 9/26/2014: Boost 0.033 seconds, underline = True
    # 9/27/2014: Boost 0.007 seconds, by delay rendering underline
    #with SkProfiler("prepare escape"): # 1/8/2015: 0.048 for Chinese, increase to 0.7 if no caching
    return d.applyTerms(text, 'encode', to, fr, host=host)

  def decodeTranslation(self, text, to, fr, host='', mark=None):
    """
    @param  text  unicode
    @param  to  str  language
    @param  fr  str  language
    @param* host  str
    @param* mark  bool or None
    @return  unicode
    """
    d = self.__d
    if not d.enabled: #or not ESCAPE_ALL and not config.is_kanji_language(language):
      return text
    # 9/25/2014: Qt 0.009 seconds
    # 9/26/2014: Boost 0.05 seconds, underline = True
    # 9/27/2014: Boost 0.01 seconds, by delaying rendering underline
    #with SkProfiler("apply escape"): # 1/8/2015: 0.051 for Chinese, increase to 0.7 if no caching
    ret = d.applyTerms(text, 'decode', to, fr, host=host, mark=mark)
    if d.marked and to.startswith('zh'):
      ret = ret.replace("> ", ">")
      ret = ret.replace(" <", "<")
    return ret

# EOF
