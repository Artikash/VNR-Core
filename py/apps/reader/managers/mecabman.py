# coding: utf8
# mecabman.py
# 10/12/2012 jichi
#
# build: http://www.freia.jp/taka/blog/759/index.html
# usage: http://d.hatena.ne.jp/fgshun/20090910/1252571625
# feature: http://mecab.googlecode.com/svn/trunk/mecab/doc/index.html?source=navbar
#     表層形\t品詞,品詞細分類1,品詞細分類2,品詞細分類3,活用形,活用型,原形,読み,発音
#
#     すもももももももものうち
#     すもも  名詞,一般,*,*,*,*,すもも,スモモ,スモモ
#     も      助詞,係助詞,*,*,*,*,も,モ,モ
#     もも    名詞,一般,*,*,*,*,もも,モモ,モモ
#     も      助詞,係助詞,*,*,*,*,も,モ,モ
#     もも    名詞,一般,*,*,*,*,もも,モモ,モモ
#     の      助詞,連体化,*,*,*,*,の,ノ,ノ
#     うち    名詞,非自立,副詞可能,*,*,*,うち,ウチ,ウチ
#     EOS

if __name__ == '__main__':
  import sys
  sys.path.append('..')
  import debug
  debug.initenv()

import os, re, weakref
#from itertools import ifilter
from PySide.QtCore import Signal, Slot, Property, Qt, QObject
from PySide.QtGui import QColor, QTextCharFormat
from Qt5.QtWidgets import QSyntaxHighlighter
from sakurakit import skstr
#from sakurakit.skqml import QmlObject
from sakurakit.skclass import Q_Q, memoized
from sakurakit.skdebug import dprint
from cconv import cconv
from mecabjlp import mecabdef, mecabfmt
import defs, rc
import _mecabman

_rx_spaces = re.compile(r'\s+')
_rx_match_spaces = re.compile(r'^\s+$')

## Parser ##

_MP = _mecabman.MeCabParser()

#warmup = _MP.warmup
parse = _MP.parse

tolou = _MP.tolou
#def tolou(text, **kwargs):
#  """
#  @param  text  unicode
#  @param* space  bool
#  @return  unicode  plan text
#  """
#  return _MP.tolou(text, **kwargs)

_repl_capitalize = skstr.multireplacer({
  #' Da ': ' da ',
  ' De ': ' de ',
  ' Ha ': ' ha ',
  ' Na ': ' na ',
  ' No ': ' no ',
  ' Ni ': ' ni ',
  ' To ': ' to ',
  #' O ': ' o ',
  ' Wo ': ' wo ',
})
def capitalizeromaji(text):
  """
  @param  text  unicode
  @return  unicode
  """
  return _repl_capitalize(text.title())

#toyomi = _MP.toyomi
def toyomi(text, space=True, capital=False, **kwargs):
  """
  @param  text  unicode
  @param* space  bool
  @return  unicode  plan text
  """
  ret = ' '.join(_MP.toyomi(it, **kwargs)
    for it in _rx_spaces.split(text) if it
  ) if space else _MP.toyomi(it, **kwargs)
  if capital:
    ret = capitalizeromaji(ret)
  return ret

def toromaji(text, **kwargs):
  """
  @param  text  unicode
  @param* space  bool
  @return  unicode  plain text
  """
  return toyomi(text, furiType=defs.FURI_ROMAJI, **kwargs)

#def getfeaturesurface(feature):
#  """
#  @param  feature  unicode
#  @return  unicode
#  """
#  if feature:
#    ret = feature.split(',')[-3]
#    if ret != '*':
#      return ret

