# coding: utf8
# mt.py
# 9/29/2014

if __name__ == '__main__':
  import sys
  sys.path.append('..')

from itertools import imap
from sakurakit.skdebug import dwarn

class Token:
  def __init__(self, text='', feature=''):
    self.text = text # unicode
    self.feature = feature # unicode

  def unparse(self): return self.text

class Parser:
  encoding = 'utf8'

  def __init__(self):
    import CaboCha
    self.cabocha = CaboCha.Parser()

  def parse(self, text):
    """
    @param  unicode
    @return  [[Token]...]
    """
    ret = []
    encoding = self.encoding
    tree = self.cabocha.parse(text.encode(encoding))

    newgroup = False
    link = 0
    group = []
    for i in xrange(tree.token_size()):
      token = tree.token(i)

      surface = token.surface.decode(encoding, errors='ignore')
      feature = token.feature.decode(encoding, errors='ignore')

      if not i and token.chunk: # first element
        link = token.chunk.link

      if newgroup and token.chunk:
        ret.append((group, link))
        group = [Token(surface, feature)]
        link = token.chunk.link
        newgroup = False
      else:
        group.append(Token(surface, feature))
      newgroup = True

    if group:
      ret.append((group, link))
    return ret


class Unparser:

  def __init__(self):
    self.tokensep = ''

  def unparse(self, x):
    """
    @param  x  Token or [[Token]...]
    @return  unicode
    """
    if isinstance(x, Token):
      return x.unparse()
    else:
      return self.tokensep.join(imap(self.unparse, x))

if __name__ == '__main__':
  text = u"私のことを好きですか"

  print text

  p = Parser()
  l = p.parse(text)

  up = Unparser()
  ret = up.unparse(l)

  print ret

# EOF
