# coding: utf8
# mt.py
# 9/29/2014

__all__ = [
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
  def __init__(self, text='', feature='', language=''):
    self.text = text # unicode
    self.feature = feature # unicode
    self.language = language

  def unparse(self): return self.text
  def dumps(self): return self.text

class Node: # tree node
  def __init__(self):
    self.children = None # [Node] or None
    self.parent = None # Node
    self.token = None # token

  def clear(self):
    self.children = None
    self.parent = None
    self.token = None

  def clearTree(self): # recursively clear all children
    if self.children:
      for it in self.children:
        it.clearTree()
    self.clear()

# Parser

_SENTENCE_RE = re.compile(ur"([。？！」\n])(?![。！？）」\n]|$)")
class Lexer:

  def splitSentences(self, text):
    """
    @param  text  unicode
    @return  [unicode]
    """
    return _SENTENCE_RE.sub(r"\1\n", text).split("\n")

class Parser:

  def __init__(self):
    import CaboCha
    self.cabocha = CaboCha.Parser()
    self.cabochaEncoding = 'utf8'

  def parse(self, text):
    """
    @param  unicode
    @return  tree
    """
    return self._parse(self._tokenize(text))

  def _tokenize(self, text):
    """Tokenize
    @param  unicode
    @return  [int link, [Token]]]  token stream
    """
    encoding = self.cabochaEncoding
    tree = self.cabocha.parse(text.encode(encoding))

    MAX_LINK = 32768 # use this value instead of -1
    link = 0

    phrase = [] # [Token]
    ret = [] # [int link, [Token]]
    for i in xrange(tree.token_size()):
      token = tree.token(i)

      surface = token.surface.decode(encoding, errors='ignore')
      feature = token.feature.decode(encoding, errors='ignore')
      word = Token(surface, feature=feature, language='ja')

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
    @return  tree
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
  def _dumps(self, x):
    """Debug only
    @param  x  Token or [[Token]...]
    @return  s
    """
    if isinstance(x, Token):
      return x.dumps()
    else:
      return "(%s)" % ' '.join(imap(self._dumps, x))

# Translator

class Translator:

  def translate(self, tree):
    """
    @param  tree  parse tree
    """
    return self._translateAny(tree)

  def _translateAny(self, tree):
    """
    @param  tree
    """
    if isinstance(tree, Token):
      return self._translateToken(tree)
    else:
      return self._translateList(tree)

  def _translateList(self, l):
    """
    @param  l  list
    """
    if not l:
      return []
    if len(l) == 1:
      return [self._translateAny(l[0])]
    return l

  def _translateToken(self, tok):
    """
    @param  tok  Token
    """
    return tok

# Unparser

class Unparser:

  def __init__(self):
    self.tokensep = ''

  def dumps(self, x): # debug print
    """
    @param  x  Token or [[Token]...]
    @return  s
    """
    if isinstance(x, Token):
      return x.dumps()
    else:
      return "(%s)" % ' '.join(imap(self.dumps, x))

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

  lex = Lexer()
  p = Parser()
  tr = Translator()
  up = Unparser()

  for s in lex.splitSentences(text):
    print "-- sentence --\n", s

    tree = p.parse(s)
    print "-- parse tree --\n", p._dumps(tree)

    tree = tr.translate(tree)
    print "-- translated tree --\n", up.dumps(tree)

    ret = up.unparse(tree)
    print "-- output --\n", ret

# EOF