def renderfeature(feature, fmt):
  """
  @param  feature  unicode
  @param  fmt  mecabfmt
  @return  unicode or None

  Example feature: 名詞,サ変接続,*,*,*,*,感謝,カンシャ,カンシャ
  Input feature:
    品詞,品詞細分類1,品詞細分類2,品詞細分類3,活用形,活用型,原形,読み,発音
  Rendered feature:
    原形,発音,品詞,品詞細分類1,品詞細分類2,品詞細分類3,活用形
  """
  l = feature.replace('*', '').split(',')

  ret = filter(bool, l[:fmt.COL_BASIC])

  surface = l[fmt.COL_SURFACE]
  kata = l[fmt.COL_KATA]
  hira = cconv.kata2hira(kata)

  if hasattr(fmt, 'COL_KANJI'):
    kanji = l[fmt.COL_KANJI]
    if kanji not in (surface, kata, hira):
      ret.insert(0, kanji)

  if hasattr(fmt, 'COL_ORIG'):
    orig = l[fmt.COL_ORIG]
    ret.append(orig)

  #if surface not in (hira, kata):
  if hira != surface:
    ret.insert(0, hira)
  ret.insert(0, surface)

  return ', '.join(ret)

def rendertable(text, termEnabled=False, furiType=defs.FURI_HIRA, features=None, rubySize='10px', colorize=False, center=True):
  """
  @param  text  unicode
  @param* furiType  int
  @param* rubySize  str
  @param* colorsize  bool
  @param* center  bool
  @param* termEnabled  bool  whether enable terms
  @param* features  {unicode surface:(unicode feature, fmt)} or None
  @return  unicode  HTML table
  """
  l = []
  i = j = 0
  hasfeature = features is not None
  color = None
  for it in parse(text, termEnabled=termEnabled, type=True, feature=hasfeature, reading=True, furiType=furiType):
    if hasfeature:
      surface, ch, yomi, f, fmt = it
    else:
      surface, ch, yomi = it
    if colorize:
      color = None
      if ch in (mecabdef.TYPE_VERB, mecabdef.TYPE_NOUN, mecabdef.TYPE_KATAGANA):
          i += 1
          color = 'rgba(255,0,0,40)' if i % 2 else 'rgba(255,255,0,40)'   # red or yellow
      elif ch == mecabdef.TYPE_MODIFIER: # adj or adv
        j += 1
        #color = "rgba(0,255,0,40)" if j % 2 else "rgba(255,0,255,40)" # green or magenta
        color = "rgba(0,255,0,40)" if j % 2 else "rgba(0,0,255,40)" # green or blue
    l.append((surface, yomi, color))
    if hasfeature:
      features[surface] = f, fmt
  return "" if not l else rc.jinja_template('html/furigana').render({
    'tuples': l,
    #'bgcolor': "rgba(0,0,0,5)",
    'rubySize': rubySize,
    'center': center,
  })

## Render HTML ruby ##

_WORD_CSS_CLASSES = 'word2', 'word1', 'word4', 'word3'
def _renderruby_word_iter(text, hasClass, features, **kwargs):
  """
  @yield  (str styleClass or None, unicode surface, unicode furigana not None)
  """
  l = []
  i = j = 0
  hasfeature = features is not None
  styleClass = None
  for it in parse(text, type=True, reading=True, feature=hasfeature, **kwargs):
    if hasfeature:
      surface, ch, yomi, f, fmt = it
    else:
      surface, ch, yomi = it
    if hasClass:
      styleClass = None
      if ch in (mecabdef.TYPE_VERB, mecabdef.TYPE_NOUN, mecabdef.TYPE_KATAGANA):
        i += 1
        #color = 'rgba(255,0,0,40)' if i % 2 else 'rgba(255,255,0,40)'   # red or cyan
        styleClass = _WORD_CSS_CLASSES[i % 2]
      elif ch == mecabdef.TYPE_MODIFIER: # adj or adv
        j += 1
        #color = "rgba(0,255,0,40)" if j % 2 else "rgba(0,0,255,40)" # green or blue
        styleClass = _WORD_CSS_CLASSES[2 + j % 2]
    if hasfeature:
      features[surface] = f, fmt
    yield styleClass, surface, yomi or ''

def _renderruby_word(text, hasClass, **kwargs):
  """
  @return  unicode  HTML
  """
  q = _renderruby_word_iter(text, hasClass=hasClass, **kwargs)
  if hasClass:
    return ''.join('<ruby class="%s"><rb>%s</rb><rt>%s</rt></ruby>' % it for it in q)
  else:
    return ''.join('<ruby><rb>%s</rb><rt>%s</rt></ruby>' % it[1:] for it in q)

