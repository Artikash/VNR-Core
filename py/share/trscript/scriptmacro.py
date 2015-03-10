# coding: utf8
# scriptmacro.py
# 10/8/2012 jichi

__all__ = 'ScriptQueryInterface', 'ScriptQuery'

#from sakurakit.skprof import SkProfiler

class ScriptQueryInterface(object):
  pass

class ScriptQuery(ScriptWriterInterface):
  def __init__(self, scriptData):
    """
    @param  scriptData  [Term]
    """
    self.data = scriptData

  RE_MACRO = re.compile('{{(.+?)}}')

  def _iterTermData(self, type, to, fr):
    """
    @param  type  str
    @param  to  str
    @param  fr  str
    @yield  _Term
    """
    if type.startswith('trans'): # trans, trans_input, trans_output
      types = ['trans', 'name']
      if not to.startswith('zh'):
        types.append('yomi')
    elif type == 'suffix':
      types = 'suffix', 'prefix'
    else:
      types = type,
    #elif type == 'input' and not ESCAPE_ALL and not config.is_kanji_language(language):
    #  types.append('trans')
    #  types.append('name')
    #  types.append('yomi')

    # Types do not apply to non-Japanese languages
    jatypes = 'macro', 'game', 'ocr'
    zhtypes = 'name', 'yomi'
    fr2 = fr[:2]
    fr_is_latin = config.is_latin_language(fr)
    #items = set() # skip duplicate names
    types = frozenset(types)
    for td in self.termData:
      if (#not td.disabled and not td.deleted and td.pattern # in case pattern is deleted
          td.type in types
          and (not td.hentai or self.hentai)
          and i18n.language_compatible_to(td.language, to)
          and (not td.special or self.gameIds and td.gameId and td.gameId in self.gameIds)
          and (td.sourceLanguage.startswith(fr2)
            or fr != 'en' and fr_is_latin and td.sourceLanguage == 'en'
            or td.sourceLanguage == 'ja' and (
              td.type in jatypes
              or fr2 == 'zh' and td.type in zhtypes
            )
          )
        ): #and (td.type, td.pattern) not in items:
        #items.add((td.type, td.pattern))
        yield td

  def queryTermMacros(self, to, fr):
    """
    @param  to  str
    @param  fr  str
    @return  {unicode pattern:unicode repl} not None
    """
    ret = {_unescape_term_text(td.pattern):_unescape_term_text(td.text) for td in self._iterTermData('macro', to, fr)}
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

  def queryTermTitles(self, to, fr, macros):
    """Terms sorted by length and id
    @param  to  str
    @param  fr  str
    @param  macros  {unicode pattern:unicode repl}
    @return  [TermTitle]
    """
    zht = to == 'zht'
    l = [] # [long id, unicode pattern, unicode replacement, bool regex]
    #ret = OrderedDict({'':''})
    #ret = OrderedDict()
    s = _get_lang_suffices(to, fr)
    if s:
      for k,v in s:
        l.append(TermTitle(pattern=k, text=v, sortKey=(0,0)))
    for td in self._iterTermData('suffix', to, fr):
      pat = td.pattern
      if td.regex:
        pat = self._applyMacros(pat, macros)
      repl = td.text
      if zht and td.language == 'zhs':
        pat = zhs2zht(pat)
        if repl: # and self.convertsChinese:
          repl = zhs2zht(repl)
      sortKey = _lang_sort_key(td.language, td.sourceLanguage), td.updateTimestamp or td.timestamp
      prefix = td.type == 'prefix'
      l.append(TermTitle(sortKey=sortKey,
        prefix=prefix,
        pattern=pat,
        text=repl,
        regex=td.regex,
        phrase=td.phrase,
        icase=td.icase,
      ))
      if s and not prefix:
        for k,v in s:
          l.append(TermTitle(sortKey=sortKey,
            pattern=pat + k,
            text=repl + v,
            regex=td.regex,
            phrase=td.phrase,
            icase=td.icase,
          ))
    # prefix terms at first, regex terms come at first, longer terms come at first, case-sensitive at first, newer come at first
    l.sort(reverse=True, key=lambda it:
        (it.prefix, it.regex, len(it.pattern), not it.icase, it.sortKey))
    #for id,pat,repl,regex in l:
    #  ret[pat] = TermTitle(repl, regex)

    for it in l:
      it.pattern = _unescape_term_text(it.pattern)
      it.text = _unescape_term_text(it.text)
      if it.phrase:
        it.phrase = False
        left = it.pattern[0]
        right = it.pattern[-1]
        if not it.regex:
          it.regex = True
          it.pattern = re.escape(it.pattern)
        if it.prefix:
          it.pattern = _phrase_lbound(left, fr) + it.pattern
        else:
          it.pattern += _phrase_rbound(right, fr)
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































import os, string, re
#from collections import OrderedDict
from functools import partial
from time import time
from PySide.QtCore import Signal, QObject, QTimer, QMutex, Qt
#from rbmt import api as rbmt
from sakurakit import skfileio, skos, skstr, skthreads
from sakurakit.skclass import memoized, Q_Q
from sakurakit.skdebug import dprint, dwarn
from opencc import opencc
from unitraits import jpchars, jpmacros
from convutil import kana2name, zhs2zht
from mytr import my
import config, cabochaman, dataman, defs, i18n, rc


