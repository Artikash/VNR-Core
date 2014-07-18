# coding: utf8
# jlpman.py
# 3/28/2014 jichi

from sakurakit.skclass import memoized
from mecabjlp import mecabdef
import defs

@memoized
def manager(): return JlpManager()

# Convert reader ruby to mecabdefs ruby

_MECABRUBY = {
  defs.FURI_KATA: mecabdef.RB_KATA,
  defs.FURI_HIRA: mecabdef.RB_HIRA,
  defs.FURI_ROMAJI: mecabdef.RB_ROMAJI,
  defs.FURI_KANJI: mecabdef.RB_KANJI,
  defs.FURI_HANGUL: mecabdef.RB_HANGUL,
  defs.FURI_THAI: mecabdef.RB_THAI,
  defs.FURI_TR: mecabdef.RB_TR,
}
def _tomecabruby(t): # str -> str
  return _MECABRUBY.get(t) or mecabdef.RB_HIRA

class JlpManager:

  def __init__(self):
    self._parserType = 'mecab' # 'mecab' or 'cabocha'

  def isEnabled(self): return bool(self._parserType) # -> bool

  def setParserType(self, v):
    pass # Always use 'mecab'
    #self._parserType = v # str ->

  def parseToRuby(self, text):
    """
    @param  text  unicode
    @return  [unicode] or None
    """
    from mecabjlp import mecabrender
    import mecabman
    m = mecabman.manager()
    return list(mecabrender.parseparagraph(text,
      parse=self._parse,
      type=True, reading=True, feature=True,
      fmt=m.meCabFormat(),
      ruby=_tomecabruby(m.rubyType),
    ))

  def _parse(self, text, **kwargs):
    """
    @param  text  unicode
    @return  iter or None
    """
    if self._parserType == 'mecab':
      from mecabjlp import mecabparse
      import mecabman
      m = mecabman.manager()
      return mecabparse.parse(text,
          tagger=m.meCabTagger(),
          **kwargs)
    if self._parserType == 'cabocha':
      from cabochajlp import cabochaparse
      import cabochaman
      m = cabochaman.manager()
      return cabochaparse.parse(text,
          parser=m.caboChaParser(),
          **kwargs)

# EOF
