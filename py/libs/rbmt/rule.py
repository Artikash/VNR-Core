# coding: utf8
# rule.py
# 8/10/2014
#
# Rules
# The expression is mostly borrowed from PERL.
#
# - Regular expression requires back-slashed quotes, such as /This is a regex/.
# - String with spaces require quotes, such as "a string with space".
# - $x: single tree node
# - @x: a list of sibling tree nodes
# - $x:n Represent a noun, or use $x:v to represent a verb
#
# Examples:
# (@x の)   => (@x 的)
# (@x で)   => (在 @x)
#
# $x=/regex/:n   the :n means $x is a noun, similarly, $vt, $v
# $x=/regex/:n:v  either .noun or .verb
# It is just like CSS class selector, but the relation is or instead of and.
#
# @x の   => @x 的
# @x で   => 在 @x
#
# (@x=/.+/ の)  => (@x 的)
# (@x=/.+/ で)  => (在 @x)
#
# Parse tree:
# 私のことを好きですか
# (((私 の) (こと を)) (好きですか?))
#
# Non-node group:   (?:A B C)  the "(?:" means it is not groupped??

__all__ = (
  'PatternList', 'RuleMatchedList',
  'Rule',
  'RuleBuilder',
)

import re
from collections import deque
from sakurakit.skdebug import dwarn

from tree import Node, Token

class PatternList(list):
  __slots__ = 'exactMatching',
  def __init__(self, *args, **kwargs):
    super(PatternList, self).__init__(*args, **kwargs)
    self.exactMatching = False # bool

class RuleMatchedList(object):
  __slots__ = (
    'nodes',
    'captureCount', 'captureStarts', 'captureStops',
  )
  def __init__(self, nodes=[]):
    self.nodes = nodes # [Node]
    self.captureCount = 0 # int
    self.captureStarts = None # [int]
    self.captureStops = None # [int]  excluding

class Rule(object):
  __slots__ = (
    'source',
    'target',
    'sourceLanguage',
    'targetLanguage',
  )

  def __init__(self, sourceLanguage, targetLanguage, source, target):
    self.source = source # list or unicode
    self.target = target # list or unicode
    self.sourceLanguage = sourceLanguage # str
    self.targetLanguage = targetLanguage # str

  def matchSource(self, x):
    """
    @param  x  Node or token or list
    @return  bool or RuleMatchedList
    """
    if self.source:
      return self._matchSource(self.source, x)

  def _matchSource(self, source, x):
    """
    @param  source  list or unicode
    @param  x  Node or Token or list
    @return  bool
    """
    if not x:
      return
    if isinstance(x, Node):
      if x.language != self.sourceLanguage:
        return
      for c in x.token, x.children:
        if c:
          return self._matchSource(source, c)
      return

    if isinstance(source, str) or isinstance(source, unicode):
      if isinstance(x, Token):
        return self._matchSourceString(source, x)
      return

    if isinstance(x, list):
      if source.exactMatching or len(source) == len(x):
        return self._exactMatchSourceList(source, x)
      elif len(source) < len(x):
        return self._matchSourceList(source, x)

  @staticmethod
  def _matchSourceString(source, token):
    """
    @param  source  unicode
    @param  token  Token
    @return  bool
    """
    return source == token.text

  def _exactMatchSourceList(self, source, nodes):
    """
    @param  source  list
    @param  nodes  list
    @return  bool
    """
    if len(source) == len(nodes):
      for s,c in zip(source, nodes):
        if not self._matchSource(s, c):
          return False
      return True

  def _matchSourceList(self, source, nodes):
    """
    @param  source  list
    @param  nodes  list
    @return  RuleMatchedList or None
    """
    starts = []
    sourceIndex = 0
    for i,c in enumerate(nodes):
      s = source[sourceIndex]
      if sourceIndex == 0 and i + len(source) > len(nodes):
        break
      if not self._matchSource(s, c):
        sourceIndex = 0
      elif sourceIndex == len(source) - 1:
        starts.append(i - sourceIndex)
        sourceIndex = 0
      else:
        sourceIndex += 1
    if starts:
      m = RuleMatchedList(nodes)
      m.captureCount = len(starts)
      m.captureStarts = starts
      m.captureStops = [it + len(source) for it in starts]
      return m

  def createTarget(self, m=None):
    """
    @param* m  matched object
    @return  Node or None
    """
    return self._createTarget(self.target, m)

  def _createTarget(self, target, m=None):
    """
    @param  target  list or unicode
    @param* m  matched object
    @return  Node or None
    """
    if target:
      if isinstance(target, str) or isinstance(target, unicode):
        return Node(Token(target), language=self.targetLanguage)
      else:
        return map(self._createTarget, target)
    return Node() # Represent deleted node, TODO: skip empty node

class RuleBuilder:

  def createRule(self, sourceLanguage, targetLanguage, source, target):
    s = self.parse(source)
    if s:
      t = self.parse(target)
      return Rule(sourceLanguage, targetLanguage, s, t)

  def parse(self, text):
    """
    @param  text
    @return  list or unicode
    """
    if not text:
      return None

    if '(' not in text and ')' not in text:
      if ' ' not in text:
        return text
      l = text.split()
      return l[0] if len(l) == 1 else PatternList(l)

    return self._parse(self._tokenize(text))

  re_split = re.compile(r'([()])|\s')
  def _tokenize(self, text):
    """
    @param  text
    @return  list
    """
    return filter(bool, self.re_split.split(text))

  def _parse(self, l):
    """
    @param  l  list
    @return  list or unicode or None
    """
    if not l:
      return
    l = deque(l)
    s = [] # stack
    while l:
      x = l.popleft()
      if x != ')':
        s.append(x)
      else:
        # reduce until '('
        p = PatternList()
        p.exactMatching = True
        while s:
          x = s.pop()
          if x == '(':
            break
          p.insert(0, x)
        if x != '(':
          dwarn("error: unbalanced parenthesis")
          return
        if p:
          s.append(p)
    if not s:
      return
    # reduce until bol
    if '(' in s:
      dwarn("error: unbalanced parenthesis")
      return
    return s[0] if len(s) == 1 else PatternList(s)

# EOF
