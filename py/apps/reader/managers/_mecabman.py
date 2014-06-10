# coding: utf8
# _mecabman.py
# 10/12/2012 jichi
#
# doc: http://mecab.googlecode.com/svn/trunk/mecab/doc/index.html
# doxygen: http://mecab.googlecode.com/svn/trunk/mecab/doc/doxygen/annotated.html

__all__ = ['MeCabParser']

import os
import MeCab
from sakurakit import skstr
from sakurakit.skdebug import dwarn
from cconv import cconv
from jptraits import jpchars
from mecabjlp import mecabdef, mecabfmt, mecabtag
import defs, dicts, osutil, termman

from msime import msime
HAS_MSIME = msime.ja_valid() # cached

#def _tothinromaji(text):
#  """
#  @param  text  unicode
#  @return  unicode
#  """
#  return wide2thin(_toromaji(text))

#def _f1(x): return x

_WORDTRANS = {} # {unicode text:unicode translate"
def _wordtrans(text):
  """
  @param  text  unicode
  @return  unicode or None
  """
  global _WORDTRANS
  if text not in _WORDTRANS:
    _WORDTRANS[text] = dicts.edict().translate(text)
  return _WORDTRANS[text]

## Parser ##

class MeCabParser:
  #def __init__(self):
  #  self.tagger = MeCab.Tagger("")
  #  self.warmed = False

  def __init__(self):
    self.enabled = False # bool
    self.userdic = '' # unicode
    self.rcfile = '' # unicode
    self.fmt = mecabfmt.DEFAULT

    self.deftagger = None # cached tagger without user dic
    self.usertagger = None # cached tagger with user dic

  def setenabled(self, v): self.enabled = v

  def setfmt(self, v): self.fmt = v

  def setuserdic(self, v):
    v = osutil.get_relpath(v) if v else '' # force relative path
    #v = os.path.abspath(v) if v else ''
    if v != self.userdic:
      self.userdic = v
      self.usertagger = None

  def setrcfile(self, v):
    #v = osutil.get_relpath(v) if v else '' # force relative path
    #v = os.path.abspath(v) if v else ''
    #v = v.replace('\\', '/')
    if v != self.rcfile:
      self.rcfile = v
      self.deftagger = None
      self.usertagger = None
      mecabtag.setenvrc(v) if v else mecabtag.delenvrc()

  def tagger(self):
    """
    @return  MeCab.Tagger or None
    """
    if not self.enabled:
      return None
    if self.userdic and self.rcfile:
      if self.usertagger:
        return self.usertagger
      if os.path.exists(self.userdic):
        args = mecabtag.maketaggerargs(userdic=self.userdic)
        self.usertagger = mecabtag.createtagger(args)
        if self.usertagger:
          return self.usertagger
      self.userdic = ''
    if self.rcfile:
      if self.deftagger:
        return self.deftagger
      if os.path.exists(self.rcfile):
        args = mecabtag.maketaggerargs()
        self.deftagger = mecabtag.createtagger(args)
        if self.deftagger:
          return self.deftagger
      self.rcfile = ''

  #_rx_cypher = re.compile(ur"(?<=["
  #  u"ルユュムフブプヌツヅスク"
  #  u"ロヨョモホボポノトドソゾコォ"
  #u"])ー")
  #def _repairkatagana(self, text):
  #  """
  #  @param  text  unicode
  #  @return  unicode
  #  """
  #  return self._rx_cypher.sub(u'ウ', text)

  #@staticmethod
  #def _prepare(text):
  #  """
  #  @param  text  unicode
  #  @return  unicode
  #  """
  #  return text

  def toyomi(self, text, termEnabled=False, furiType=defs.FURI_HIRA):
    """
    @param  text  unicode
    @param* termEnabled  bool  whether query terms
    @param* type  bool  whether return type
    @param* furiType  unicode
    @return  unicode
    """
    if not self.enabled:
      return ''
    furitrans = (cconv.kata2hira if furiType == defs.FURI_HIRA else
                 cconv.hira2kata if furiType == defs.FURI_KATA else
                 cconv.yomi2romaji if furiType == defs.FURI_ROMAJI else
                 cconv.yomi2hangul if furiType == defs.FURI_HANGUL else
                 cconv.yomi2thai if furiType == defs.FURI_THAI else
                 cconv.yomi2kanji if furiType == defs.FURI_KANJI else
                 cconv.kata2hira)
    # Add space between words
    return ' '.join(furigana or furitrans(surface) for surface,furigana in
        self.parse(text, termEnabled=termEnabled, reading=True, furiType=furiType))

  def tolou(self, text, termEnabled=False, furiType=defs.FURI_TR):
    """
    @param  text  unicode
    @param* termEnabled  bool  whether query terms
    @param* type  bool  whether return type
    @param* furiType  unicode
    @return  unicode
    """
    if not self.enabled:
      return ''
    # Add space between words
    return ' '.join(furigana or surface for surface,furigana in
        self.parse(text, termEnabled=termEnabled, reading=True, lougo=True, furiType=furiType))

  def parse(self, text, termEnabled=False, type=False, fmt=None, reading=False, feature=False, lougo=False, furiType=defs.FURI_HIRA, readingTypes=(mecabdef.TYPE_VERB, mecabdef.TYPE_NOUN)):
    """
    @param  text  unicode
    @param* termEnabled  bool  whether query terms
    @param* type  bool  whether return type
    @param* reading  bool   whether return yomigana
    @param* feature  bool   whether return feature
    @param* furiType  unicode
    @param* readingTypes  (int type) or [int type]
    @param* lougo  bool
    @yield  (unicode surface, int type, unicode yomigana or None, unicode feature, fmt or None)
    """
    tagger = self.tagger()
    if not tagger:
      raise StopIteration
    if termEnabled:
      tm = termman.manager()
      termEnabled = tm.isEnabled()
    if termEnabled:
      text = tm.applyWordTerms(text)
    if not fmt:
      fmt = self.fmt # mecabfmt
    if reading:
      wordtrans = _wordtrans if furiType == defs.FURI_TR else None
      katatrans = (cconv.kata2hira if furiType == defs.FURI_HIRA else
                   cconv.kata2hangul if furiType == defs.FURI_HANGUL else
                   cconv.kata2thai if furiType == defs.FURI_THAI else
                   cconv.kata2kanji if furiType == defs.FURI_KANJI else
                   cconv.kata2romaji if furiType in (defs.FURI_ROMAJI, defs.FURI_TR) else
                   None)
      if termEnabled:
        furitrans = (cconv.kata2hira if furiType == defs.FURI_HIRA else
                     cconv.hira2kata if furiType == defs.FURI_KATA else
                     cconv.yomi2hangul if furiType == defs.FURI_HANGUL else
                     cconv.yomi2thai if furiType == defs.FURI_THAI else
                     cconv.yomi2romaji)
      if furiType in (defs.FURI_ROMAJI, defs.FURI_HANGUL, defs.FURI_THAI, defs.FURI_KANJI):
        readingTypes = None
    feature2katana = fmt.getkata
    node = tagger.parseToNode(text.encode(mecabdef.DICT_ENCODING))
    while node:
      if node.stat not in (MeCab.MECAB_BOS_NODE, MeCab.MECAB_EOS_NODE):
        surface = node.surface[:node.length];
        surface = surface.decode(mecabdef.DICT_ENCODING, errors='ignore')
        #surface = surface.encode('sjis')
        if surface:
          if len(surface) == 1 and surface in jpchars.set_punc:
            char_type = mecabdef.TYPE_PUNCT
          else:
            char_type = node.char_type

          if reading:
            yomigana = None
            #if node.char_type in (mecabdef.TYPE_VERB, mecabdef.TYPE_NOUN, mecabdef.TYPE_KATAGANA, mecabdef.TYPE_MODIFIER):
            f = None
            if feature:
              f = node.feature.decode(mecabdef.DICT_ENCODING, errors='ignore')
            if not readingTypes or char_type in readingTypes or char_type == mecabdef.TYPE_KATAGANA and wordtrans: # always translate katagana
              if wordtrans:
                if termEnabled:
                  yomigana = tm.queryLatinWordTerms(surface)
                if not yomigana:
                  yomigana = wordtrans(surface)
              if not yomigana and not lougo:
                if not feature:
                  f = node.feature.decode(mecabdef.DICT_ENCODING, errors='ignore')
                katagana = feature2katana(f)
                if katagana:
                  furigana = tm.queryFuriTerms(surface) if termEnabled else None
                  if furigana:
                    furigana = furitrans(furigana)
                    if furigana != surface:
                      yomigana = furigana
                  elif katagana == '*':
                    # Use MSIME as fallback
                    unknownYomi = True
                    if HAS_MSIME and len(surface) < msime.IME_MAX_SIZE:
                      if furiType == defs.FURI_HIRA:
                        yomigana = msime.to_yomi_hira(surface)
                      else:
                        yomigana = msime.to_yomi_kata(surface)
                        if yomigana:
                          if furiType == defs.FURI_HIRA:
                            pass
                          elif furiType == defs.FURI_ROMAJI:
                            yomigana = cconv.wide2thin(cconv.kata2romaji(yomigana))
                            if yomigana == surface:
                              yomigana = None
                              unknownYomi = False
                          elif furiType == defs.FURI_HANGUL:
                            yomigana = cconv.kata2hangul(yomigana)
                          elif furiType == defs.FURI_KANJI:
                            yomigana = cconv.kata2kanji(yomigana)
                    if not yomigana and unknownYomi and readingTypes:
                      yomigana = '?'
                  else:
                    #katagana = self._repairkatagana(katagana)
                    yomigana = katatrans(katagana) if katatrans else katagana
                    if yomigana == surface:
                      yomigana = None
            if not type and not feature:
              yield surface, yomigana
            elif type and not feature:
              yield surface, char_type, yomigana
            elif not type and feature:
              yield surface, yomigana, f, fmt
            else: # render all
              yield surface, char_type, yomigana, f, fmt
          elif not type and not feature:
            yield surface
          elif type and not feature: # and type
            yield surface, char_type
          elif not type and feature:
            f = node.feature.decode(mecabdef.DICT_ENCODING, errors='ignore')
            yield surface, f, fmt
          elif type and feature:
            f = node.feature.decode(mecabdef.DICT_ENCODING, errors='ignore')
            yield surface, char_type, f, fmt
          #else:
          #  assert False, "unreachable"

      node = node.next

