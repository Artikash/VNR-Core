# coding: utf8
# mecabparser.py
# 10/12/2012 jichi
#
# doc: http://mecab.googlecode.com/svn/trunk/mecab/doc/index.html
# doxygen: http://mecab.googlecode.com/svn/trunk/mecab/doc/doxygen/annotated.html

if __name__ == '__main__': # DEBUG
  import sys
  sys.path.append("..")

from itertools import imap
import MeCab
from jaconv import jaconv
from unitraits import jpchars, uniconv
import mecabdef, mecabformat, mecablex

# Helper functions

lambda_eigen = lambda x:x

_capitalizeromaji_fix = (
  #(' Da ', ' da ',
  (' De ', ' de '),
  (' Ha ', ' ha '),
  (' Na ', ' na '),
  (' No ', ' no '),
  (' Ni ', ' ni '),
  (' To ', ' to '),
  #(' O ', ' o '),
  (' Wo ', ' wo '),
  #(' Yo ', ' yo '),
)
def capitalizeromaji(text):
  """
  @param  text  unicode
  @return  unicode
  """
  text = text.title()
  for k,v in _capitalizeromaji_fix:
    text = text.replace(k, v)
  return text

def getkataconv(ruby):
  """
  @param  ruby  str
  @return  unicode -> unicode
  """
  return (
      uniconv.kata2hira if ruby == mecabdef.RB_HIRA else
      jaconv.kata2romaji if ruby == mecabdef.RB_ROMAJI else
      jaconv.kata2ru if ruby == mecabdef.RB_RU else
      jaconv.kata2ko if ruby == mecabdef.RB_KO else
      jaconv.kata2th if ruby == mecabdef.RB_TH else
      jaconv.kata2ar if ruby == mecabdef.RB_AR else
      #lambda_eigen if ruby == mecabdef.RB_KATA else
      lambda_eigen)

# Classes

class _MeCabParser:
  formatter = mecabformat.UNIDIC_FORMATTER

  def __init__(self, tagger):
    self._tagger = tagger

  @property
  def tagger(self):
    if not self._tagger:
      import mecabtagger
      self._tagger = mecabtagger.MeCabTagger()
    return self._tagger

  def iterparseToKata(self, text):
    """
    @param  text  unicode
    @yield  unicode  ruby
    """
    for surface,feature in self.tagger.iterparse(text):
      kata = self.formatter.getkata(feature)
      if kata:
        yield kata

  UNKNOWN_RUBY = '?'
  def iterparseToRuby(self, text, kataconv, show_ruby_kana):
    """
    @param  text  unicode
    @param  kataconv  unicode -> unicode
    @param  show_ruby_kana  bool
    @yield  (unicode surface, unicode ruby or None, unicode feature)
    """
    for surface, feature in self.tagger.iterparse(text):
      ruby = None
      type = mecablex.getsurfacetype(surface)
      show_ruby = type == mecablex.SURFACE_KANJI or show_ruby_kana and type == mecablex.SURFACE_KANA
      if show_ruby:
        kata = self.formatter.getkata(feature)
        if kata and kata != ruby:
          ruby = kataconv(kata)
          if ruby == surface:
            ruby = None
        if not ruby and type == mecablex.SURFACE_KANJI:
          ruby = self.UNKNOWN_RUBY
      yield surface, ruby, feature

class MeCabParser(object):
  def __init__(self, tagger=None):
    self.__d = _MeCabParser(tagger)

  def tagger(self): return self.__d.tagger # -> MeCabTagger
  def setTagger(self, v): self.__d.tagger = v

  def iterparseToRuby(self, text, ruby=mecabdef.RB_HIRA, show_ruby_kana=False):
    """
    @param  text  unicode
    @param  ruby  str  type
    @param* show_ruby_kana  bool  show ruby for kana as well  by default, only for kanji
    @yield  (unicode surface, unicode ruby, unicode surface)
    """
    conv = getkataconv(ruby)
    return self.__d.iterparseToRuby(text, conv, show_ruby_kana=show_ruby_kana)

  def toRuby(self, text, ruby=mecabdef.RB_HIRA, sep=''):
    """
    @param  text  unicode
    @param* ruby  unicode
    @param* sep  unicode
    @return  unicode
    """
    conv = getkataconv(ruby)
    if ruby in (mecabdef.RB_ROMAJI, mecabdef.RB_RU):
      wide2thin = uniconv.wide2thin
      f = conv
      conv = lambda x: wide2thin(f(x))
    return sep.join(imap(conv,  self.__d.iterparseToKata(text)))

  def toRomaji(self, text, capitalize=True):
    """
    @param  text  unicode
    @param* space  bool
    @return  unicode  plain text
    """
    ret = self.toRuby(text, sep=' ', ruby=mecabdef.RB_ROMAJI)
    if capitalize:
      ret = capitalizeromaji(ret)
    return ret

if __name__ == '__main__':
  dicdir = '/opt/local/lib/mecab/dic/ipadic-utf8'
  dicdir = '/Users/jichi/opt/Visual Novel Reader/Library/Dictionaries/ipadic'
  dicdir = '/Users/jichi/src/unidic'
  dicdir = '/opt/local/lib/mecab/dic/naist-jdic-utf8'

  #rcfile = '/Users/jichi/stream/Library/Dictionaries/mecabrc/ipadic.rc'
  rcfile = '/Users/jichi/stream/Library/Dictionaries/mecabrc/unidic.rc'

  import mecabtagger
  mecabtagger.setenvrc(rcfile)

  mp = MeCabParser()

  t = u"可愛いよ"
  t = u'今日はいい天気ですね'
  t = u'すもももももももものうち'
  t = u'しようぜ'
  t = u'思ってる'
  t = u'巨乳'
  print mp.toRuby(t)
  print mp.toRomaji(t)

  for it in mp.iterparseToRuby(t):
    print it[0], it[1], it[2]

# EOF

#def tolou(self, text, termEnabled=False, ruby=mecabdef.RB_TR):
#  """
#  @param  text  unicode
#  @param* termEnabled  bool  whether query terms
#  @param* type  bool  whether return type
#  @param* ruby  unicode
#  @return  unicode
#  """
#  # Add space between words
#  return ' '.join(furigana or surface for surface,furigana in
#      self.parse(text, termEnabled=termEnabled, reading=True, lougo=True, ruby=ruby))

