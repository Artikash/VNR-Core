# coding: utf8
# tree.py
# 8/10/2014

__all__ = 'Token', 'Node'

# Tree

class Token:
  def __init__(self, text='', feature=''):
    self.text = text # unicode
    self.feature = feature # unicode

  def unparse(self): return self.text
  def dump(self): return self.text

class Node(object): # tree node
  __slots__ = (
    'children',
    'parent',
    'token',
    'language',
  )

  def __init__(self, token=None, children=None, parent=None, language=''):
    self.children = children # [Node] or None
    self.parent = parent # Node
    self.token = token # token
    self.language = language # str

    if children:
      for it in children:
        it.parent = self

  def isEmpty(self): return not self.token and not self.children

  # Children

  def appendChild(self, node):
    """
    @param  node  Node
    """
    self.children.append(node)
    node.parent = self

  def prependChild(self, node):
    """
    @param  node  Node
    """
    self.children.insert(0, node)
    node.parent = self

  def appendChildren(self, l):
    """
    @param  l  [Node]
    """
    for it in l:
      self.appendChild(it)

  def prependChildren(self, l):
    """
    @param  l  [Node]
    """
    for it in l:
      self.prependChild(it)

  def compactAppend(self, x):
    """
    @param  node  Node or list
    """
    if isinstance(x, Node):
      if not x.isEmpty():
        self.appendChild(x)
    elif x:
      if len(x) == 1:
        self.appendChild(x[0])
      else:
        self.appendChildren(x)

  # Delete

  def clear(self):
    self.children = None
    self.parent = None
    self.token = None

  def clearTree(self): # recursively clear all children
    if self.children:
      for it in self.children:
        it.clearTree()
    self.clear()

  # Output

  def dumpTree(self): # recursively clear all children
    """
    @return  unicode
    """
    if self.token:
      return self.token.dump()
    elif self.children:
      return "(%s)" % ' '.join((it.dumpTree() for it in self.children))
    else:
      return ''

  def unparseTree(self, sep): # recursively clear all children
    """
    @param  sep  unicode
    @return  unicode
    """
    if self.token:
      return self.token.unparse()
    elif self.children:
      return sep.join((it.unparseTree(sep) for it in self.children))
    else:
      return ''

# EOF