#_rx_spaces = re.compile(r'\s+')
def _renderruby_segment(text, **kwargs):
  """
  @return  unicode  HTML
  """
  return ' '.join(_renderruby_word(it, **kwargs)
      for it in _rx_spaces.split(text) if it)

__SENTENCE_DELIM = u"\n。!！？♪…【】■" # missing "?", which would cause trouble when it comes to "(?)"
_SENTENCE_SET = frozenset(__SENTENCE_DELIM)
_SENTENCE_RE = re.compile(r"(%s)" % '|'.join(_SENTENCE_SET))
def _renderruby_sentence_iter(text, **kwargs):
  """
  @yield  unicode
  """
  for it in _SENTENCE_RE.split(text):
    if it:
      if it in _SENTENCE_SET:
        yield it
      else:
        yield _renderruby_segment(it, **kwargs)

def _renderruby_sentence(text, **kwargs):
  """
  @return  unicode  HTML
  """
  return ''.join(' ' if _rx_match_spaces.match(it) else '<span class="sentence">%s</span>' % it
      for it in _renderruby_sentence_iter(text, **kwargs) if it)

_rx_html_tags = re.compile(r"(<.+?>)")
def _renderruby_html_iter(text, **kwargs):
  """
  @yield  unicode  HTML
  """
  for it in _rx_html_tags.split(text):
    if it:
      if it[0] == '<':
        yield it
      elif _rx_match_spaces.match(text):
        yield ' '
      else:
        yield _renderruby_sentence(it, **kwargs)

def _renderruby_html(text, **kwargs):
  """
  @return  unicode  HTML
  """
  return ''.join(_renderruby_html_iter(text, **kwargs))

def renderruby(text, html=False, termEnabled=False, hasClass=True, furiType=defs.FURI_HIRA, features=None):
  """
  @param  text  unicode
  @param* html  bool  whether input text contains html tags
  @param* termEnabled  bool  whether use termman
  @param* furiType  int
  @param* hasClass  bool  whether have style classes
  @param* features  {unicode surface:unicode feature} or None
  @return  unicode  HTML ruby list
  """
  if not text:
    return ''
  if html:
    return _renderruby_html(text,
        termEnabled=termEnabled, hasClass=hasClass, furiType=furiType, features=features)
  else:
    return _renderruby_segment(text,
        termEnabled=termEnabled, hasClass=hasClass, furiType=furiType, features=features)

## Manager ##

class MeCabManager:

  def __init__(self):
    self.rubyType = '' # str
    self.dicName = '' # str

  def isEnabled(self):
    return _MP.enabled #and os.path.exists(_MP.dicdir)

  def setEnabled(self, t): # bool
    _MP.setenabled(t)

  def setRubyType(self, v):
    """
    @param  name  str
    """
    self.rubyType = v

  def userDictionary(self): # -> unicode path
    """User-defined dictionary path
    @return  unicode  path
    """
    return _MP.userdic

  def setUserDictionary(self, v): # unicode path ->
    """
    @param  v  unicode  path
    """
    dprint(v)
    _MP.setuserdic(v)

  def clearUserDictionary(self): self.setUserDictionary('')

  def setDictionaryName(self, v):
    """
    @param  name  str
    """
    if self.dicName != v:
      dprint(v)
      self.dicName = v
      _MP.setfmt(mecabfmt.getfmt(v))
      _MP.setrcfile(rc.mecab_rc_path(v))

  def toRuby(self, text, html=False):
    """
    @param  text  unicode  HTML
    @param* html  bool  whether input text contains html tags
    @return  unicode HTML not None
    """
    if not _MP.enabled:
      return text or ''
    return renderruby(text, html=html, furiType=self.rubyType)

  def toYomi(self, text):
    """
    @param  text  unicode  plain text
    @return  unicode plain text
    """
    if not _MP.enabled:
      return ''
    return toyomi(text, furiType=self.rubyType)

@memoized
def manager(): return MeCabManager()

class MeCabCoffeeBean(QObject):
  def __init__(self, parent=None):
    super(MeCabCoffeeBean, self).__init__(parent)

  @Slot(unicode, bool, result=unicode)
  def toRuby(self, text, html):
    """
    @param  text  unicode  HTML
    @param  html  bool  whether input text contains html tags
    @return  unicode  HTML
    """
    return manager.toRuby(text, html)

  @Slot(unicode, result=unicode)
  def toYomi(self, text):
    """
    @param  text  unicode  plain text
    @return  unicode  plain text
    """
    return manager().toYomi(text)

