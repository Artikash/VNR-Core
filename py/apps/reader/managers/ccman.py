# coding: utf8
# ccman.py
# 10/19/2014 jichi
from sakurakit.skclass import memoized
from sakurakit.skdebug import dprint
from opencc import opencc

@memoized
def manager(): return ChineseConversionManager()

class _ChineseConversionManager: pass
class ChineseConversionManager:
  def __init__(self):
    d = self.__d = _ChineseConversionManager()
    d.variant = '' # str  Among '', 'ja', 'tw', 'hk'
    d.variantConvert = None # lambda: unicode -> unicode

  def variant(self): return self.__d.variant # -> str
  def setVariant(self, v): # str ->
    d = self.__d
    if d.variant != v:
      dprint(v)
      d.variant = v
      if v == 'tw':
        d.variantConvert = opencc.zht2tw
      elif v == 'hk':
        d.variantConvert = opencc.zht2hk
      elif v == 'ja':
        d.variantConvert = opencc.zht2ja
      else:
        d.variantConvert = None

  def convert(self, text):
    """
    @param  text  unicode
    @return  unicode
    """
    d = self.__d
    ret = opencc.zhs2zht(text)
    if d.variantConvert:
      ret = d.variantConvert(text)
    return ret

def zhs2zht(text): return manager().convert(text)

# EOF
