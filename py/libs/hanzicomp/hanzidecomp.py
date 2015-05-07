# coding: utf8
# hanzidecomp.py
# 5/6/2015 jichi

if __name__ == '__main__':
  import sys
  sys.path.append('..')

class HanziDecomposer(object):
  def __init__(self):
    self.__d = _HanziDecomposer()

  def isEmpty(self):
    """
    @return  bool
    """
    pass

  def size(self):
    """
    @return  bool
    """
    pass

  def loadFile(self, path):
    """
    @param  path  unicode
    @return  bool
    """
    pass

  def lookup(self, ch):
    """
    @param  ch  unicode
    @return  [unicode] or None
    """
    pass

class _HanziDecomposer:
  def __init__(self):
    pass

if __name__ == '__main__':
  path = '../../../../../Dictionaries/hanzi/cjk-decomp-0.4.0.txt'
  cc = HanziDecomposer()
  print cc.loadFile(path)
  print cc.isEmpty()
  print cc.size()

  t = u'密'
  print cc.lookup(t)

# EOF