## Highlighter ##

class MeCabHighlighter(QSyntaxHighlighter):

  def highlightBlock(self, text):
    """@reimp @protected"""
    if not _MP.enabled:
      return
    i = j = 0
    offset = 0
    for surface, ch in parse(text, type=True):
      offset += len(surface)
      if ch in (mecabdef.TYPE_VERB, mecabdef.TYPE_NOUN, mecabdef.TYPE_KATAGANA):
        i += 1
        color = Qt.red if i % 2 else Qt.cyan
        alpha = 40
      elif ch == mecabdef.TYPE_MODIFIER: # adj or adv
        j += 1
        color = Qt.green if j % 2 else Qt.magenta
        alpha = 20
      else:
        continue
      color = QColor(color)
      color.setAlpha(alpha)

      fmt = QTextCharFormat()
      fmt.setBackground(color)

      length = len(surface)
      start = offset - length
      self.setFormat(start, length, fmt)

@Q_Q
class _QmlMeCabHighlighter(object):
  def __init__(self):
    self.enabled = False
    self.document = None # QTextDocument

  # WHY weakref do not work here? This will cause memory leak!
  #@property
  #def document(self):
  #  """
  #  @return  QTextDocument or None
  #  """
  #  try: return self._document_ref()
  #  except (AttributeError, TypeError): pass

  #@document.setter
  #def document(self, value):
  #  self._document_ref = weakref.ref(value) if value else None

  @property
  def highlighter(self):
    try: return self._highlighter_ref()
    except AttributeError:
      obj = MeCabHighlighter(self.q)
      #obj.setDocument(self.document)
      self._highlighter_ref = weakref.ref(obj)
      return self._highlighter_ref()

  def hasHighlighter(self): return hasattr(self, '_highlighter_ref')

  def clearDocument(self):
    #self.q.setDocument(None)
    self.document = None
    self.highlighter.setDocument(None)
    #self.q.documentChanged.emit(None)

#@QmlObject # jichi: why not needed?
class QmlMeCabHighlighter(QObject):
  def __init__(self, parent=None):
    super(QmlMeCabHighlighter, self).__init__(parent)
    self.__d = _QmlMeCabHighlighter(self)

  def setDocument(self, doc):
    d = self.__d
    if d.document is not doc:
      if doc:
        doc.destroyed.connect(d.clearDocument)
      d.document = doc
      if d.hasHighlighter():
        d.highlighter.setDocument(doc if d.enabled else None)
      self.documentChanged.emit(doc)

  documentChanged = Signal(QObject) # QtWidgets.QTextDocument
  document = Property(QObject,
      lambda self: self.__d.document,
      setDocument,
      notify=documentChanged)

  def setEnabled(self, value):
    d  = self.__d
    if value != d.enabled:
      d.enabled = value
      d.highlighter.setDocument(d.document if value else None)
      self.enabledChanged.emit(value)
  enabledChanged = Signal(bool)
  enabled = Property(bool,
      lambda self: self.__d.enabled,
      setEnabled,
      notify=enabledChanged)

if __name__ == '__main__':
  #t = u"すもももももももものうち"
  #t = u"憎しみは憎しみしか生まない"
  #t = u"童話はほとんど墓场から始まるものさ"
  t = u"Chun、ShinGura"
  t = u"レミニ、センス"
  t = u'桜小路ルナ'
  t = u'【桜小路ルナ】'
  for x,y in parse(t, feature=True):
    print y.encode('sjis', errors='ignore')

  import sys
  sys.exit(0)

  print 'toyomi:', toyomi(t)
  print 'toromaji:', toromaji(t)

  t = u"人類は衰退しました"
  t = u"ゆうせい"
  t = u"遊星"
  t = u"ユーザ設定が必要です。"
  t = u"遊星が必要です。"
  t = u"桜小路ルナが必要です。"
  print 'tolou:', tolou(t)

# EOF