def _unescape_term_text(text):
  """
  @param  text  unicode
  @return  unicode
  """
  if not text or '&' not in text or ';' not in text:
    return text
  return skstr.unescapehtml(text).replace('&eos;', defs.TERM_ESCAPE_EOS)

def _phrase_lbound(text, language):
  """
  @param  text  unicode
  @param  language  str
  @return  unicode
  """
  if not text:
    return text
  ch = text[0]
  if language == 'ja':
    cat = jpchars.getcat(ch)
    if cat:
      m = jpmacros.getmacro('?<!' + cat)
      if m:
        return m
  return r'\b' if ch not in string.punctuation else ''

def _phrase_rbound(text, language):
  """
  @param  text  unicode
  @param  language  str
  @return  unicode
  """
  if not text:
    return text
  ch = text[-1]
  if language == 'ja':
    cat = jpchars.getcat(ch)
    if cat:
      m = jpmacros.getmacro('?!' + cat)
      if m:
        return m
  return r'\b' if ch not in string.punctuation else ''

def _combine_name_title(name, title, prefix, paddingPrefix):
  """
  @param  name  unicode
  @param  title  unicode
  @param  prefix  bool
  @param  paddingPrefix  bool
  @return  unicode
  """
  if defs.TERM_CLASS_NAME in title:
    return title.replace(defs.TERM_CLASS_NAME, name)
  elif not prefix: # suffix
    return name + title
  elif paddingPrefix:
    return title + ' ' + name
  else:
    return title + name

LANG_SUFFIX_TR = {
  ('ja', 'en'): ((u"の", u"'s"),),
  ('ja', 'ko'): ((u"の", u"의"),),
  #('ja', 'zh'): ((u"の", u"的"),),

  ('en', 'zh'): ((u"'s", u"的"),),
  ('en', 'ko'): ((u"'s", u"의"),),
  ('en', 'ja'): ((u"'s", u"の"),),

  ('zh', 'en'): ((u"的", u"'s"),),
  ('zh', 'ko'): ((u"的", u"'의"),),
  ('zh', 'ja'): ((u"的", u"'の"),),
}
def _get_lang_suffices(to, fr):
  """
  @param  to  str
  @param  fr  str
  @return  [(unicode pattern, unicode replacement)] or None
  """
  # Disabled for Chinese as it might use lots of memory and
  # it will break のほう
  #if lang.startswith('zh'):
  #  return LANG_SUFFIX_TR['zh']
  if config.is_latin_language(to):
    to = 'en'
  return LANG_SUFFIX_TR.get((fr[:2], to[:2]))

S_PUNCT = u"、？！。…「」『』【】" # full-width punctuations
def _partition_punct(text, punct=S_PUNCT):
  """
  @param  text  unicode
  @return  (unicode left, unicode middle, unicode right) not None
  """
  left = right = ''
  count = skstr.countright(text, punct)
  if count:
    right = text[-count:]
    text = text[:-count]
  count = skstr.countleft(text, punct)
  if count:
    left = text[:count]
    text = text[count:]
  return left, text, right

def __lang_level(lang):
  """Larger applied first
  @param  lang  str
  @return  int
  """
  if not lang or lang == 'ja':
    return 0
  if lang == 'en':
    return 1
  if config.is_latin_language(lang):
    return 2
  return 3
def _lang_sort_key(t, s):
  """Larger applied first
  @param  t  str  target language
  @param  s  str  source language
  @return  int
  """
  return __lang_level(t) * 10 + __lang_level(s)

def _td_sort_key(td):
  """Larger applied first, true is applied first
  @param  td  _Term
  @return  tuple
  """
  return (len(td.pattern), td.private, td.special, not td.icase, _lang_sort_key(td.language, td.sourceLanguage), td.id) #, it.regex)

#_re_marks = re.compile(r'<[0-9a-zA-Z: "/:=-]+?>')
#def _remove_marks(text): #return _re_marks.sub('', text) # unicode -> unicode

def _host_category(host): # str -> int
  if host:
    if host == 'lecol':
      host = 'lec'
    elif host == 'excite':
      host = 'jbeijing'
    try: return 1 << dataman.Term.HOSTS.index(host)
    except: pass
  return 0

def _host_categories(host): # str -> int
  if not host:
    return 0
  sep = ','
  if sep not in host:
    return _host_category(host)
  ret = 0
  l = host.split(sep)
  for i,h in enumerate(dataman.Term.HOSTS):
    if h in l:
      ret |= 1 << i
  return ret

class TermTitle(object):
  __slots__ = 'prefix', 'pattern', 'text', 'regex', 'phrase', 'icase', 'sortKey'
  def __init__(self, sortKey, pattern='', text='', prefix=False, regex=False, phrase=False, icase=False):
    self.pattern = pattern # unicode
    self.text = text # unicode
    self.prefix = prefix # bool
    self.regex = regex # bool
    self.phrase = phrase # bool
    self.icase = icase # bool
    self.sortKey = sortKey # int

#class TermTranslator(rbmt.MachineTranslator):
#  def __init__(self, cabocha, language, underline=True):
#    #escape = ESCAPE_ALL or config.is_kanji_language(language)
#    sep = '' if language.startswith('zh') else ' '
#    super(TermTranslator, self).__init__(cabocha, language,
#        sep=sep,
#        escape=True,
#        underline=underline and escape)
#
#  def translate(self, text, tr=None, fr="", to=""):
#    return super(TermTranslator, self).translate(text, tr=tr)

# EOF
