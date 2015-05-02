# coding: utf8
# _termman.py
# 10/8/2012 jichi
#
# Terminology:
# - data: raw game text byte code
# - text: original game text
# - translation: machine translation
# - comment: user's subtitle or comment

#from sakurakit.skprof import SkProfiler

import os, string, re
from itertools import imap
#from collections import OrderedDict
from sakurakit import skfileio, skstr
from sakurakit.skdebug import dprint, dwarn
from opencc import opencc
from unitraits import jpchars, jpmacros
from convutil import toalphabet, kana2name, zhs2zht, zht2zhs, \
                     ja2zh_name_test, ja2zhs_name, ja2zht_name, ja2zht_name_fix
import config, dataman, defs, i18n

## Translation proxy

class TranslationProxy(object):
  __slots__ = (
    'id',
    'category',
    'input',
    'output',
    'role',
  )
  def __init__(self, td):
    """
    @param  td  _Term
    """
    self.id = td.id
    self.role = td.role
    self.input = _unescape_term_text(td.pattern)
    self.output = _unescape_term_text(td.text)
    self.category = host_categories(td.host)

## Helper functions used by termman

# http://sakuradite.com/wiki/en/Machine_Translators
def host_category(host):
  """
  @param  host  str  single host
  @return  int  category flag mark
  """
  if host:
    if host == 'lecol':
      host = 'lec'
    elif host == 'excite':
      host = 'jbeijing'
    elif host == 'nifty':
      host = 'atlas'
    try: return 1 << dataman.Term.HOSTS.index(host)
    except: pass
  return -1

def host_categories(host): # str -> int
  """
  @param  host  str  host separated by ','
  @return  int  category flag mark
  """
  if not host:
    return -1
  sep = ','
  if sep not in host:
    return host_category(host)
  ret = 0
  for h in host.split(sep):
    h = host_category(h)
    if h > 0:
      ret |= h
  return ret or -1 # if no match, apply to all hosts

def _lang_level(lang):
  """Larger applied first
  @param  lang  str
  @return  int
  """
  if not lang or lang == 'ja':
    return 0
  if lang == 'en':
    return 1
  if lang == 'ru':
    return 2
  if lang == 'el':
    return 5
  if config.is_cyrillic_language(lang):
    return 4
  if config.is_latin_language(lang):
    return 3
  return 6
def _lang_sort_key(t, s):
  """Larger applied first
  @param  t  str  target language
  @param  s  str  source language
  @return  int
  """
  return _lang_level(t) * 20 + _lang_level(s)

def _role_priority(role, type, has_symbol):
  """Larger applied first
  @param  role  str
  @param  type  str
  @param  has_symbol  bool
  @return  int
  """
  if type == 'suffix':
    return 1
  if type == 'prefix':
    return 2
  if role == defs.TERM_NAME_ROLE: # apply name rules first
    if type == 'yomi': # apply yomi after individual name definition
      return 3
    else:
      return 4
  if role and role != defs.TERM_PHRASE_ROLE: # apply user-defined symbols at last
    return -1
  # Zero is the base line for phrase
  # When no symbol, X has the higher priority (5)
  # When there is symbol, M has the higher priority
  return 0 if has_symbol else 5

def _td_sort_key(td):
  """Sort term data reversely. Larger applied first, true is applied first
  @param  td  _Term
  @return  tuple
  """
  role = td.role or _td_default_role(td)
  has_symbol = _contains_syntax_symbol(td.pattern) or td.type in ('prefix', 'suffix')
  role_priority = _role_priority(role, td.type, has_symbol)
  lang_priority = _lang_sort_key(td.language, td.sourceLanguage)
  return (not has_symbol, role_priority, len(td.pattern), td.private, td.special, not td.icase, lang_priority, td.id) #, it.regex)

def sort_terms(termdata):
  """
  @param  termdata  [_Term]
  """
  termdata.sort(reverse=True, key=_td_sort_key)

def _td_default_role(td): # string -> string or None
  if td.type in ('name', 'prefix', 'suffix', 'yomi'):
    return defs.TERM_NAME_ROLE
  if td.type in ('trans'):
    return defs.TERM_PHRASE_ROLE

## Local helper functions

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

def _contains_syntax_symbol(text):
  """Return if text contains things such as [[x#1]]. In accurate test.
  @param  text  unicode
  @return  bool
  """
  return bool(text) and '[[' in text and ']]' in text

