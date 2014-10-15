# coding: utf8
# midend.py
# 9/29/2014

__all__ = 'TreeBuilder', 'RuleBasedTranslator'

from tree import Node, Token
from rule import RuleMatchedList
from defs import ANY_LANGUAGE

# Parser

class TreeBuilder:

  language = 'ja'

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
      return Node(token=x, language=self.language if x.feature else ANY_LANGUAGE)
    if x:
      return Node(children=map(self._parse, x), language=self.language)
    else:
      return Node()

# Translator

class RuleBasedTranslator:

  def __init__(self, fr, to, rules=[]):
    self.fr = fr # str
    self.to = to # str
    self.rules = rules # [Rule]

    #rp = RuleParser()
    #self.rules = [rp.createRule(fr, to, *it) for it in (
    #  (u"顔", u"表情"),
    #  (u"(分から ない の 。)", u"不知道的。"),
    #  (u"どんな", u"怎样的"),
    #)]

  def translate(self, tree):
    """
    @param  tree  Node
    @return  Node
    """
    ret = self._translate(tree)
    if ret:
      self._updateLanguage(ret)
      return ret

  def _updateLanguage(self, node):
    """
    @param  node  Node
    """
    if not node.language:
      if node.token:
        node.language = self.fr
      elif node.children:
        language = None
        for it in node.children:
          self._updateLanguage(it)
          if not it.language:
            language = ''
          elif language is None:
            language = it.language
          elif language != it.language:
            if language == ANY_LANGUAGE:
              language = it.language
            elif it.language != ANY_LANGUAGE:
              language = ''
        if language:
          node.language = language

  def _translate(self, x):
    """
    @param  x  Node
    @return  Node
    """
    for rule in self.rules:
      x = rule.translate(x)
    if isinstance(x, Node) and x.children:
      for i,it in enumerate(x.children):
        r = self._translate(it)
        if r is not it:
          x.children[i] = r # TODO: Allow the logic to merge the list
    return x

# EOF

#  def _translate(self, x):
#    """
#    @param  x  Node or token or list
#    @return  Node not None
#    """
#    for rule in self.rules:
#      m = rule.matchSource(x)
#      if m:
#        if isinstance(m, RuleMatchedList):
#          return self._translateMatchedList(rule, m)
#        else:
#          return rule.createTarget()
#    if isinstance(x, list):
#      return Node(children=map(self._translate, x))
#    if isinstance(x, Token):
#      return Node(token=x)
#    if isinstance(x, Node):
#      if x.token:
#        return Node(token=x.token)
#      elif x.children:
#        return Node(children=map(self._translate, x.children))
#    return Node()
#
#  def _translateMatchedList(self, rule, m):
#    """
#    @param  rule  Rule
#    @param  m  RuleMatchedList
#    @return  Node
#    """
#    ret = Node(children=[])
#    for i in xrange(m.captureCount):
#      if not i:
#        start = m.captureStarts[i]
#        if start > 0:
#          left = m.nodes[:start]
#          if left:
#            left = self._translate(left)
#            if left.children:
#              left = left.children
#            ret.compactAppend(left)
#      ret.compactAppend(rule.createTarget())
#      if i == m.captureCount - 1:
#        stop = m.captureStops[i]
#        if stop < len(m.nodes):
#          right = m.nodes[stop:]
#          if right:
#            right = self._translate(right)
#            if right.children:
#              right = right.children
#            ret.compactAppend(right)
#    return ret
