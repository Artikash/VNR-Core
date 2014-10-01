# coding: utf8
# mt.py
# 9/29/2014

__all__ = [
  'Lexer',
  'Parser',
  'Transformer',
  'Unparser',
]

if __name__ == '__main__':
  import sys
  sys.path.append('..')

import re
from collections import OrderedDict
from itertools import imap
from sakurakit.skdebug import dwarn

# Tree

class Token:
  def __init__(self, text='', feature=''):
    self.text = text # unicode
    self.feature = feature # unicode

  def unparse(self): return self.text
  def dump(self): return self.text

class Node: # tree node
  unparsesep = ""

  def __init__(self, children=None, parent=None, token=None):
    self.children = children # [Node] or None
    self.parent = parent # Node
    self.token = token # token

    if children:
      for it in children:
        it.parent = self

  def isEmpty(self): return not self.token and not self.children

  def clear(self):
    self.children = None
    self.parent = None
    self.token = None

  def clearTree(self): # recursively clear all children
    if self.children:
      for it in self.children:
        it.clearTree()
    self.clear()

  def dumpTree(self): # recursively clear all children
    """
    @return  unicode
    """
    if self.token:
      return self.token.dump()
    elif self.children:
      return "(%s)" % ' '.join((it.dumpTree() for it in self.children))

  def unparseTree(self): # recursively clear all children
    """
    @return  unicode
    """
    if self.token:
      return self.token.unparse()
    elif self.children:
      return self.unparsesep.join((it.unparseTree() for it in self.children))

# Lexer

_SENTENCE_RE = re.compile(ur"([。？！」\n])(?![。！？）」\n]|$)")

class Lexer:

  def __init__(self):
    import CaboCha
    self.cabocha = CaboCha.Parser()
    self.cabochaEncoding = 'utf8'

    self.unparsesep = ''

  def splitSentences(self, text):
    """
    @param  text  unicode
    @return  [unicode]
    """
    return _SENTENCE_RE.sub(r"\1\n", text).split("\n")

  def parse(self, text):
    """
    @param  unicode
    @return  stream
    """
    return self._parse(self._tokenize(text))

  def _tokenize(self, text):
    """Tokenize
    @param  unicode
    @return  [int link, [Token]]]  token stream
    """
    encoding = self.cabochaEncoding
    stream = self.cabocha.parse(text.encode(encoding))

    MAX_LINK = 32768 # use this value instead of -1
    link = 0

    phrase = [] # [Token]
    ret = [] # [int link, [Token]]
    for i in xrange(stream.token_size()):
      token = stream.token(i)

      surface = token.surface.decode(encoding, errors='ignore')
      feature = token.feature.decode(encoding, errors='ignore')
      word = Token(surface, feature=feature)

      if token.chunk is not None:
        if phrase:
          ret.append((link, phrase))
          phrase = []
        link = token.chunk.link
        if link == -1:
          link = MAX_LINK
      phrase.append(word)

    if phrase:
      ret.append((link, phrase))
    return ret

  def _parse(self, phrases):
    """This is a recursive function.
    [@param  phrases [int link, [Token]]]  token stream
    @return  stream
    """
    if not phrases: # This should only happen at the first iteration
      return []
    elif len(phrases) == 1:
      return phrases[0][1]
    else: # len(phrases) > 2
      lastlink, lastphrase = phrases[-1]
      ret = [lastphrase]
      l = []
      for i in xrange(len(phrases) - 2, -1, -1):
        link, phrase = phrases[i]
        if lastlink > link:
          l.insert(0, (link, phrase))
        else:
          if l:
            ret.insert(0, self._parse(l))
          l = [(link, phrase)]
      ret.insert(0, self._parse(l))
      return ret

  # For cebug usage
  def dump(self, x):
    """
    @param  x  Token or [[Token]...]
    @return  s
    """
    if isinstance(x, Token):
      return x.dump()
    else:
      return "(%s)" % ' '.join(imap(self.dump, x))

  def unparse(self, x):
    """
    @param  x  Token or [[Token]...]
    @return  unicode
    """
    if isinstance(x, Token):
      return x.unparse()
    else:
      return self.unparsesep.join(imap(self.unparse, x))

# Parser
class Parser:

  def parse(self, stream):
    """
    @param  stream  Token or [[Token]...]
    @return  Node
    """
    return self._parse(stream)

  def _parse(self, x):
    """
    @param  x  Token or [[Token]...]
    @return  Node
    """
    if isinstance(x, Token):
      return Node(token=x)
    if x:
      return Node(children=map(self._parse, x))
    else:
      return Node()

# Translator

class Translator:

  def translate(self, tree):
    """
    @param  tree  Node
    @return  Node
    """
    return self._translate(tree)

  def _translate(self, tree):
    """
    @param  tree  Node
    @return  Node
    """
    if tree.token:
      return Node(token=tree.token)
    elif tree.children:
      return Node(token=tree.token, children=map(self._translate, tree.children))
    else:
      return Node()

# Unparser

class Unparser:

  def __init__(self):
    self.tokensep = ''

  def dump(self, x): # debug print
    """
    @param  x  Token or [[Token]...]
    @return  s
    """
    if isinstance(x, Token):
      return x.dump()
    else:
      return "(%s)" % ' '.join(imap(self.dump, x))

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
  # Example (link, surface) pairs:
  # 太郎は花子が読んでいる本を次郎に渡した
  # 5 太郎
  # none は
  # 2 花子
  # none が
  # 3 読ん
  # none で
  # none いる
  # 5 本
  # none を
  # 5 次郎
  # none に
  # -1 渡し
  # none た
  #text = u"太郎は花子が読んでいる本を次郎に渡した。"
  #text = u"立派な太郎は、可愛い花子が読んでいる本を立派な次郎に渡した。"
  #text = u"あたしは、日本人です。"
  #text = u"あたしは日本人です。"

  #text = u"【綾波レイ】「ごめんなさい。こう言う時どんな顔すればいいのか分からないの。」"
  #text = u"ごめんなさい。こう言う時どんな顔すればいいのか分からないの。"
  text = u"こう言う時どんな顔すればいいのか分からないの。"
  #text = u"こう言う時どんな顔すればいいのか分からないのか？"

  #text = u"私のことを好きですか？"
  #text = u"憎しみは憎しみしか生まない"

  #text = u"近未来の日本、多くの都市で大小の犯罪が蔓延。"
  #text = u"近未来の日本は、多くの都市で大小の犯罪が蔓延。"

  lexer = Lexer()
  parser = Parser()
  tr = Translator()
  up = Unparser()

  for s in lexer.splitSentences(text):
    print "-- sentence --\n", s

    stream = lexer.parse(s)
    t = lexer.unparse(stream)
    print "-- token stream == text ? %s  --\n" % (s == t), t
    print "-- token stream --\n", lexer.dump(stream)

    tree = parser.parse(stream)
    print "-- parse tree --\n", tree.dumpTree()

    print "-- unparse tree --\n", tree.unparseTree()

    newtree = tr.translate(tree)
    print "-- translated tree --\n", newtree.dumpTree()

    tree.clearTree()
    newtree.clearTree()

    #ret = up.unparse(tree)
    #print "-- output --\n", ret

# EOF