# EOF

  #@staticmethod
  #def textColorIter(text, colorize):
  #  """
  #  @param  unicode  text
  #  @param  bool  colorize
  #  @yield  unicode surface, str color
  #  """
  #  i = j = 0
  #  for surface, ch in parse(text, type=True):
  #    color = None
  #    if ch in (mecabdef.TYPE_VERB, mecabdef.TYPE_NOUN, mecabdef.TYPE_KATAGANA):
  #      if colorize:
  #        i += 1
  #        color = 'rgba(255,0,0,40)' if i % 2 else 'rgba(255,255,0,40)'   # red or cyan
  #    elif ch == mecabdef.TYPE_MODIFIER: # adj or adv
  #      if colorize:
  #        j += 1
  #        color = "rgba(0,255,0,40)" if j % 2 else "rgba(255,0,255,40)" # green or magenta
  #    yield surface, color

  #@staticmethod
  #def textIter(text): return parse(text)

#@QmlObject
#class MeCabJinja(QObject):
#  def __init__(self, parent=None):
#    super(MeCabJinja, self).__init__(parent)

#@Slot(unicode, bool, result=unicode)
  #@Slot(unicode, result=unicode)
  #def renderText(self, text):
  #  return ' '.join(_MeCabJinja.textIter(text))


# def renderFurigana(self, text,
#     rubySize="10px", colorize=True, colorizeAlpha=40):
#   """
#   @param  text  unicode
#   @param  rubySize  unicode   Font size of <ruby> tag
#   @param  colorize  bool  Whether render background color
#   @param  colorizeAlpha  int  Alpha of background color
#   @return  unicode or ""
#   """
#   ret = u""
#   yomigana = u""
#   i = j = 0
#   for surface, ch, furigana in self.parse(text, type=True, reading=True):
#     color = None

