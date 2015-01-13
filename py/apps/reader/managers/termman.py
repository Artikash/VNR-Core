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

import os, re
#from collections import OrderedDict
from functools import partial
from time import time
from PySide.QtCore import Signal, QObject, QTimer, QMutex, Qt
from rbmt import api as rbmt
from sakurakit import skfileio, skos, skthreads
from sakurakit.skclass import memoized, Q_Q
from sakurakit.skdebug import dprint, dwarn
from convutil import kana2yomi, zhs2zht
import config, cabochaman, dataman, defs, i18n, rc

if skos.WIN:
  from pytrscript import TranslationScriptManager

@memoized
def manager(): return TermManager()

_re_marks = re.compile(r'<[0-9a-zA-Z: "/:=-]+?>')
def _remove_marks(text): return _re_marks.sub('', text)

SCRIPT_KEY_SEP = ',' # Separator of script manager key

class TermTitle(object):
  __slots__ = 'id', 'pattern', 'text', 'regex'
  def __init__(self, id, pattern, text, regex):
    self.id = id # long
    self.pattern = pattern # unicode
    self.text = text # unicode
    self.regex = regex # bool

class TermTranslator(rbmt.MachineTranslator):
  def __init__(self, cabocha, language, underline=True):
    escape = config.is_kanji_language(language)
    sep = '' if language.startswith('zh') else ' '
    super(TermTranslator, self).__init__(cabocha, language,
        sep=sep,
        escape=escape,
        underline=underline and escape)

  def translate(self, text, tr=None, fr="", to=""):
    return super(TermTranslator, self).translate(text, tr=tr)

