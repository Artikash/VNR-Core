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

__all__ = 'Rule', 'RuleBuilder'

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

  @property
  def lengthFixed(self): # -> bool
    for it in self:
      if isinstance(it, PatternVariable) and it.type == PatternVariable.TYPE_LIST:
        return False
    return True

class PatternVariable(object):
  __slots__ = 'name', 'type'

  TYPE_SCALAR = 0   # such as $x
  TYPE_LIST = 1     # such as @x

  SIGN_SCALAR = '$'
  SIGN_LIST = '@'

  def __init__(self, name="", type=TYPE_SCALAR):
    self.name = name # str
    self.type = type # int

  def dump(self):
    """
    @return  str
    """
    return (self.SIGN_LIST if self.type == self.TYPE_LIST else self.SIGN_SCALAR) + self.name

  @classmethod
  def signType(cls, c): # str -> int or None
    if c == cls.SIGN_SCALAR:
      return cls.TYPE_SCALAR
    if c == cls.SIGN_LIST:
      return cls.TYPE_LIST

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
    self.captureVariables = [] # [{str name:list or Node}]

  def clearTree(self):
    if self.captureVariables:
      for d in self.captureVariables:
        if d:
          for value in d.itervalues():
            if isinstance(value, Node):
              value.clearTree()
            elif isinstance(value, list):
              for it in value:
                it.clearTree()
      self.captureVariables = []

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
    m = self.matchSource(x, obj.variables)
    if m:
      x = self.updateTarget(x, m, obj.variables)
      if isinstance(m, MatchedList):
        m.clearTree()
    obj.clearTree()
    return x

  def updateTarget(self, x, m, vars):
    """
    @param  x  Node
    @param* m  bool or MatchedList or None
    @param* vars  {str name:list or Node}
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
              v = m.captureVariables[i] if m.captureVariables else vars
              if fragment:
                x.insertChildren(start, self.createTargetList(v))
              else:
                x.insertChild(start, self.createTarget(v))
          return x

    y = self.createTarget(vars)
    x.assign(y, skip='parent')
    return x

  def matchSource(self, x, vars):
    """
    @param  x
    @param  vars  {str name:list or Node}
    @return  bool or MatchedList or None
    """
    return bool(self.source) and self._matchSource(self.source, x, vars)

  def _matchSource(self, source, x, vars):
    """
    @param  source  list or unicode
    @param  x  Node
    @param  vars  {str name:list or Node}
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
        if source.lengthFixed:
          if source.exactMatching or len(source) == len(x.children):
            return self._exactMatchFixedSourceList(source, x.children, vars)
          elif len(source) < len(x.children):
            return self._matchFixedSourceList(source, x.children, vars)
        else:
          if source.exactMatching:
            return self._exactMatchVariantSourceList(source, x.children, vars)
          else:
            return self._matchVariantSourceList(source, x.children, vars)
    elif sourceType == self.TYPE_VAR:
      if source.type == PatternVariable.TYPE_SCALAR:
        if x.token:
          value = vars.get(source.name)
          if value:
            dwarn("warning: duplicate variable definition: %s" % source.name)
            value.clearTree()
          vars[source.name] = x.copyTree()
          return True
      elif source.type == PatternVariable.TYPE_LIST:
        if x.children:
          value = vars.get(source.name)
          if value:
            dwarn("warning: duplicate variable definition: %s" % source.name)
            value.clearTree()
          vars[source.name] = [it.copyTree() for it in x.children]
          return True

  def _exactMatchFixedSourceList(self, source, nodes, *args):
    """Exact match list with fixed length.
    @param  source  list
    @param  nodes  list
    @param* vars  {str name:list or Node}
    @return  bool
    """
    if len(source) == len(nodes):
      for s,c in zip(source, nodes):
        if not self._matchSource(s, c, *args):
          return False
      return True
    return False

  def _matchFixedSourceList(self, source, nodes, *args):
    """Non-exact match list with fixed length.
    @param  source  list
    @param  nodes  list
    @param* vars  {str name:list or Node}
    @return  MatchedList or None
    """
    starts = []
    sourceIndex = 0
    for i,n in enumerate(nodes):
      if sourceIndex == 0 and i + len(source) > len(nodes):
        break
      s = source[sourceIndex]
      if not self._matchSource(s, n, *args):
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

  def _exactMatchVariantSourceList(self, source, nodes, vars):
    """Exact match list with variant length.
    @param  source  list
    @param  nodes  list
    @param  vars  {str name:list or Node}
    @return  bool
    """
    if not nodes or not source or len(source) > len(nodes):
      return False

    nodes = deque(nodes)
    source = deque(source)

    # TODO: match allow backtrack
    while source and nodes:
      # right to left
      while source and nodes and not (
          isinstance(source[-1], PatternVariable) and source[-1].type == PatternVariable.TYPE_LIST):
        s = source.pop()
        n = nodes.pop()
        if not self._matchSource(s, n, vars):
          return False
      # left to right
      while source and nodes and not (
          isinstance(source[0], PatternVariable) and source[0].type == PatternVariable.TYPE_LIST):
        s = source.popleft()
        n = nodes.popleft()
        if not self._matchSource(s, n, vars):
          return False
      if (source
          and isinstance(source[0], PatternVariable) and source[0].type == PatternVariable.TYPE_LIST
          and isinstance(source[-1], PatternVariable) and source[-1].type == PatternVariable.TYPE_LIST):
        if not nodes:
          return False
        s = source.pop()
        if source:
          return False
        value = vars.get(s.name)
        if value:
          dwarn("warning: duplicate variable definition: %s" % s.name)
          value.clearTree()
        vars[s.name] = [it.copyTree() for it in nodes]
        return True
    return not source and not nodes

  def _matchVariantSourceList(self, source, nodes, *args):
    """Non-exact match list with variant length.
    @param  source  list
    @param  nodes  list
    @param* vars  {str name:list or Node}
    @return  MatchedList or None
    """
    if not nodes or not source or len(source) > len(nodes):
      return

    starts = []
    varslist = []
    sourceIndex = 0
    vars = {}
    for i,n in enumerate(nodes):
      if sourceIndex == 0 and i + len(source) > len(nodes):
        break
      s = source[sourceIndex]
      # CHECKPOINT: not implemented
      if not self._matchSource(s, n, vars):
        sourceIndex = 0
        vars.clear()
      elif sourceIndex == len(source) - 1:
        starts.append(i - sourceIndex)
        sourceIndex = 0
        varslist.append(vars)
        vars = {}
      else:
        sourceIndex += 1
    if starts:
      m = MatchedList(nodes)
      m.captureCount = len(starts)
      m.captureStarts = starts
      m.captureStops = [it + len(source) for it in starts]
      m.captureVariables = varslist
      return m

  def createTarget(self, vars):
    """
    @param  vars  {str name:list or Node}
    @return  Node
    """
    ret = self._createTarget(self.target, vars)
    ret.fragment = bool(self.targetType == self.TYPE_LIST) and not self.target.exactMatching
    return ret

  def createTargetList(self, vars):
    """
    @param  vars  {str name:list or Node}
    @return  [Node]
    """
    if self.target:
      islist = self.targetType == self.TYPE_LIST
      if islist:
        return self._createTargetList(self.target, vars)
      isscalar = (
          self.targetType == self.TYPE_STRING
          or self.targetType == self.TYPE_VAR and self.target.type == PatternVariable.TYPE_SCALAR)
      if isscalar:
        return [self._createTarget(self.target, vars)]
    return []

  def _createTargetList(self, target, vars):
    """
    @param  target  list
    @param  vars  {str name:list or Node}
    @return  [Node]
    """
    #return [self._createTarget(it, vars) for it in target]
    ret = []
    for it in target:
      node = self._createTarget(it, vars)
      if not node.isEmpty():
        if node.children and self.typeName(it) == self.TYPE_VAR and it.type == PatternVariable.TYPE_LIST:
          ret.extend(node.children)
        else:
          ret.append(node)
    return ret

  def _createTarget(self, target, vars):
    """
    @param  target  list or unicode
    @param  vars  {str name:list or Node}
    @return  Node
    """
    if target:
      targetType = self.typeName(target)
      if targetType == self.TYPE_STRING:
        return Node(Token(target),
            language=self.targetLanguage)
      if targetType == self.TYPE_LIST:
        return Node(children=self._createTargetList(target, vars),
            language=self.targetLanguage)
      if targetType == self.TYPE_VAR:
        value = vars.get(target.name)
        if value is None:
          text = target.dump()
          return Node(Token(text), language=self.sourceLanguage)
        if target.type == PatternVariable.TYPE_SCALAR:
          if isinstance(value, Node):
            return value.copyTree()
        elif target.type == PatternVariable.TYPE_LIST:
          if isinstance(value, list):
            return Node(children=[it.copyTree() for it in value],
                language=self.sourceLanguage)
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

    if not self._containsSpecialCharacter(text):
      if ' ' not in text:
        return text
      l = text.split()
      return l[0] if len(l) == 1 else PatternList(l)

    return self._parse(self._tokenize(text))

  SPECIAL_CHARS = '()$@'

  def _containsSpecialCharacter(self, text):
    """
    @param  text  unicode
    @return  bool
    """
    for c in text:
      if c in self.SPECIAL_CHARS:
        return True
    return False

  re_split = re.compile(r'([%s])|\s' % SPECIAL_CHARS)
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
      if x in ('$', '@'):
        if not l:
          dwarn("error: dangle character: %s" % x)
          return
        y = l.popleft()
        if y in self.SPECIAL_CHARS:
          dwarn("error: using special characters as variable name is not allowed: %s" % y)
          return
        s.append(PatternVariable(y,
            PatternVariable.signType(x)))
      elif x != ')':
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
