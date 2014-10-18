# coding: utf8
# _cabochaman.py
# 6/14/2014 jichi
# See: cabocha/src/tree.cpp

__all__ = 'CaboChaParser',

import os
#from PySide.QtCore import QMutex
from sakurakit.skdebug import dprint, dwarn
from unitraits import jpchars
from cabochajlp import cabochadef, cabocharc
from mecabjlp import mecabfmt
import convutil, defs, dicts, termman

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
    #self.mutex = QMutex() # parse mutex
    self.enabled = False # bool
    self.dic = '' # str
    self.rcfile = '' # unicode
    self.fmt = mecabfmt.DEFAULT

    self.parsers = {} # {unicode dicfile:CaboCha.Parser}
    #self.parsersbydic = {} # unicode dic:CaboCha.Parser}

  def setenabled(self, v): self.enabled = v

  def setdic(self, v):
    if v != self.dic:
      self.dic = v

  def setfmt(self, v): self.fmt = v

  def setrcfile(self, v):
    if v != self.rcfile:
      self.rcfile = v
      cabocharc.setenvrc(v) if v else cabocharc.delenvrc()

  def parser(self):
    """
    @return  CaboCha.Parser or None
    """
    if self.enabled and self.dic and self.rcfile:
      ret = self.parsers.get(self.rcfile)
      if not ret and os.path.exists(self.rcfile):
        dprint("create new parser: dic = %s" % self.dic, self.rcfile)
        # posset value in rcfile does not work on Windows
        args = cabocharc.makeparserargs(posset=self.dic)
        ret = self.parsers[self.rcfile] = cabocharc.createparser(args)
        if not ret:
          dwarn("failed to create cabocha parser")
        #else:
        #  self.parsersbydic[self.dic] = ret
      return ret

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
    termEnabled = False # Force disabling terms for being slow
    if termEnabled:
      tm = termman.manager()
      termEnabled = tm.isEnabled()
    #if termEnabled:
    #  text = tm.applyWordTerms(text)
    if not fmt:
      fmt = self.fmt # mecabfmt
    if reading:
      wordtrans = _wordtrans if furiType == defs.FURI_TR else None
      katatrans = (convutil.kata2hira if furiType == defs.FURI_HIRA else
                   convutil.kata2hangul if furiType == defs.FURI_HANGUL else
                   convutil.kata2thai if furiType == defs.FURI_THAI else
                   #convutil.kata2kanji if furiType == defs.FURI_KANJI else
                   convutil.kata2romaji if furiType in (defs.FURI_ROMAJI, defs.FURI_TR) else
                   None)
      if termEnabled:
        furitrans = (convutil.kata2hira if furiType == defs.FURI_HIRA else
                     convutil.hira2kata if furiType == defs.FURI_KATA else
                     convutil.kana2hangul if furiType == defs.FURI_HANGUL else
                     convutil.kana2thai if furiType == defs.FURI_THAI else
                     convutil.kana2romaji)
      if furiType in (defs.FURI_ROMAJI, defs.FURI_HANGUL, defs.FURI_THAI): #, defs.FURI_KANJI
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
        if not readingTypes or char_type in readingTypes or wordtrans and char_type == cabochadef.TYPE_RUBY and jpchars.allkata(surface): # always translate katagana or wordtrans
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
                        yomigana = convutil.wide2thin(convutil.kata2romaji(yomigana))
                        if yomigana == surface:
                          yomigana = None
                          unknownYomi = False
                      elif furiType == defs.FURI_HANGUL:
                        yomigana = convutil.kata2hangul(yomigana)
                      #elif furiType == defs.FURI_KANJI:
                      #  yomigana = convutil.kata2kanji(yomigana)
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