class TermWriter:

  #@staticmethod
  #def _markText(text): # unicode -> unicode
  #  return '<span style="text-decoration:underline">%s</span>' % text

  RE_MACRO = re.compile('{{(.+?)}}')

  def __init__(self, createTime, termData, gameIds, hentai):
    self.createTime = createTime # float
    self.termData = termData # [_Term]
    self.gameIds = gameIds # set(ing gameId)
    self.hentai = hentai # bool

  def isOutdated(self): # -> bool
    return self.createTime < _TermManager.instance.updateTime

  def saveTerms(self, path, type, language, macros, titles):
    """This method is invoked from a different thread
    @param  path  unicode
    @param  type  str  term type
    @param  language  str  target text language
    @param  macros  {unicode pattern:unicode repl}
    @param  titles  [TermTitle] not None not empty
    @return  bool
    """
    #marksChanges = self.marked and type in ('target', 'escape_target')
    convertsChinese = language == 'zht' and type in ('target', 'escape_source', 'escape_target')
    if type not in ('source', 'escape_source', 'escape_target'):
      titles = None

    titleCount = len(titles) if titles else 0 # int

    empty = True

    escape_source = type == 'escape_source'
    escape_target = type == 'escape_target'

    count = len(self.termData)
    try:
      with open(path, 'w') as f:
        f.write(self._renderHeader(type, language))
        for index,td in enumerate(self._iterTermData(type, language)):
          if self.isOutdated():
            raise Exception("cancel saving out-of-date terms")
          z = convertsChinese and td.language == 'zhs'
          # no padding space for Chinese names
          padding = escape_source or (
              td.language not in ('ja', 'ko', 'zhs', 'zht')
              and td.type in ('name', 'yomi', 'translation'))

          regex = td.regex and not escape_target

          if escape_source or escape_target:
            priority = count - index
            key = defs.TERM_ESCAPE % priority

          if escape_source:
            repl = key
          else:
            repl = td.text
            if repl:
              if z:
                repl = zhs2zht(repl)
              if td.type == 'yomi':
                repl = kana2yomi(repl, language) or repl

              #elif config.is_latin_language(td.language):
              #  repl += " "
              #if marksChanges:
              #  repl = self._markText(repl)

          if escape_target:
            pattern = key
          else:
            pattern = td.pattern
            if regex:
              pattern = self._applyMacros(pattern, macros)
            if z:
              pattern = zhs2zht(pattern)

          # See: http://stackoverflow.com/questions/6005821/how-to-do-multiple-substitutions-using-boost-regex
          # pattern: A(?:(sama)|(1)|(2)|(4)|(5)|(6)|(7)|(8)|(9)|(sam))
          # repl: BCD(?1yy)(?10xxx)
          #if titleCount and td_is_name:
          #  if escape_source:
          #    repl_prefix = (defs.NAME_ESCAPE_PREFIX % priority) + "."
          #    repl_suffix = defs.NAME_ESCAPE_SUFFIX + " " # padding space
          #    pattern += r"(?:"
          #    for i,k in enumerate(titles.iterkeys()):
          #      if i:
          #        pattern += r"|(%s)" % k
          #        repl_prefix += r":(?%i%i" % (i+1,titleCount-i)
          #        repl_suffix = r")" + repl_suffix
          #      else: # first
          #        pattern += r"(%s)" % k
          #        repl_prefix += r"(?%i" % (titleCount-i)
          #        repl_suffix = r")" + repl_suffix
          #    pattern += r"|)" # trailing "|" so that the title can be empty
          #    repl = repl_prefix + repl_suffix
          #  elif escape_target:
          #    pattern = (defs.NAME_ESCAPE_PREFIX % priority) + r"\.(?:" # escape the dot
          #    for i,v in enumerate(titles.itervalues()):
          #      if i:
          #        pattern += r"|(%i)" % (titleCount-i)
          #      else: # first
          #        pattern += r"(%i)" % (titleCount-i)
          #      if v:
          #        repl += r"(?%i%s)" % (i+1,v)
          #    pattern += r")" + defs.NAME_ESCAPE_SUFFIX
          #    #repl+= " " # do not padding space as only Chinese/Korean uses escaped terms
          #  else:
          #    pattern += r"(?:"
          #    for i,(k,v) in enumerate(titles.iteritems()):
          #      if i:
          #        pattern += r"|(%s)" % k
          #      else: # first
          #        pattern += r"(%s)" % k
          #      if v:
          #        repl += r"(?%i%s)" % (i+1,v)
          #    pattern += r")"
          #    repl += " " # padding space

          name = None
          if titleCount and td.type in ('name', 'yomi'):
            if escape_source:
              name = True
              esc = defs.NAME_ESCAPE + " " # padding space
              for i,it in enumerate(titles):
                self._writeLine(f,
                    td.id,
                    (re.escape(pattern) if not regex and it.regex else pattern) + it.pattern,
                    esc % (priority, titleCount - i),
                    regex or it.regex,
                    name=False)
            elif escape_target:
              esc = defs.NAME_ESCAPE
              for i,it in enumerate(titles):
                self._writeLine(f,
                    td.id,
                    esc % (priority, titleCount - i),
                    repl + it.text + (" " if padding else ""), # it will be escaped in C++
                    #(re.escape(repl) if not regex and it.regex else repl) + it.text + (" " if padding else ""),
                    regex) # no padding space for Chinese names
            else:
              name = True
              #assert padding # this is supposed to be always true
              for it in titles:
                self._writeLine(f,
                    td.id,
                    (re.escape(pattern) if not regex and it.regex else pattern) + it.pattern,
                    repl + it.text + " ",
                    regex or it.regex,
                    name=False) # padding space for Japanese names

          if padding:
            repl += " "
          self._writeLine(f, td.id, pattern, repl, regex, name=name)

          empty = False

      if not empty:
        return True

    except Exception, e:
      dwarn(e)

    skfileio.removefile(path) # Remove file when failed
    return False

  @staticmethod
  def _writeLine(f, tid, pattern, repl, regex, name=None):
    """
    @param  f  file
    @param  tid  long
    @param  pattern  unicode
    @param  repl  unicode
    @param  regex  bool
    @param  name  True (name) of False (suffix) or None
    @return  unicode or None
    """
    if '\n' in pattern or '\n' in repl:
      dwarn("skip new line in term: id = %s" % tid)
      return
    ret = '\t'.join((str(tid), pattern, repl) if repl else (str(tid), pattern))
    ret = "\t%s\n" % ret
    if regex:
      ret = 'r' + ret
    if name is not None:
      if name:
        ret = 'n' + ret # a name without suffix
      else:
        ret = 's' + ret # a name with suffix
    f.write(ret)

  def _renderHeader(self, type, language):
    """
    @param  type  str
    @param  language  str
    @return  unicode
    """
    return """\
# This file is automatically generated for debugging purposes.
# Modifying this file will NOT affect VNR.
#
# Unix time: %s
# Options: type = %s, language = %s, hentai = %s, games = (%s)
#
""" % (self.createTime, type, language, self.hentai,
    ','.join(map(str, self.gameIds)) if self.gameIds else 'empty')

  def _iterTermData(self, type, language, syntax=False):
    """
    @param  type  str
    @param  language  str
    @param* syntax  bool
    @yield  _Term
    """
    types = [type]
    if type.startswith('escape'):
      types[0] = 'escape' # override type
      if config.is_kanji_language(language):
        types.append('name')
        if language == 'ko':
          types.append('yomi')
    elif type == 'source' and not config.is_kanji_language(language):
      types.append('escape')
      types.append('name')
      types.append('yomi')

    types = frozenset(types)
    for td in self.termData:
      if (#not td.disabled and not td.deleted and td.pattern # in case pattern is deleted
          td.type in types
          and (not td.special or self.gameIds and td.gameId and td.gameId in self.gameIds)
          and (not td.hentai or self.hentai)
          and i18n.language_compatible_to(td.language, language)
          and td.syntax == syntax
        ):
        yield td

  def queryTermMacros(self, language):
    """
    @param  language  str
    @return  {unicode pattern:unicode repl} not None
    """
    ret = {td.pattern:td.text for td in self._iterTermData('macro', language)}
    MAX_ITER_COUNT = 1000
    for count in xrange(1, MAX_ITER_COUNT):
      dirty = False
      for pattern,text in ret.iteritems(): # not iteritems as I will modify ret
        if text and defs.TERM_MACRO_BEGIN in text:
          dirty = True
          ok = False
          for m in self.RE_MACRO.finditer(text):
            macro = m.group(1)
            repl = ret.get(macro)
            if repl:
              text = text.replace("{{%s}}" % macro, repl)
              ok = True
            else:
              dwarn("missing macro", macro, text)
              ok = False
              break
          if ok:
            ret[pattern] = text
          else:
            ret[pattern] = None # delete this pattern
      if not dirty:
        break
    if count == MAX_ITER_COUNT - 1:
      dwarn("recursive macro definition")
    return {k:v for k,v in ret.iteritems() if v is not None}

  def queryTermTitles(self, language, macros):
    """Terms sorted by length and id
    @param  language
    @param  macros  {unicode pattern:unicode repl}
    @return  [TermTitle]
    """
    zht = language == 'zht'
    l = [] # [long id, unicode pattern, unicode replacement, bool regex]
    #ret = OrderedDict({'':''})
    #ret = OrderedDict()
    for td in self._iterTermData('title', language):
      pat = td.pattern
      if td.regex:
        pat = self._applyMacros(pat, macros)
      repl = td.text
      if zht and td.language == 'zhs':
        pat = zhs2zht(pat)
        if repl: # and self.convertsChinese:
          repl = zhs2zht(repl)
      l.append(TermTitle(
        id=td.id,
        pattern=pat,
        text=repl,
        regex=td.regex,
      ))
    l.sort(reverse=True, key=lambda it:
        (it.regex, len(it.pattern), it.id)) # regex terms come at first, longer terms come at first, newer come at first
    #for id,pat,repl,regex in l:
    #  ret[pat] = TermTitle(repl, regex)
    return l

  def _applyMacros(self, text, macros):
    """
    @param  text  unicode
    @param  macros  {unicode pattern:unicode repl}
    @return  unicode
    """
    for m in self.RE_MACRO.finditer(text):
      macro = m.group(1)
      repl = macros.get(macro)
      if repl is None:
        dwarn("missing macro", macro)
      else:
        text = text.replace("{{%s}}" % macro, repl)
    return text

