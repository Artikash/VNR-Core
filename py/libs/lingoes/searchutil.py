# coding: utf8
# searchutil.py
# 1/16/2013 jichi


__all__ = ['lookup']

import bisect
from itertools import islice

#def lookupprefix(*args, **kwargs): # unicode, [(unicode fr,unicode to)] -> [pair]
#  return reversed(list(lookupprefix_r(*args, **kwargs)))

def lookupprefix(prefix, pairs): # unicode, [(unicode fr,unicode to)] -> return None or yield pair; reverse look up
  """Do binary search to get range of pairs matching the prefix.
  @param  prefix  unicode
  @param  pairs  [(unicode key, unicode value)]  sorted pairs of text and translation
  @return  None or yield matched pairs
  """
  c = PrefixCompares(prefix)
  rightindex = bisect.bisect_right(pairs, c) # int
  if rightindex > 0:
    i = rightindex - 2
    while i >= 0 and pairs[i][0].startswith(prefix):
      i  -= 1
    leftindex = i + 1
    #if leftIndex == rightIndex + 1
    #  yield pairs[leftindex]
    #else
    return islice(pairs, leftindex, rightindex)

# Only works for right search
# http://stackoverflow.com/questions/7380629/perform-a-binary-search-for-a-string-prefix-in-python
class PrefixCompares:
  def __init__(self, value):
    self.value = value

  def __lt__(self, other): # unicode, pair -> bool
    """@reimp"""
    return self.lt(other[0]) # only compare the first element

  def lt(self, other): # unicode, unicode -> bool
    """@reimp"""
    return self.value < other[:len(self.value)]

if __name__ == '__main__':
  pairs = [
    ('a', 1),
    ('ab', 2),
    ('b', 3),
    ('bc', 4),
    ('bcd', 5),
    ('bcdawef', 6),
    ('bd', 7),
    ('cd', 8),
    ('z', 9),
  ]

  for t in lookupprefix('bc', pairs):
    print t

# EOF
