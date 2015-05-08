# coding: utf8
# hanzidict.py
# 5/7/2015 jichi

import os
from sakurakit.skdebug import dprint, dwarn
from sakurakit.skclass import memoized, memoizedproperty

@memoized
def manager(): return HanziDictionary()

class HanziDictionary:
  def __init__(self):
    self.__d = _HanziDictionary()

  def translateKanji(self, ch):
    """
    @param  ch  unicode
    @return  unicode or None
    """
    import dicts
    return dicts.kanjidic().lookup(ch)

  def translateRadical(self, ch):
    """
    @param  ch  unicode
    @return  unicode or None
    """
    return self.__d.radicals.get(ch)

  def lookupRadicals(self, ch):
    """
    @param  ch  unicode
    @return  list or None
    """
    return self.__d.decomp.lookup(ch)

  def renderRadicals(self, l):
    """
    @param  ch  unicode
    @return  list
    """
    ret = ''
    if l:
      for it in l:
        if isinstance(it, basestring):
          t = self.translateRadical(it) or self.translateKanji(it)
          if t:
            it += '{' + t + '}'
        if ret:
          ret += ', '
        if isinstance(it, basestring):
          ret += it
        else:
          ret += self.renderRadicals(it)
      ret = '(' + ret + ')'
    return ret

  def lookupRadicalString(self, ch):
    """
    @param  ch  unicode
    @return  unicode
    """
    rads = self.lookupRadicals(ch)
    return self.renderRadicals(rads) if rads else ''

class _HanziDictionary:
  def __init__(self):
    self._kanjidic = None

  @memoizedproperty
  def decomp(self):
    from hanzicomp.hanzidecomp import HanziDecomposer
    import config
    ret = HanziDecomposer()
    ret.loadFile(config.HANZI_DECOMP_DIC_PATH)
    dprint("size = %s" % ret.size())
    return ret

  @memoizedproperty
  def radicals(self):
    from hanzicomp import hanziradic
    import config
    return hanziradic.parse(config.HANZI_RADICAL_DIC_PATH) or {}

# EOF