@Q_Q
class _TermManager:

  instance = None # _TermManager  needed for updateTime

  # Cover all term types, but decouple escape into before and after
  #SAVE_TYPES = 'origin', 'source', 'target', 'speech', 'ocr', 'escape_source', 'escape_target'

  def __init__(self, q):
    _TermManager.instance = self

    #self.convertsChinese = False
    self.enabled = True # bool
    self.hentai = False # bool
    self.marked = False # bool
    self.syntax = False # bool

    # For saving terms
    self.updateTime = 0 # float

    self.targetLanguage = 'ja' # str  targetLanguage

    self.saveMutex = QMutex()

    self.scripts = {} # {str key:TranslationScriptManager}  key = lang + type
    self.scriptTimes = {} # [str key:float time]

    self.rbmt = {} # {str language:rbmt.api.Translator}
    self.rbmtTimes = {} # [str language:float time]

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

  def _createScriptManager(self, type, language): # unicode, unicode -> TranslationScriptManager
    key = SCRIPT_KEY_SEP.join((type, language))
    ret = self.scripts.get(key)
    if not ret:
      ret = self.scripts[key] = TranslationScriptManager()
      ret.setLinkEnabled(self.marked and type in ('target', 'escape_target'))
    return ret

  #@classmethod
  #def needsEscape(cls):
  #  return config.is_kanji_language(cls.language)

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
    rbmtTimes = self.rbmtTimes
    if (not scriptTimes and not rbmtTimes) or createTime < self.updateTime:
      return

    dprint("enter")

    dm = dataman.manager()

    gameIds = dm.currentGameIds()
    if gameIds:
      gameIds = set(gameIds) # in case it is changed during iteration

    termData = (t.d for t in dm.terms() if not t.d.disabled and not t.d.deleted and t.d.pattern) # filtered
    termData = sorted(termData, reverse=True, key=lambda td:
          (len(td.pattern), td.private, td.special, td.id)) # it.regex  true is applied first

    if scriptTimes and createTime >= self.updateTime:
      self._saveScriptTerms(createTime=createTime, termData=termData, gameIds=gameIds, times=scriptTimes)

    if rbmtTimes and createTime >= self.updateTime:
      self._saveSyntaxTerms(createTime=createTime, termData=termData, gameIds=gameIds, times=rbmtTimes)

    if createTime >= self.updateTime:
      dprint("cache changed")
      self.q.cacheChangedRequested.emit()
    dprint("leave")

  def _saveSyntaxTerms(self, createTime, termData, gameIds, times):
    """
    @param  createTime  float
    @param  termData  [_Term]
    @param  gameIds  [int] or None
    @param  times  {str key:float time}
    """
    dprint("enter")
    rules = []
    hentai = self.hentai
    for language, time in times.iteritems():
      convertsChinese = language == 'zht'
      for td in termData:
        if (#not td.disabled and not td.deleted and td.pattern # in case pattern is deleted
            td.syntax and td.type == 'escape'
            and (not td.special or gameIds and td.gameId and td.gameId in gameIds)
            and (not td.hentai or hentai)
            and i18n.language_compatible_to(td.language, language)
          ):
          if createTime < self.updateTime:
            dwarn("leave: cancel saving out-of-date syntax terms")
            return
          z = convertsChinese and td.language == 'zhs'
          pattern = td.pattern
          repl = td.text
          if repl and z:
            repl = zhs2zht(repl)
          rule = rbmt.createrule(pattern, repl, language)
          if not rule:
            dwarn("failed to parse rule:", source, target)
          else:
            rules.append(rule)

      mt = self.rbmt[language]
      if createTime < self.updateTime:
        dwarn("leave: cancel saving out-of-date syntax terms")
        return
      rules.sort(key=lambda it:-it.priority())
      mt.setRules(rules)
      times[language] = createTime
      dprint("lang = %s, count = %s" % (language, mt.ruleCount()))

    dprint("leave")

  def _saveScriptTerms(self, createTime, termData, gameIds, times):
    """
    @param  createTime  float
    @param  termData  [_Term]
    @param  gameIds  [int] or None
    @param  times  {str key:float time}
    """
    dprint("enter")
    w = TermWriter(
      createTime=createTime,
      termData=termData, # not back up to save memory
      gameIds=gameIds,
      hentai=self.hentai,
      #marked=self.marked,
    )

    #for scriptKey,ts in times.iteritems():
    for scriptKey,ts in times.items(): # back up items
      if ts < self.updateTime: # skip language that does not out of date
        type, lang = scriptKey.split(SCRIPT_KEY_SEP)
        macros = w.queryTermMacros(lang)
        titles = w.queryTermTitles(lang, macros)

        if w.isOutdated():
          dwarn("leave: cancel saving out-of-date terms")
          return

        path = rc.term_path(type, lang) # unicode
        dir = os.path.dirname(path) # unicode path
        if not os.path.exists(dir):
          skfileio.makedirs(dir)

        man = self._createScriptManager(type, lang)
        if not man.isEmpty():
          man.clear()

        if w.saveTerms(path, type, lang, macros, titles) and man.loadFile(path):
          dprint("type = %s, lang = %s, count = %s" % (type, lang, man.size()))

        times[scriptKey] = createTime
    dprint("leave")

  def applyTerms(self, text, type, language):
    """
    @param  text  unicode
    @param  type  str
    @param  language  str
    """
    # TODO: Schedule to update terms when man is missing
    key = SCRIPT_KEY_SEP.join((type, language))
    man = self.scripts.get(key)
    if man is None:
      self.scriptTimes[key] = 0
      self.rebuildCacheLater()
    return man.translate(text) if man and not man.isEmpty() else text

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

  def getRuleBasedTranslator(self, language):
    """
    @param  language  str
    @return rbmt.api.Translator or None
    """
    d = self.__d
    if not d.syntax or not d.enabled or not language:
      return
    ret = d.rbmt.get(language)
    if ret:
      return ret if ret.ruleCount() else None
    cabocha = cabochaman.cabochaparser()
    if not cabocha:
      dwarn("failed to create cabocha parser")
    else:
      ret = d.rbmt[language] = TermTranslator(cabocha, language, underline=d.marked)
      d.rbmtTimes[language] = 0
      d.rebuildCacheLater()

  def setTargetLanguage(self, v):
    d = self.__d
    if v and v != d.targetLanguage:
      d.targetLanguage = v
      # Reset translation scripts
      d.scripts = {}
      d.scriptTimes = {}
      # Reset rule-based translator
      d.rbmt = {}
      d.rbmtTimes = {}

  def isEnabled(self): return self.__d.enabled
  def setEnabled(self, value): self.__d.enabled = value

  def isHentaiEnabled(self): return self.__d.hentai
  def setHentaiEnabled(self, value):
    dprint(value)
    self.__d.hentai = value

  def isSyntaxEnabled(self): return self.__d.syntax
  def setSyntaxEnabled(self, value):
    dprint(value)
    self.__d.syntax = value

  def isMarked(self): return self.__d.marked
  def setMarked(self, t):
    d = self.__d
    if d.marked != t:
      d.marked = t
      for key,man in d.scripts.iteritems():
        type = key.split(SCRIPT_KEY_SEP)[0]
        marked = t and type in ('target', 'escape_target')
        man.setLinkEnabled(marked)

      for it in d.rbmt.itervalues():
        it.setUnderline(t and it.isEscape())

  ## Marks ##

  #def clearMarkCache(self): # invoked on escapeMarked changes in settings
  #  for term in dataman.manager().iterEscapeTerms():
  #    term.applyReplace = None

  #def markEscapeText(self, text): # unicode -> unicode
  #  return _mark_text(text) if text and self.__d.marked else text

  def removeMarks(self, text): # unicode -> unicode
    return _remove_marks(text) if self.__d.marked else text

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

  def applyTargetTerms(self, text, language):
    """
    @param  text  unicode
    @param  language  unicode
    @return  unicode
    """
    d = self.__d
    # 9/25/2014: Qt 0.0003 seconds
    # 9/26/2014: Boost 0.0005 seconds, underline = True
    #with SkProfiler():
    return d.applyTerms(text, 'target', language) if d.enabled else text
    #if d.marked and language.startswith('zh'):
    #  ret = ret.replace('> ', '>')
    #return self.__d.applyTerms(dataman.manager().iterTargetTerms(),
    #    text, language, convertsChinese=True, marksChanges=self.__d.marked)

  def applyOriginTerms(self, text, language):
    """
    @param  text  unicode
    @param  language  unicode
    @return  unicode
    """
    d = self.__d
    # 9/25/2014: Qt 3e-05 seconds
    # 9/26/2014: Boost 4e-05 seconds
    #with SkProfiler():
    return d.applyTerms(text, 'origin', language or d.targetLanguage) if d.enabled else text
    #return self.__d.applyTerms(dataman.manager().iterOriginTerms(), text, language)

  #def applyNameTerms(self, text, language):
  #  """
  #  @param  text  unicode
  #  @param  language  unicode
  #  @return  unicode
  #  """
  #  return self.__d.applyTerms(dataman.manager().iterNameTerms(), text, language)

  def applySpeechTerms(self, text, language=None):
    """
    @param  text  unicode
    @param  language  unicode
    @return  unicode
    """
    d = self.__d
    return d.applyTerms(text, 'speech', language or d.targetLanguage) if d.enabled else text

  def applyOcrTerms(self, text, language=None):
    """
    @param  text  unicode
    @param  language  unicode
    @return  unicode
    """
    d = self.__d
    return d.applyTerms(text, 'ocr', language or d.targetLanguage) if d.enabled else text

  def applySourceTerms(self, text, language):
    """
    @param  text  unicode
    @param  language  unicode
    @return  unicode
    """
    d = self.__d
    # 9/25/2014: Qt 0.0005 seconds
    # 9/26/2014: Boost 0.001 seconds
    #with SkProfiler():
    return d.applyTerms(text, 'source', language) if d.enabled else text
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

  def prepareEscapeTerms(self, text, language):
    """
    @param  text  unicode
    @param  language  unicode
    @return  unicode
    """
    d = self.__d
    if not d.enabled or not config.is_kanji_language(language):
      return text
    # 9/25/2014: Qt 0.01 seconds
    # 9/26/2014: Boost 0.033 seconds, underline = True
    # 9/27/2014: Boost 0.007 seconds, by delay rendering underline
    #with SkProfiler("prepare escape"): # 1/8/2014: 0.048 for Chinese, increase to 0.7 if no caching
    return d.applyTerms(text, 'escape_source', language)

  def applyEscapeTerms(self, text, language):
    """
    @param  text  unicode
    @param  language  unicode
    @return  unicode
    """
    d = self.__d
    if not d.enabled or not config.is_kanji_language(language):
      return text
    # 9/25/2014: Qt 0.009 seconds
    # 9/26/2014: Boost 0.05 seconds, underline = True
    # 9/27/2014: Boost 0.01 seconds, by delaying rendering underline
    #with SkProfiler("apply escape"): # 1/8/2014: 0.051 for Chinese, increase to 0.7 if no caching
    ret = d.applyTerms(text, 'escape_target', language)
    if d.marked and language.startswith('zh'):
      ret = ret.replace("> ", ">")
      ret = ret.replace(" <", "<")
    return ret

