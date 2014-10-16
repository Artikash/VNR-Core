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
  'PatternList',
  'Rule',
  'RuleBuilder',
)

import re
from collections import deque
from sakurakit.skdebug import dwarn

from tree import Node, Token, EMPTY_NODE

# Patterns

class PatternList(list):
  __slots__ = 'exactMatching',
  def __init__(self, *args, **kwargs):
    super(PatternList, self).__init__(*args, **kwargs)
    self.exactMatching = False # bool, whether has outer parenthesis

class PatternVariable(object):
  __slots__ = 'name', 'type'

  TYPE_SCALAR = 0   # such as $x
  TYPE_LIST = 1     # such as @x

  def __init__(self, name="", type=TYPE_SCALAR):
    self.name = name # str
    self.type = type # int

  def dump(self):
    """
    @return  str
    """
    return ('@' if self.type == self.TYPE_LIST else '$') + self.name

# Matchers

class MatchedObject(object):
  def __init__(self):
    self.variables = {} # {str name, Node or [Node]}

  def clearTree(self):
    if self.variables:
      for value in self.variables.itervalues():
        if isinstance(value, Node):
          value.clearTree()
        elif isinstance(value, list):
          for it in value:
            it.clearTree()
      self.variables = {}

class MatchedList(object): # for matching PatternList
  __slots__ = (
    'nodes',
    'captureCount', 'captureStarts', 'captureStops',
  )
  def __init__(self, nodes=[]):
    self.nodes = nodes # [Node]
    self.captureCount = 0 # int
    self.captureStarts = None # [int]
    self.captureStops = None # [int]  excluding

# Rule