#     if furigana:
#       yomigana += (u"<td align='center' style='font-size:%s;font-weight:bold;background-color:rgba(0,0,0,5)'>%s</td>"
#                 % (rubySize, furigana))
#     else:
#       yomigana += u"<td/>"

#     if ch in (mecabdef.TYPE_VERB, mecabdef.TYPE_NOUN, mecabdef.TYPE_KATAGANA):
#       if colorize:
#         i+=1
#         color = QColor(Qt.red if i % 2 else Qt.cyan)
#         color.setAlpha(colorizeAlpha)
#     elif ch == mecabdef.TYPE_MODIFIER: # adj or adv
#       if colorize:
#         j+=1
#         color = QColor(Qt.green if j % 2 else Qt.magenta)
#         color.setAlpha(colorizeAlpha)
#     else:
#       ret += (u"<td align='center' style='background-color:rgba(0,0,0,5)'>%s</td>"
#            % surface)
#       continue

#     if color:
#       rgba = (u"rgba(%i,%i,%i,%i)"
#            % (color.red(), color.green(), color.blue(), color.alpha()))
#     else:
#       rgba = u"rgba(0,0,0,5)"

#     ret += (u"<td align='center' style='background-color:%s'>%s</td>"
#          % (rgba, surface))

#   ret = (
#     u"<center>"
#       u"<table>"
#         u"<tr valign='bottom'>%s</tr>"
#         u"<tr>%s</tr>"
#       u"</table>"
#     u"</center>"
#     % (yomigana, ret))

#   return ret
