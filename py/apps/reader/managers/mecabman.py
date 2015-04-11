# coding: utf8
# mecabman.py
# 10/12/2012 jichi
from sakurakit.skclass import memoized
from mecabparser import mecabdef
from mytr import my
import growl, rc

@memoized
def manager(): return MeCabManager()

## Manager class

class _MeCabManager:
  def __init__(self):
    self.userdicEnabled = False
    self._parserWithUserdic = None
    self._parserWithoutUserdic = None

  def getParser(self):
    """
    @return mecabparser.MeCabParser or None
    """
    return self._getParserWithUnidic() if self.userdicEnabled else self._getParserWithoutUserdic()

  def isParserLoaded(self):
    return bool(self._parserWithUserdic or self._parserWithoutUserdic)

  def _getParserWithoutUnidic(self):
    if not self._parserWithUserdic:
      import dicts
      if not dicts.unidic().exists():
        dwarn("missing unidic")
        growl.warn(my.tr("MeCab UniDic dictionary not found"))
        return
      from mecabparser import mecabparser
      self._parserWithUserdic = mecabparser.MeCabParser()
    return self._parserWithUserdic

  def _getParserWithUnidic(self):
    if not self._parserWithUserdic:
      import os
      userdic = rc.MECAB_EDICT_RELPATH
      if not os.path.exists(userdic):
        dwarn("missing edict")
        growl.warn(my.tr("MeCab EDICT dictionary not found"))
        return self._getParserWithoutUserdic()
      from mecabparser import mecabparser
      ret = mecabparser.MeCabParser()
      ret.setUseric(userdic)
      self._parserWithUserdic = ret
    return self._parserWithUserdic

class MeCabManager:

  def __init__(self):
    self.__d = _MeCabManager()

  def isLoaded(self): return self.__d.isParserLoaded()

  def isEdictEnabled(self, t): return self.__d.userdicEnabled
  def setEdictEnabled(self, t): self.__d.userdicEnabled = t

  def parse(self, text, rubyType, rubyKana=False):
    """
    @param  rubyType  str
    @param* rubyKana  bool
    @yield  (unicode surface, unicode ruby, unicode feature) or return None
    """
    try: return self.__d.getParser().iterparseToRuby(self, text, rubyType, show_ruby_kana=rubyKana)
    except Exception, e: dwawrn(e)

  def toRomaji(self, capital=True):
    """
    @param  text  unicode
    @param  capital  bool
    @return  unicode
    """
    try: return self.__d.getParser().toRomaji(text, capital=capital)
    except Exception, e: dwarn(e)
    return ''

## Render feature

def renderfeature(feature):
  """
  @param  feature  unicode
  @return  unicode
  """
  # TODO: Unimplemented
  return feature

## Render table

def _iterrendertable(text, rubyType, rubyKana=False, features=None, charPerLine=100, rubySize=10, colorize=False, center=True):
  """
  @param  text  unicode
  @param  rubyType  str
  @param* rubyKana  bool
  @param* charPerLine  int  maximum number of characters per line
  @param* rubySize  float
  @param* colorsize  bool
  @param* center  bool
  @param* features  {unicode surface:(unicode feature, fmt)} or None
  @yield  unicode  HTML table
  """
  q = manager().parse(text, rubyType, rubyKana=rubyKana)
  if q:
    render = rc.jinja_template('html/furigana').render

    i = j = 0
    line = []
    lineCount = 0 # int  estimated line width, assume ruby has half width
    hasfeature = features is not None
    color = None

    PADDING_FACTOR = 0.3
    LATIN_YOMI_WIDTH = 0.33 # = 2/6
    KANJI_YOMI_WIDTH = 0.55 # = 1/2
    # yomi size / surface size
    yomiWidth = LATIN_YOMI_WIDTH if rubyType in (mecabdef.RB_ROMAJI, mecabdef.RB_RU, mecabdef.RB_TH, mecabdef.RB_AR, mecabdef.RB_TR) else KANJI_YOMI_WIDTH

    invertRuby = False # always disable inverting
    roundRubySize = int(round(rubySize)) or 1
    paddingSize = int(round(rubySize * PADDING_FACTOR)) or 1 if invertRuby else 0

    for surface, yomi, feature, surface_type in q:
      if hasfeature:
        features[surface] = feature

      if colorize:
        if surface_type == mecabdef.SURFACE_KANJI:
          i += 1
          color = 'rgba(255,0,0,40)' if i % 2 else 'rgba(255,255,0,40)' # red or yellow
        elif surface_type == mecabdef.SURFACE_KANA:
          j += 1
          #color = "rgba(0,255,0,40)" if j % 2 else "rgba(255,0,255,40)" # green or magenta
          color = "rgba(0,255,0,40)" if j % 2 else "rgba(0,0,255,40)" # green or blue
        else:
          color = None

      width = max(len(surface), len(yomi)*yomiWidth if yomi else 0)
      if width + lineCount <= charPerLine:
        pass
      elif line:
        yield render({
          'tuples': line,
          'rubySize': roundRubySize,
          'paddingSize': paddingSize,
          'center': center,
        })
        line = []
        lineCount = 0
      group = None # group is none
      if invertRuby and yomi:
        #if surface:
        #  surface = wide2thin(surface)
        if furiType in (defs.FURI_ROMAJI, defs.FURI_ROMAJI_RU) and len(yomi) > 2:
          yomi = yomi.title()
        t = yomi, surface, color, group
      else:
        t = surface, yomi, color, group
      line.append(t)
      lineCount += width
    if line:
      yield render({
        'tuples': line,
        'rubySize': roundRubySize,
        'paddingSize': paddingSize,
        'center': center,
      })

