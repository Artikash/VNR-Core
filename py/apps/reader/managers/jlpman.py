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
    @return  unicode
    """
    return text

# EOF
