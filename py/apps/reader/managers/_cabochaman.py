# coding: utf8
# _cabochaman.py
# 6/14/2014 jichi
# See: cabocha/src/tree.cpp

__all__ = ['CaboChaParser']

import os
from cconv import cconv
from cabochajlp import cabochadef, cabocharc
from mecabjlp import mecabfmt
import defs, dicts, termman

from msime import msime
HAS_MSIME = msime.ja_valid() # cached

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

class CaboChaParser(object):
  #def __init__(self):
  #  self.parser = CaboCha.Parser()

  def __init__(self):
    self.enabled = False # bool
    self.rcfile = '' # unicode
    self.fmt = mecabfmt.DEFAULT

    self.defparser = None # cached parser

  def setenabled(self, v): self.enabled = v

  def setfmt(self, v): self.fmt = v

  def setrcfile(self, v):
    #v = osutil.get_relpath(v) if v else '' # force relative path
    #v = os.path.abspath(v) if v else ''
    #v = v.replace('\\', '/')
    if v != self.rcfile:
      self.rcfile = v
      self.tagger = None
      cabocharc.setenvrc(v) if v else cabocharc.delenvrc()

  def parser(self):
    """
    @return  CaboCha.Parser or None
    """
    if not self.enabled:
      return None
    if self.rcfile:
      if self.defparser:
        return self.defparser
      if os.path.exists(self.rcfile):
        self.defparser = cabocharc.createparser()
        if self.defparser:
          return self.defparser
      self.rcfile = ''

  def parse(self, text, termEnabled=False, type=False, fmt=None, group=False, reading=False, feature=False, furiType=defs.FURI_HIRA, readingTypes=(cabochadef.TYPE_KANJI,)):
    """
    @param  text  unicode
    @param* termEnabled  bool  whether query terms
    @param* type  bool  whether return type
    @param* group  bool   whether return group id
    @param* reading  bool   whether return yomigana
    @param* feature  bool   whether return feature
    @param* furiType  unicode
    @param* readingTypes  (int type) or [int type]
    @param* lougo  bool
    @yield  (unicode surface, int type, unicode yomigana or None, unicode feature, fmt or None)
    """
    parser = self.parser()
    if not parser:
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
    encoding = cabochadef.DICT_ENCODING
    feature2katana = fmt.getkata

    tree = parser.parse(text.encode(encoding))
    size = tree.token_size()
    newgroup = True
    group_id = 0

    for i in xrange(tree.token_size()):
      token = tree.token(i)
      surface = token.surface.decode(encoding, errors='ignore')

      if newgroup and token.chunk:
        group_id += 1
        newgroup = False
      newgroup = True

      char_type = cabochadef.surface_type(surface)

      if reading:
        yomigana = None
        #if node.char_type in (mecabdef.TYPE_VERB, mecabdef.TYPE_NOUN, mecabdef.TYPE_KATAGANA, mecabdef.TYPE_MODIFIER):
        f = None
        if feature:
          f = token.feature.decode(encoding, errors='ignore')
        if not readingTypes or char_type in readingTypes or char_type == cabochadef.TYPE_RUBY and wordtrans: # always translate katagana
          if wordtrans:
            if termEnabled:
              yomigana = tm.queryLatinWordTerms(surface)
            if not yomigana:
              yomigana = wordtrans(surface)
          if not yomigana: #and not lougo:
            if not feature:
              f = token.feature.decode(encoding, errors='ignore')
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
        if group:
          if not type and not feature:
            yield surface, yomigana, group_id
          elif type and not feature:
            yield surface, char_type, yomigana, group_id
          elif not type and feature:
            yield surface, yomigana, f, fmt, group_id
          else: # render all
            yield surface, char_type, yomigana, f, fmt, group_id
        else:
          if not type and not feature:
            yield surface, yomigana
          elif type and not feature:
            yield surface, char_type, yomigana
          elif not type and feature:
            yield surface, yomigana, f, fmt
          else: # render all
            yield surface, char_type, yomigana, f, fmt
      elif group:
        if not type and not feature:
          yield surface, group_id
        elif type and not feature: # and type
          yield surface, char_type, group_id
        elif not type and feature:
          f = token.feature.decode(encoding, errors='ignore')
          yield surface, f, fmt, group_id
        elif type and feature:
          f = token.feature.decode(encoding, errors='ignore')
          yield surface, char_type, f, fmt, group_id
        #else:
        #  assert False, "unreachable"
      else:
        if not type and not feature:
          yield surface
        elif type and not feature: # and type
          yield surface, char_type
        elif not type and feature:
          f = token.feature.decode(encoding, errors='ignore')
          yield surface, f, fmt
        elif type and feature:
          f = token.feature.decode(encoding, errors='ignore')
          yield surface, char_type, f, fmt
        #else:
        #  assert False, "unreachable"

# EOF