def rendertable(*args, **kwargs):
  """Parameters are the same as _iterrendertable"""
  return ''.join(_iterrendertable(*args, **kwargs))

# EOF

#_rx_spaces = re.compile(r'\s+')
#_rx_match_spaces = re.compile(r'^\s+$')
#
### Render HTML ruby ##
#
#_WORD_CSS_CLASSES = 'word2', 'word1', 'word4', 'word3'
#def _renderruby_word_iter(text, hasClass, features, **kwargs):
#  """
#  @yield  (str styleClass or None, unicode surface, unicode furigana not None)
#  """
#  l = []
#  i = j = 0
#  hasfeature = features is not None
#  styleClass = None
#  for it in parse(text, type=True, reading=True, feature=hasfeature, **kwargs):
#    if hasfeature:
#      surface, ch, yomi, f, fmt = it
#    else:
#      surface, ch, yomi = it
#    if hasClass:
#      styleClass = None
#      if ch in (mecabdef.TYPE_VERB, mecabdef.TYPE_NOUN, mecabdef.TYPE_KATAGANA):
#        i += 1
#        #color = 'rgba(255,0,0,40)' if i % 2 else 'rgba(255,255,0,40)'   # red or cyan
#        styleClass = _WORD_CSS_CLASSES[i % 2]
#      elif ch == mecabdef.TYPE_MODIFIER: # adj or adv
#        j += 1
#        #color = "rgba(0,255,0,40)" if j % 2 else "rgba(0,0,255,40)" # green or blue
#        styleClass = _WORD_CSS_CLASSES[2 + j % 2]
#    if hasfeature:
#      features[surface] = f, fmt
#    yield styleClass, surface, yomi or ''
#
#def _renderruby_word(text, hasClass, **kwargs):
#  """
#  @return  unicode  HTML
#  """
#  q = _renderruby_word_iter(text, hasClass=hasClass, **kwargs)
#  if hasClass:
#    return ''.join('<ruby class="%s"><rb>%s</rb><rt>%s</rt></ruby>' % it for it in q)
#  else:
#    return ''.join('<ruby><rb>%s</rb><rt>%s</rt></ruby>' % it[1:] for it in q)
#
##_rx_spaces = re.compile(r'\s+')
#def _renderruby_segment(text, **kwargs):
#  """
#  @return  unicode  HTML
#  """
#  return ' '.join(_renderruby_word(it, **kwargs)
#      for it in _rx_spaces.split(text) if it)
#
#__SENTENCE_DELIM = u"\n。!！？♪…【】■" # missing "?", which would cause trouble when it comes to "(?)"
#_SENTENCE_SET = frozenset(__SENTENCE_DELIM)
#_SENTENCE_RE = re.compile(r"(%s)" % '|'.join(_SENTENCE_SET))
#def _renderruby_sentence_iter(text, **kwargs):
#  """
#  @yield  unicode
#  """
#  for it in _SENTENCE_RE.split(text):
#    if it:
#      if it in _SENTENCE_SET:
#        yield it
#      else:
#        yield _renderruby_segment(it, **kwargs)
#
#def _renderruby_sentence(text, **kwargs):
#  """
#  @return  unicode  HTML
#  """
#  return ''.join(' ' if _rx_match_spaces.match(it) else '<span class="sentence">%s</span>' % it
#      for it in _renderruby_sentence_iter(text, **kwargs) if it)
#
#_rx_html_tags = re.compile(r"(<.+?>)")
#def _renderruby_html_iter(text, **kwargs):
#  """
#  @yield  unicode  HTML
#  """
#  for it in _rx_html_tags.split(text):
#    if it:
#      if it[0] == '<':
#        yield it
#      elif _rx_match_spaces.match(text):
#        yield ' '
#      else:
#        yield _renderruby_sentence(it, **kwargs)
#
#def _renderruby_html(text, **kwargs):
#  """
#  @return  unicode  HTML
#  """
#  return ''.join(_renderruby_html_iter(text, **kwargs))
#
#def renderruby(text, html=False, termEnabled=False, hasClass=True, furiType=defs.FURI_HIRA, features=None):
#  """
#  @param  text  unicode
#  @param* html  bool  whether input text contains html tags
#  @param* termEnabled  bool  whether use termman
#  @param* furiType  int
#  @param* hasClass  bool  whether have style classes
#  @param* features  {unicode surface:unicode feature} or None
#  @return  unicode  HTML ruby list
#  """
#  if not text:
#    return ''
#  if html:
#    return _renderruby_html(text,
#        termEnabled=termEnabled, hasClass=hasClass, furiType=furiType, features=features)
#  else:
#    return _renderruby_segment(text,
#        termEnabled=termEnabled, hasClass=hasClass, furiType=furiType, features=features)
#
#class MeCabCoffeeBean(QObject):
#  def __init__(self, parent=None):
#    super(MeCabCoffeeBean, self).__init__(parent)
#
#  @Slot(unicode, bool, result=unicode)
#  def toRuby(self, text, html):
#    """
#    @param  text  unicode  HTML
#    @param  html  bool  whether input text contains html tags
#    @return  unicode  HTML
#    """
#    return manager.toRuby(text, html)
#
#  @Slot(unicode, result=unicode)
#  def toYomi(self, text):
#    """
#    @param  text  unicode  plain text
#    @return  unicode  plain text
#    """
#    return manager().toYomi(text)
#
### Highlighter ##
#
#class MeCabHighlighter(QSyntaxHighlighter):
#
#  def highlightBlock(self, text):
#    """@reimp @protected"""
#    if not _MP.enabled:
#      return
#    i = j = 0
#    offset = 0
#    for surface, ch in parse(text, type=True):
#      offset += len(surface)
#      if ch in (mecabdef.TYPE_VERB, mecabdef.TYPE_NOUN, mecabdef.TYPE_KATAGANA):
#        i += 1
#        color = Qt.red if i % 2 else Qt.cyan
#        alpha = 40
#      elif ch == mecabdef.TYPE_MODIFIER: # adj or adv
#        j += 1
#        color = Qt.green if j % 2 else Qt.magenta
#        alpha = 20
#      else:
#        continue
#      color = QColor(color)
#      color.setAlpha(alpha)
#
#      fmt = QTextCharFormat()
#      fmt.setBackground(color)
#
#      length = len(surface)
#      start = offset - length
#      self.setFormat(start, length, fmt)
#
#@Q_Q
#class _QmlMeCabHighlighter(object):
#  def __init__(self):
#    self.enabled = False
#    self.document = None # QTextDocument
#
#  # WHY weakref do not work here? This will cause memory leak!
#  #@property
#  #def document(self):
#  #  """
#  #  @return  QTextDocument or None
#  #  """
#  #  try: return self._document_ref()
#  #  except (AttributeError, TypeError): pass
#
#  #@document.setter
#  #def document(self, value):
#  #  self._document_ref = weakref.ref(value) if value else None
#
#  @property
#  def highlighter(self):
#    try: return self._highlighter_ref()
#    except AttributeError:
#      obj = MeCabHighlighter(self.q)
#      #obj.setDocument(self.document)
#      self._highlighter_ref = weakref.ref(obj)
#      return self._highlighter_ref()
#
#  def hasHighlighter(self): return hasattr(self, '_highlighter_ref')
#
#  def clearDocument(self):
#    #self.q.setDocument(None)
#    self.document = None
#    self.highlighter.setDocument(None)
#    #self.q.documentChanged.emit(None)
#
##@QmlObject # jichi: why not needed?
#class QmlMeCabHighlighter(QObject):
#  def __init__(self, parent=None):
#    super(QmlMeCabHighlighter, self).__init__(parent)
#    self.__d = _QmlMeCabHighlighter(self)
#
#  def setDocument(self, doc):
#    d = self.__d
#    if d.document is not doc:
#      if doc:
#        doc.destroyed.connect(d.clearDocument)
#      d.document = doc
#      if d.hasHighlighter():
#        d.highlighter.setDocument(doc if d.enabled else None)
#      self.documentChanged.emit(doc)
#
#  documentChanged = Signal(QObject) # QtWidgets.QTextDocument
#  document = Property(QObject,
#      lambda self: self.__d.document,
#      setDocument,
#      notify=documentChanged)
#
#  def setEnabled(self, value):
#    d  = self.__d
#    if value != d.enabled:
#      d.enabled = value
#      d.highlighter.setDocument(d.document if value else None)
#      self.enabledChanged.emit(value)
#  enabledChanged = Signal(bool)
#  enabled = Property(bool,
#      lambda self: self.__d.enabled,
#      setEnabled,
#      notify=enabledChanged)
