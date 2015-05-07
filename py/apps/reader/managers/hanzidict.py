# coding: utf8
# hanzidict.py
# 5/7/2015 jichi

from sakurakit.skdebug import dprint
from sakurakit.skclass import memoized, memoizedproperty

@memoized
def manager(): return HanziDictionary()

class HanziDictionary:
  def __init__(self):
    pass

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
        if ret:
          ret += ','
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

  @memoizedproperty
  def decomp(self):
    from hanzicomp.hanzidecomp import HanziDecomposer
    import config
    ret = HanziDecomp()
    ret.loadFile(config.HANZI_RADICAL_DIC_PATH)
    dprint("size = %s" % ret.size())
    return ret

# EOF
