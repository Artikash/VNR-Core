# coding: utf8
# termman.py
# 10/8/2012 jichi
#
# Terminology:
# - data: raw game text byte code
# - text: original game text
# - translation: machine translation
# - comment: user's subtitle or comment

from functools import partial
from zhszht.zhszht import zhs2zht
from sakurakit import skthreads
from sakurakit.skclass import memoized, staticproperty
from sakurakit.skdebug import dprint, dwarn
import config, dataman, defs, i18n

class _TermManager:

  def __init__(self):
    #self.convertsChinese = False
    self.enabled = True # bool
    self.locked = False  # bool
    self.hentai = False # bool
    self.language = 'en' # str

  #@classmethod
  #def needsEscape(cls):
  #  return config.is_asian_language(cls.language)

  def applyTerms(self, terms, text, language, convertsChinese=False):
    """
    @param  terms  iterable dataman.Term
    @param  text  unicode
    @param  language  unicode
    @param* convertsChinese  bool
    @return  unicode
    """
    if not self.enabled:
      return text
    dm = dataman.manager()
    hasTitles = dm.hasTermTitles() # cached
    zht = language == 'zht' # cached
    for term in terms:
      td = term.d # To improve performance
      if (not td.hentai or self.hentai) and td.pattern and i18n.language_compatible_to(td.language, language):
        if hasTitles and term.needsReplace():
          try: text = term.replace(text)
          except Exception, e: dwarn(td.pattern, td.text, e)
        else:
          z = convertsChinese and zht and td.language == 'zhs'
          #repl = term.bbcodeText if term.bbcode else term.text
          repl = td.text
          if z and repl: # and self.convertsChinese:
            repl = zhs2zht(repl)
            #elif config.is_latin_language(td.language):
            #  repl += " "
          if not term.patternNeedsRe():
            pattern = zhs2zht(td.pattern) if z else td.pattern
            text = text.replace(pattern, repl)
          else:
            try: text = term.patternRe.sub(repl, text)
            except Exception, e: dwarn(td.pattern, td.text, e)
        if not text: # well, the text is deleted by terms
          break
    return text

  def iterTerms(self, terms, language):
    """
    @param  terms  iterable dataman.Term
    @param  language  unicode
    @yield  Term
    """
    if not self.enabled:
      return
    for term in terms:
      td = term.d # To improve performance
      if (not td.hentai or self.hentai) and td.pattern and i18n.language_compatible_to(td.language, language):
        yield term

  def warmup(self, terms, language='', hasTitles=False, hentai=False): # [dataman.Term], str, bool ->
    dprint("enter")
    needsEscape = config.is_asian_language(language)
    for term in terms:
      td = term.d # To improve performance
      if not td.disabled and not td.special and (not td.hentai or hentai) and td.pattern and i18n.language_compatible_to(td.language, language):
        if hasTitles and term.needsReplace():
          try:
            if needsEscape:
              term.prepareReplace
              term.applyReplace
            else:
              term.replace
          except Exception, e: dwarn(td.pattern, td.text, e)
        elif td.pattern and term.patternNeedsRe():
          try: term.patternRe
          except Exception, e: dwarn(td.pattern, td.text, e)

    import trman
    trman.manager().clearCacheRequested.emit()

    self.locked = False
    dprint("leave")

