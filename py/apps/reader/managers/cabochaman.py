# coding: utf8
# cabochaman.py
# 6/14/2014 jichi

if __name__ == '__main__':
  import sys
  sys.path.append('..')
  import debug
  debug.initenv()

import os, re, weakref
from sakurakit.skclass import memoized
from sakurakit.skdebug import dprint
#from sakurakit.skthreads import SkMutexLocker
from cabochajlp import cabochadef
from mecabjlp import mecabfmt
import defs, rc
import _cabochaman

## Parser ##

_CP = _cabochaman.CaboChaParser()

parse = _CP.parse
#def parse(*args, **kwargs):
#  with SkMutexLocker(_CP.mutex):
#    return _CP.parse(*args, **kwargs)

def cabochaparser():
  """
  @return  CaboCha.Parser or None
  """
  return _CP.parser()

#def cabochaparse(*args, **kwargs):
#  """"
#  @param  text  unicode
#  @param* sync  bool
#  @return   CaboCha.Tree
#  """
#  with SkMutexLocker(_CP.mutex):
#    return cabochaparser().parse(*args, **kwargs)

def _iterrendertable(text, features=None, charPerLine=100, rubySize='10px', colorize=False, center=True, furiType=defs.FURI_HIRA, **kwargs):
  """
  @param  text  unicode
  @param* charPerLine  int  maximum number of characters per line
  @param* rubySize  str
  @param* colorsize  bool
  @param* center  bool
  @param* features  {unicode surface:(unicode feature, fmt)} or None
  @yield  unicode  HTML table
  """
  i = j = 0
  line = []
  lineCount = 0 # int  estimated line width, assume ruby has half width
  hasfeature = features is not None
  color = None
  #groupColor = '#001' if color else '#ffa' # black if color else yellow
  #groupColor = '#fefa4f' # yellow, the same as MacVim yellow
  #groupColor = '#ffa' # yellow
  #groupColor = '#fefa9f' # yellow
  groupColor = '#ff8' # yellow

  LATIN_YOMI_WIDTH = 0.33 # = 2/6
  KANJI_YOMI_WIDTH = 0.55 # = 1/2
  # yomi size / surface size
  yomiWidth = LATIN_YOMI_WIDTH if furiType in (defs.FURI_ROMAJI, defs.FURI_THAI, defs.FURI_TR) else KANJI_YOMI_WIDTH

  for it in parse(text, type=True, feature=hasfeature, reading=True, group=True, furiType=furiType, **kwargs):
    if hasfeature:
      surface, ch, yomi, f, fmt, group = it
    else:
      surface, ch, yomi, group = it
    if ch == cabochadef.TYPE_PUNCT: # disable underline for punctuation
      group = None
    if colorize:
      if ch == cabochadef.TYPE_KANJI:
        i += 1
        color = 'rgba(255,0,0,40)' if i % 2 else 'rgba(255,255,0,40)'   # red or yellow
      elif ch == cabochadef.TYPE_RUBY:
        j += 1
        #color = "rgba(0,255,0,40)" if j % 2 else "rgba(255,0,255,40)" # green or magenta
        color = "rgba(0,255,0,40)" if j % 2 else "rgba(0,0,255,40)" # green or blue
      else:
        color = None
    if hasfeature:
      features[surface] = f, fmt

    width = max(len(surface), len(yomi)*yomiWidth if yomi else 0)
    if width + lineCount <= charPerLine:
      pass
    elif line:
      yield rc.jinja_template('html/furigana').render({
        'tuples': line,
        'rubySize': rubySize,
        'center': center,
        'groupColor': groupColor,
      })
      line = []
      lineCount = 0
    line.append((surface, yomi, color, group))
    lineCount += width
  if line:
    yield rc.jinja_template('html/furigana').render({
      'tuples': line,
      'rubySize': rubySize,
      'center': center,
      'groupColor': groupColor,
    })

def rendertable(*args, **kwargs):
  return ''.join(_iterrendertable(*args, **kwargs))

## Manager ##

class CaboChaManager:

  def __init__(self):
    self.rubyType = '' # str
    self.dicName = '' # str

  def isEnabled(self):
    return _CP.enabled #and os.path.exists(_CP.dicdir)

  def setEnabled(self, t): # bool
    _CP.setenabled(t)

  def caboChaParser(self): return _CP.parser() # CaboCha.Parser
  def meCabFormat(self): return _CP.fmt # mecabfmt

  def setRubyType(self, v):
    """
    @param  name  str
    """
    self.rubyType = v

  def setDictionaryName(self, v):
    """
    @param  name  str
    """
    if self.dicName != v:
      dprint(v)
      self.dicName = v
      _CP.setdic(v)
      _CP.setfmt(mecabfmt.getfmt(v))
      #_CP.setuserdic('') # clear default user dic
      _CP.setrcfile(rc.cabocha_rc_path(v))

@memoized
def manager(): return CaboChaManager()

if __name__ == '__main__':
  rcfile = '/Users/jichi/stream/Library/Dictionaries/cabocharc/unidic.rc'
  _CP.setenabled(True)
  _CP.setrcfile(rcfile)
  #t = u"すもももももももものうち"
  #t = u"憎しみは憎しみしか生まない"
  #t = u"童話はほとんど墓场から始まるものさ"
  #t = u"Chun、ShinGura"
  #t = u"レミニ、センス"
  #t = u'桜小路ルナ'
  #t = u'【桜小路ルナ】'
  #t = u"太郎はこの本を二郎を見た女性に渡した。"
  t = u"「どれだけ必死に働こうとも、所詮、安月給の臨時教師ですけどね」"

  #for surf,ch,reading,feature,fmt,group in parse(t, type=True, reading=True, feature=True, group=True):
  #  print ch, surf, group

  print rendertable(t) #, colorize=True)

# EOF
