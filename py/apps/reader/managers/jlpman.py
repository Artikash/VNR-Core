# coding: utf8
# jlpman.py
# 3/28/2014 jichi

from sakurakit.skclass import memoized

@memoized
def manager(): return JlpManager()

class JlpManager:

  def __init__(self):
    self._parserType = 'mecab' # 'mecab' or 'cabocha'

  def isEnabled(self): return bool(self._parserType) # -> bool

  def setParserType(self, v): self._parserType = v # str ->

  def parseToRuby(self, text):
    """
    @param  text  unicode
    @return  [unicode] or None
    """
    from mecabjlp import mecabrender
    return list(mecabrender.parseparagraph(text, self._parse))

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
          type=True, reading=True, feature=True,
          fmt=m.meCabFormat(), ruby=m.rubyType,
          tagger=m.meCabTagger(),
          **kwargs)
    if self._parserType == 'cabocha':
      from cabochajlp import cabochaparse
      import cabochaman
      m = cabochaman.manager()
      return cabochaparse.parse(text,
          type=True, reading=True, feature=True,
          fmt=m.meCabFormat(), ruby=m.rubyType,
          parser=m.caboChaParser(),
          **kwargs)

# EOF