class TermWriter:

  RE_MACRO = re.compile('{{(.+?)}}')

  def __init__(self, createTime, termData, gameIds, hentai, parent):
    self.createTime = createTime # float
    self.termData = termData # [_Term]
    self.gameIds = gameIds # set(ing gameId)
    self.hentai = hentai # bool
    self.parent = parent # _TermManager.instance

  def isOutdated(self): # -> bool
    return self.createTime < self.parent.updateTime

  def saveTerms(self, path, type, to, fr, macros):
    """This method is invoked from a different thread
    @param  path  unicode
    @param  type  str  term type
    @param  to  str  target text language
    @param  fr  str  source text language
    @param  macros  {unicode pattern:unicode repl}
    @return  bool
    """
    #marksChanges = self.marked and type in ('output', 'trans_output')
    convertsSimplifiedChinese = to == 'zhs' and type in ('output', 'trans')
    convertsTraditionalChinese = to == 'zht' and type in ('output', 'trans')
    #if type not in ('input', 'trans_input', 'trans_output'):
    #  titles = None

    trans_type = type == 'trans'

    fr2 = fr[:2]
    to2 = to[:2]

    frKanjiLanguage = config.is_kanji_language(fr)
    frSpaceLanguage = config.language_word_has_space(fr)

    toKanjiLanguage = config.is_kanji_language(to)
    toSpaceLanguage = config.language_word_has_space(to)

    empty = True

    #padding = trans_input or toLatinLanguage and td.type in ('trans', 'name', 'yomi')

    count = len(self.termData)
    try:
      with open(path, 'w') as f:
        f.write(self._renderHeader(type, to, fr))
        for td in self.iterTermData(type, to, fr):
          if self.isOutdated():
            raise Exception("cancel saving out-of-date terms")
          zs = convertsSimplifiedChinese and td.language == 'zht'
          zt = convertsTraditionalChinese and td.language == 'zhs'
          # no padding space for Chinese names

          regex = td.regex

          role = td.role or _td_default_role(td)

          pattern = _unescape_term_text(td.pattern)
          pattern = self._applyMacros(pattern, macros)
          if zs:
            pattern = zht2zhs(pattern)
          elif zt:
            pattern = zhs2zht(pattern)
            #if role == defs.TERM_NAME_ROLE:
            #  pattern = jazh.ja2zht_name_fix(pattern)

          if trans_type and role == defs.TERM_NAME_ROLE and fr2 == 'zh':
            if fr == 'zhs':
              pattern = opencc.ja2zhs(pattern)
            elif fr == 'zht':
              pattern = opencc.ja2zht(pattern)

          if td.type == 'yomi' and to2 == 'zh':
            repl = ja2zhs_name(pattern) if to == 'zhs' else ja2zht_name(pattern)
          else:
            repl = _unescape_term_text(td.text)
            repl = self._applyMacros(repl, macros)
            if zs:
              repl = zht2zhs(repl)
            elif zt:
              repl = zhs2zht(repl)
              if role == defs.TERM_NAME_ROLE:
                repl = ja2zht_name_fix(repl)
            if td.type == 'yomi':
              repl = kana2name(repl, to) or repl
            elif td.type == 'name' and td.language != to and to != 'el': # temporarily skip Greek
              repl = toalphabet(repl, to, td.language) #fr=td.language

          if td.phrase:
            left = pattern[0]
            right = pattern[-1]
            if not regex:
              regex = True
              pattern = re.escape(pattern)
            pattern = _phrase_lbound(left, fr) + pattern + _phrase_rbound(right, fr)

          if trans_type:
            if td.type == 'suffix':
              if not _contains_syntax_symbol(pattern):
                pattern = "[[%s]]%s" % (defs.TERM_NAME_ROLE, pattern)
              if not _contains_syntax_symbol(repl):
                repl = "[[]]%s" % repl
            elif td.type == 'prefix':
              if not _contains_syntax_symbol(pattern):
                if frSpaceLanguage:
                  pattern = "%s [[%s]]" % (pattern, defs.TERM_NAME_ROLE)
                else:
                  pattern = "%s[[%s]]" % (pattern, defs.TERM_NAME_ROLE)
              if not _contains_syntax_symbol(repl):
                if toSpaceLanguage:
                  repl = "%s [[]]" % repl
                else:
                  repl = "%s[[]]" % repl

          if trans_type:
            self._writeCodecLine(f, td.id, pattern, repl, regex, td.icase, td.host, role)
          else:
            self._writeTransformLine(f, td.id, pattern, repl, regex, td.icase, td.host)

          empty = False

      if not empty:
        return True

    except Exception, e:
      dwarn(e)

    skfileio.removefile(path) # Remove file when failed
    return False

  @staticmethod
  def _writeTransformLine(f, tid, pattern, repl, regex, icase, host):
    """
    @param  f  file
    @param  tid  long
    @param  pattern  unicode
    @param  repl  unicode
    @param  regex  bool
    @param  icase  bool
    @param  host  str
    @return  unicode or None
    """
    if '\n' in pattern or '\n' in repl or '\t' in pattern or '\t' in repl:
      dwarn("skip tab or new line in term: id = %s" % tid)
      return
    cat = host_categories(host)
    cols = [str(tid), str(cat), pattern]
    if repl:
      cols.append(repl)
    line = '\t'.join(cols)
    line = "\t%s\n" % line # add leading/trailing spaces
    if icase:
      line = 'i' + line
    if regex:
      line = 'r' + line
    f.write(line)

  @staticmethod
  def _writeCodecLine(f, tid, pattern, repl, regex, icase, host, role):
    """
    @param  f  file
    @param  tid  long
    @param  pattern  unicode
    @param  repl  unicode
    @param  regex  bool
    @param  icase  bool
    @param  host  str
    @param  role  str
    @return  unicode or None
    """
    if '\n' in pattern or '\n' in repl:
      dwarn("skip new line in term: id = %s" % tid)
      return
    cat = host_categories(host)
    features = [str(tid), str(cat)]
    flags = ''
    if icase:
      flags += 'i'
    if regex:
      flags += 'r'
    if flags:
      features.append(flags)

    feature = ' '.join(features)

    if not repl:
      role = '' # disable role if need to delete replacement
    cols = [role, pattern, repl, feature]
    line = ' ||| '.join(cols) + '\n'
    f.write(line)

  def _renderHeader(self, type, to, fr):
    """
    @param  type  str
    @param  to  str
    @param  fr  str
    @return  unicode
    """
    return """\
# This file is automatically generated for debugging purposes.
# Modifying this file will NOT affect VNR.
#
# Unix time: %s
# Options: type = %s, to = %s, fr = %s, hentai = %s, files = (%s)
#
""" % (self.createTime, type, to, fr, self.hentai,
    ','.join(imap(str, self.gameIds)) if self.gameIds else 'empty')

  def iterTermData(self, type, to, fr):
    """
    @param  type  str
    @param  to  str
    @param  fr  str
    @yield  _Term
    """
    if type == 'trans':
      types = 'trans', 'name', 'yomi', 'suffix', 'prefix'
      #if not to.startswith('zh'):
      #  types.append('yomi')
    else:
      types = type,

    # Types do not apply to non-Japanese languages
    jatypes = 'macro', 'game', 'ocr' # Japanese types applied to all languages
    zhtypes = 'name', 'yomi' # Japanese types applied to all Chinese languages
    fr2 = fr[:2]
    to2 = to[:2]
    fr_is_latin = config.is_latin_language(fr)
    fr_is_cyrillic = config.is_cyrillic_language(fr)
    items = set() # skip duplicate names
    types = frozenset(types)
    for td in self.termData:
      if (#not td.disabled and not td.deleted and td.pattern # in case pattern is deleted
          td.type in types
          and (not td.hentai or self.hentai)
          and i18n.language_compatible_to(td.language, to)
          and (not td.special or self.gameIds and td.gameId and td.gameId in self.gameIds)
          and not (td.type == 'yomi' and to2 == 'zh' and not ja2zh_name_test(td.pattern)) # skip yomi for Chinese if not pass name test
          and (td.sourceLanguage.startswith(fr2)
            #or fr != 'ru' and fr_is_cyrillic and td.sourceLanguage == 'ru'
            or fr != 'en' and fr_is_latin and td.sourceLanguage == 'en'
            or td.sourceLanguage == 'ja' and (
              td.type in jatypes
              or fr2 == 'zh' and td.type in zhtypes
            )
          )
        ) and td.pattern not in items:
          items.add(td.pattern)
          yield td

  def queryProxies(self, to, fr):
    """
    @param  to  str
    @param  fr  str
    @return  [TranslationProxy] not None
    """
    to2 = to[:2]
    return [TranslationProxy(td)
        for td in self.iterTermData('proxy', to, fr)
        if td.pattern and td.text and td.role] #and td.language[:2] == to2] # force language

  def queryMacros(self, to, fr):
    """
    @param  to  str
    @param  fr  str
    @return  {unicode pattern:unicode repl} not None
    """
    ret = {_unescape_term_text(td.pattern):_unescape_term_text(td.text) for td in self.iterTermData('macro', to, fr)}
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

  def _applyMacros(self, text, macros):
    """
    @param  text  unicode
    @param  macros  {unicode pattern:unicode repl}
    @return  unicode
    """
    if not text:
      return ''
    for m in self.RE_MACRO.finditer(text):
      macro = m.group(1)
      repl = macros.get(macro)
      if repl is None:
        dwarn("missing macro", macro)
      else:
        text = text.replace("{{%s}}" % macro, repl)
    return text

# EOF

# S_PUNCT = u"、？！。…「」『』【】" # full-width punctuations
# def _partition_punct(text, punct=S_PUNCT):
#   """
#   @param  text  unicode
#   @return  (unicode left, unicode middle, unicode right) not None
#   """
#   left = right = ''
#   count = skstr.countright(text, punct)
#   if count:
#     right = text[-count:]
#     text = text[:-count]
#   count = skstr.countleft(text, punct)
#   if count:
#     left = text[:count]
#     text = text[count:]
#   return left, text, right

#          if (toLatinLanguage and (trans_input or trans_output)
#              and repl and (repl[0] in S_PUNCT or repl[-1] in S_PUNCT)):
#            if trans_output:
#              repl = repl.strip(S_PUNCT)
#            elif trans_input:
#              repl_left, repl, repl_right = _partition_punct(repl)