class Rule(object):
  __slots__ = (
    'source', 'target',
    'sourceType', 'targetType',
    'sourceLanguage', 'targetLanguage',
  )

  TYPE_NONE = 0
  TYPE_STRING = 1   # str or unicode, single node, such as: あ
  TYPE_LIST = 2     # PatternList, normal tree, such as: (あ)
  TYPE_VAR = 3      # PatternVariable, scalar or list variable, such as $x or @x

  def __init__(self, sourceLanguage, targetLanguage, source, target):
    self.source = source # list or unicode
    self.target = target # list or unicode
    self.sourceType = self.typeName(source)
    self.targetType = self.typeName(target)
    self.sourceLanguage = sourceLanguage # str
    self.targetLanguage = targetLanguage # str

  @classmethod
  def typeName(cls, x):
    """
    @param  x  list or str or unicode
    @return  str
    """
    if not x:
      return cls.TYPE_NONE
    if isinstance(x, str) or isinstance(x, unicode):
      return cls.TYPE_STRING
    if isinstance(x, list):
      return cls.TYPE_LIST
    if isinstance(x, PatternVariable):
      return cls.TYPE_VAR

  def translate(self, x):
    """
    @param  x  Node
    @return  Node
    """
    return self._translate(x)

  def _translate(self, x):
    """
    @param  x  Node
    @return  Node
    """
    if not x or x.language != self.sourceLanguage:
      return x
    obj = MatchedObject()
    m = self.matchSource(x, obj)
    if m:
      x = self.updateTarget(x, m, obj)
    obj.clearTree()
    return x

    #  for c in x.token, x.children:
    #    if c:
    #      return self._translate(source, c)
    #  return

    #if isinstance(source, str) or isinstance(source, unicode):
    #  if isinstance(x, Token):
    #    return self._matchSourceString(source, x)
    #  return

    #if isinstance(x, list):
    #  if source.exactMatching or len(source) == len(x):
    #    return self._exactMatchSourceList(source, x)
    #  elif len(source) < len(x):
    #    return self._matchSourceList(source, x)

  def updateTarget(self, x, m, obj):
    """
    @param  x  Node
    @param* m  bool or MatchedList or None
    @param* obj  MatchedObject
    @return  Node
    """
    if m is not None:
      if isinstance(m, MatchedList):
        if m.captureCount:
          fragment = (self.sourceType == self.TYPE_LIST and self.targetType == self.TYPE_LIST
                      and self.source.exactMatching)
          for i in range(m.captureCount - 1, -1, -1):
            start = m.captureStarts[i]
            stop = m.captureStops[i]
            x.removeChildren(start, stop)
            if self.target:
              if fragment:
                x.insertChildren(start, self.createTargetList(obj))
              else:
                x.insertChild(start, self.createTarget(obj))
          return x

    y = self.createTarget(obj)
    x.assign(y, skip='parent')
    return x

  def matchSource(self, x, obj):
    """
    @param  x
    @param* obj  MatchedObject
    @return  bool or MatchedList or None
    """
    return bool(self.source) and self._matchSource(self.source, x, obj)

  def _matchSource(self, source, x, obj):
    """
    @param  source  list or unicode
    @param  x  Node or Token or list
    @param  m  MatchedObject
    @return  bool or MatchedList or None
    """
    if not x or x.language != self.sourceLanguage:
      return
    sourceType = self.typeName(source)
    if sourceType == self.TYPE_STRING:
      if x.token:
        return source == x.token.text
    elif sourceType == self.TYPE_LIST:
      if x.children:
        if source.exactMatching or len(source) == len(x.children):
          return self._exactMatchSourceList(source, x.children, obj)
        elif len(source) < len(x.children):
          return self._matchSourceList(source, x.children, obj)
    elif sourceType == self.TYPE_VAR:
      if x.token:
        obj.variables[source.name] = x.copyTree()
        return True

  def _exactMatchSourceList(self, source, nodes, *args):
    """
    @param  source  list
    @param  nodes  list
    @param* m  MatchedObject
    @return  bool
    """
    if len(source) == len(nodes):
      for s,c in zip(source, nodes):
        if not self._matchSource(s, c, *args):
          return False
      return True
    return False

  def _matchSourceList(self, source, nodes, *args):
    """Non-exact match.
    @param  source  list
    @param  nodes  list
    @param* m  MatchedObject
    @return  MatchedList or None
    """
    starts = []
    sourceIndex = 0
    for i,c in enumerate(nodes):
      s = source[sourceIndex]
      if sourceIndex == 0 and i + len(source) > len(nodes):
        break
      if not self._matchSource(s, c, *args):
        sourceIndex = 0
      elif sourceIndex == len(source) - 1:
        starts.append(i - sourceIndex)
        sourceIndex = 0
      else:
        sourceIndex += 1
    if starts:
      m = MatchedList(nodes)
      m.captureCount = len(starts)
      m.captureStarts = starts
      m.captureStops = [it + len(source) for it in starts]
      return m

  def createTarget(self, obj):
    """
    @param* obj  MatchedObject
    @return  Node
    """
    ret = self._createTarget(self.target, obj)
    ret.fragment = bool(self.targetType == self.TYPE_LIST) and not self.target.exactMatching
    return ret

  def createTargetList(self, obj):
    """
    @param  obj  MatchedObject
    @return  [Node]
    """
    if self.target:
      if self.targetType == self.TYPE_LIST:
        ret = [self._createTarget(it, obj) for it in self.target]
        return ret
      if self.targetType == self.TYPE_STRING or self.targetType == self.TYPE_VAR and self.target.type == PatternVariable.TYPE_SCALAR:
        return [self._createTarget(self.target, obj)]
    return []

  def _createTarget(self, target, obj):
    """
    @param  target  list or unicode
    @param* obj  MatchedObject
    @return  Node
    """
    if target:
      targetType = self.typeName(target)
      if targetType == self.TYPE_STRING:
        return Node(Token(target),
            language=self.targetLanguage)
      if targetType == self.TYPE_VAR:
        if target.type == PatternVariable.TYPE_SCALAR:
          value = obj.variables.get(target.name)
          if value is None:
            text = target.dump()
            return Node(Token(text), language=self.sourceLanguage)
          if isinstance(value, Node):
            return value.copyTree()
      if targetType == self.TYPE_LIST:
        return Node(children=[self._createTarget(it, obj) for it in target],
            language=self.targetLanguage)
    return EMPTY_NODE

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