# EOF

#  # Escaped
#
#  def prepareEscapeTerms(self, text, language):
#    """
#    @param  text  unicode
#    @param  language  unicode
#    @return  unicode
#    """
#    d = self.__d
#    if not d.enabled or d.locked:
#      return text
#    dm = dataman.manager()
#    hasTitles = dm.hasTermTitles() # cached
#    esc = defs.TERM_ESCAPE + ' '
#    for term in dm.iterEscapeTerms():
#      td = term.d # To improve performance
#      if (not td.hentai or d.hentai) and td.pattern and i18n.language_compatible_to(td.language, language):
#        if hasTitles and term.needsReplace():
#          try: text = term.prepareReplace(text)
#          except Exception, e: dwarn(td.pattern, td.text, e)
#        else:
#          h = term.priority or td.id or id(term)
#          key = esc % h
#          if not term.patternNeedsRe():
#            text = text.replace(td.pattern, key)
#          else:
#            try: text = term.patternRe.sub(key, text)
#            except Exception, e: dwarn(td.pattern, td.text, e)
#        if not text:
#          break
#    #if text and dm.hasNameItems() and config.is_kanji_language(d.targetLanguage):
#    #  try:
#    #    for name in dm.iterNameItems():
#    #      if name.translation:
#    #        text = name.prepareReplace(text)
#    #  except Exception, e: dwarn(e)
#    return text
#
#  def applyEscapeTerms(self, text, language):
#    """
#    @param  text  unicode
#    @param  language  unicode
#    @return  unicode
#    """
#    d = self.__d
#    if not d.enabled or d.locked:
#      return text
#    dm = dataman.manager()
#    hasTitles = dm.hasTermTitles() # cached
#    esc = defs.TERM_ESCAPE
#    zht = language == 'zht' # cached
#    for term in dm.iterEscapeTerms():
#      td = term.d # To improve performance
#      if (not td.hentai or d.hentai) and td.pattern and i18n.language_compatible_to(td.language, language):
#        if hasTitles and term.needsReplace():
#          try: text = term.applyReplace(text)
#          except Exception, e: dwarn(td.pattern, td.text, e)
#        else:
#          #repl = term.bbcodeText if term.bbcode else term.text
#          repl = td.text
#          if repl:
#            if zht and td.language == 'zhs':
#              repl = zhs2zht(repl)
#            if d.marked:
#              repl = _mark_text(repl)
#          #elif config.is_latin_language(td.language):
#          #  repl += " "
#          h = term.priority or td.id or id(term)
#          key = esc % h
#          text = text.replace(key, repl)
#        if not text:
#          break
#    #if text and dm.hasNameItems() and config.is_kanji_language(d.targetLanguage):
#    #  try:
#    #    for name in dm.iterNameItems():
#    #      if name.translation:
#    #        text = name.applyReplace(text)
#    #  except Exception, e: dwarn(e)
#    return text
#
#  ## MeCab ##
#
#  def applyWordTerms(self, text):
#    """
#    @param  text  unicode
#    @return  unicode
#    """
#    # This feature is disabled
#    # Compiled MeCab dictionary id preferred
#    return text
#
#   #d = self.__d
#   #if not d.enabled or d.locked:
#   #  return text
#   #dm = dataman.manager()
#   #for term in dm.iterWordTerms():
#   #  td = term.d
#   #  if (not td.hentai or d.hentai) and not td.regex: #and td.language == 'ja': # skip using regular expressions, ignore languages
#   #    text = text.replace(td.pattern, " %s " % td.pattern)
#   #if dm.hasNameItems():
#   #  for name in dm.iterNameItems():
#   #    text = text.replace(name.text, " %s " % name.text)
#   #return text.strip()
#
#   Temporarily disabled for being slow
#  def queryLatinWordTerms(self, text):
#    """
#    @param  text  unicode
#    @return  unicode or None
#    """
#    d = self.__d
#    if not d.enabled or d.locked:
#      return text
#    dm = dataman.manager()
#    for term in dm.iterLatinSourceTerms():
#      td = term.d
#      if (not td.hentai or d.hentai) and td.pattern and not td.regex and td.language == 'en':
#        if text == td.pattern and td.text:
#          return td.text.capitalize() if td.type == 'name' else td.text
#
#   Temporarily disabled for being slow
#  def queryFuriTerms(self, text):
#    """
#    @param  text  unicode
#    @return  unicode or None
#    """
#    d = self.__d
#    if not d.enabled or d.locked:
#      return text
#    dm = dataman.manager()
#    for term in dm.iterFuriTerms():
#      td = term.d
#      if (not td.hentai or d.hentai) and td.pattern and not td.regex and td.language == 'ja': # skip using regular expressions
#        if text == td.pattern and td.text:
#          return td.text
#
#   #if dm.hasNameItems():
#    #  for name in dm.iterNameItems():
#    #    if text == name.text:
#    #      return name.yomi or text
#
#  def applyTerms(self, terms, text, language, convertsChinese=False, marksChanges=False):
#    """
#    @param  terms  iterable dataman.Term
#    @param  text  unicode
#    @param  language  unicode
#    @param* convertsChinese  bool
#    @param*  marksChanges  bool  mark the replacement text
#    @return  unicode
#    """
#    if not self.enabled:
#      return text
#    dm = dataman.manager()
#    hasTitles = dm.hasTermTitles() # cached
#    zht = language == 'zht' # cached
#    for term in terms:
#      td = term.d # To improve performance
#      if (not td.hentai or self.hentai) and td.pattern and i18n.language_compatible_to(td.language, language):
#        if hasTitles and term.needsReplace():
#          try: text = term.replace(text)
#          except Exception, e: dwarn(td.pattern, td.text, e)
#        else:
#          z = convertsChinese and zht and td.language == 'zhs'
#          #repl = term.bbcodeText if term.bbcode else term.text
#          repl = td.text
#          if repl:
#            if z: # and self.convertsChinese:
#              repl = zhs2zht(repl)
#            #elif config.is_latin_language(td.language):
#            #  repl += " "
#            if marksChanges:
#              repl = _mark_text(repl)
#          if not term.patternNeedsRe():
#            pattern = zhs2zht(td.pattern) if z else td.pattern
#            text = text.replace(pattern, repl)
#          else:
#            try: text = term.patternRe.sub(repl, text)
#            except Exception, e: dwarn(td.pattern, td.text, e)
#        if not text: # well, the text is deleted by terms
#          break
#    return text
#
#  def iterTerms(self, terms, language):
#    """
#    @param  terms  iterable dataman.Term
#    @param  language  unicode
#    @yield  Term
#    """
#    if not self.enabled:
#      return
#    for term in terms:
#      td = term.d # To improve performance
#      if (not td.hentai or self.hentai) and td.pattern and i18n.language_compatible_to(td.language, language):
#        yield term
#
#  def warmup(self, terms, language='', hasTitles=False, hentai=False): # [dataman.Term], str, bool ->
#    dprint("enter")
#    needsEscape = config.is_kanji_language(language)
#    for term in terms:
#      td = term.d # To improve performance
#      if not td.disabled and not td.special and (not td.hentai or hentai) and td.pattern and i18n.language_compatible_to(td.language, language):
#        if hasTitles and term.needsReplace():
#          try:
#            if needsEscape:
#              term.prepareReplace
#              term.applyReplace
#            else:
#              term.replace
#          except Exception, e: dwarn(td.pattern, td.text, e)
#        elif td.pattern and term.patternNeedsRe():
#          try: term.patternRe
#          except Exception, e: dwarn(td.pattern, td.text, e)
#
#    import trman
#    trman.manager().clearCacheRequested.emit()
#
#    self.locked = False
#    dprint("leave")
#