class TermManager:

  ## Construction ##

  def __init__(self): self.__d = _TermManager()

  ## Properties ##

  def isLocked(self): return self.__d.locked

  def language(self): return self.__d.language
  def setLanguage(self, value): self.__d.language = value

  def isEnabled(self): return self.__d.enabled
  def setEnabled(self, value): self.__d.enabled = value

  def isHentai(self): return self.__d.hentai
  def setHentai(self, value):
    dprint(value)
    self.__d.hentai = value

  #def convertsChinese(self): return self.__d.convertsChinese
  #def setConvertsChinese(self, value): self.__d.convertsChinese = value

  ## Warm up ##

  def warmup(self, async=True, interval=0): # bool, int
    d = self.__d
    if not d.enabled or d.locked:
      return
    dprint("enter")
    dm = dataman.manager()

    task = partial(d.warmup,
        terms=dm.terms(),
        hasTitles=dm.hasTermTitles(),
        hentai=d.hentai,
        language=d.language)

    if not async:
      apply(task)
    else:
      d.locked = True
      if interval:
        skthreads.runasynclater(task, interval)
      else:
        skthreads.runasync(task)
    dprint("leave")

  ## Queries ##

  def queryTermTitles(self, language):
    """
    @param  language  unicode
    @return  {unicode from:unicode to} not None not empty
    """
    d = self.__d
    ret = {'':''}
    if not d.enabled or d.locked:
      return ret
    zht = language == 'zht'
    q = self.__d.iterTerms(
        dataman.manager().iterTitleTerms(),
        language)
    for t in q:
      td = t.d
      if not td.hentai or d.hentai:
        pat = td.pattern
        repl = td.text
        if zht and td.language == 'zhs':
          pat = zhs2zht(pat)
          if repl: # and self.convertsChinese:
            repl = zhs2zht(repl)
        ret[pat] = repl
    return ret

  def applyTargetTerms(self, text, language):
    """
    @param  text  unicode
    @param  language  unicode
    @return  unicode
    """
    return self.__d.applyTerms(dataman.manager().iterTargetTerms(),
        text, language, convertsChinese=True)

  def applyOriginTerms(self, text, language):
    """
    @param  text  unicode
    @param  language  unicode
    @return  unicode
    """
    return self.__d.applyTerms(dataman.manager().iterOriginTerms(), text, language)

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
    if not language:
      language = self.__d.language
    return self.__d.applyTerms(dataman.manager().iterSpeechTerms(), text, language)

  def applySourceTerms(self, text, language):
    """
    @param  text  unicode
    @param  language  unicode
    @return  unicode
    """
    dm = dataman.manager()
    d = self.__d
    text = d.applyTerms(dm.iterSourceTerms(), text, language)
    if text and dm.hasNameItems() and config.is_latin_language(d.language):
      try:
        for name in dm.iterNameItems():
          if name.translation:
            text = name.replace(text)
      except Exception, e: dwarn(e)
      text = text.rstrip() # remove trailing spaces
    return text

  # Escaped

  def prepareEscapeTerms(self, text, language):
    """
    @param  text  unicode
    @param  language  unicode
    @return  unicode
    """
    d = self.__d
    if not d.enabled or d.locked:
      return text
    dm = dataman.manager()
    hasTitles = dm.hasTermTitles() # cached
    esc = defs.TERM_ESCAPE + ' '
    for term in dm.iterEscapeTerms():
      td = term.d # To improve performance
      if (not td.hentai or d.hentai) and td.pattern and i18n.language_compatible_to(td.language, language):
        if hasTitles and term.needsReplace():
          try: text = term.prepareReplace(text)
          except Exception, e: dwarn(td.pattern, td.text, e)
        else:
          h = term.priority or td.id or id(term)
          key = esc % h
          if not term.patternNeedsRe():
            text = text.replace(td.pattern, key)
          else:
            try: text = term.patternRe.sub(key, text)
            except Exception, e: dwarn(td.pattern, td.text, e)
        if not text:
          break
    if text and dm.hasNameItems() and config.is_asian_language(d.language):
      try:
        for name in dm.iterNameItems():
          if name.translation:
            text = name.prepareReplace(text)
      except Exception, e: dwarn(e)
    return text

  def applyEscapeTerms(self, text, language):
    """
    @param  text  unicode
    @param  language  unicode
    @return  unicode
    """
    d = self.__d
    if not d.enabled or d.locked:
      return text
    dm = dataman.manager()
    hasTitles = dm.hasTermTitles() # cached
    esc = defs.TERM_ESCAPE
    zht = language == 'zht' # cached
    for term in dm.iterEscapeTerms():
      td = term.d # To improve performance
      if (not td.hentai or d.hentai) and td.pattern and i18n.language_compatible_to(td.language, language):
        if hasTitles and term.needsReplace():
          try: text = term.applyReplace(text)
          except Exception, e: dwarn(td.pattern, td.text, e)
        else:
          #repl = term.bbcodeText if term.bbcode else term.text
          repl = td.text
          if repl and zht and td.language == 'zhs':
            repl = zhs2zht(repl)
          #elif config.is_latin_language(td.language):
          #  repl += " "
          h = term.priority or td.id or id(term)
          key = esc % h
          text = text.replace(key, repl)
        if not text:
          break
    if text and dm.hasNameItems() and config.is_asian_language(d.language):
      try:
        for name in dm.iterNameItems():
          if name.translation:
            text = name.applyReplace(text)
      except Exception, e: dwarn(e)
    return text

  ## MeCab ##

  def applyWordTerms(self, text):
    """
    @param  text  unicode
    @return  unicode
    """
    # This feature is disabled
    # Compiled MeCab dictionary id preferred
    return text

    #d = self.__d
    #if not d.enabled or d.locked:
    #  return text
    #dm = dataman.manager()
    #for term in dm.iterWordTerms():
    #  td = term.d
    #  if (not td.hentai or d.hentai) and not td.regex: #and td.language == 'ja': # skip using regular expressions, ignore languages
    #    text = text.replace(td.pattern, " %s " % td.pattern)
    #if dm.hasNameItems():
    #  for name in dm.iterNameItems():
    #    text = text.replace(name.text, " %s " % name.text)
    #return text.strip()

  def queryLatinWordTerms(self, text):
    """
    @param  text  unicode
    @return  unicode or None
    """
    d = self.__d
    if not d.enabled or d.locked:
      return text
    dm = dataman.manager()
    for term in dm.iterLatinSourceTerms():
      td = term.d
      if (not td.hentai or d.hentai) and td.pattern and not td.regex and td.language == 'en':
        if text == td.pattern and td.text:
          return td.text.capitalize() if td.type == 'name' else td.text

  def queryFuriTerms(self, text):
    """
    @param  text  unicode
    @return  unicode or None
    """
    d = self.__d
    if not d.enabled or d.locked:
      return text
    dm = dataman.manager()
    for term in dm.iterFuriTerms():
      td = term.d
      if (not td.hentai or d.hentai) and td.pattern and not td.regex and td.language == 'ja': # skip using regular expressions
        if text == td.pattern and td.text:
          return td.text
    if dm.hasNameItems():
      for name in dm.iterNameItems():
        if text == name.text:
          return name.yomi or text

@memoized
def manager(): return TermManager()

# EOF
