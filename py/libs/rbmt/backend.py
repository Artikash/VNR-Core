# coding: utf8
# backend.py
# 10/4/2014

__all__ = 'MachineTranslator',

class MachineTranslator:

  def __init__(self, fr, to, tr, escape=True, frsep="", tosep="", underline=True):
    self.fr = fr # str
    self.to = to # str
    self.tr = tr # function
    self.frsep = frsep # str
    self.tosep = tosep # str
    self.escape = escape # bool
    self.underline = underline # bool

  def translate(self, tree):
    """
    @param  tree  Node
    @return  unicode
    """
    ret = self._translateTree(tree)
    if self.underline and not self.tosep:
      ret = ret.replace("> ", ">")
      ret = ret.replace(" <", "<")
    return ret

  def _renderText(self, text):
    return self._underlineText(text) if self.underline else text
  def _underlineText(self, text):
    return '<span style="text-decoration:underline">%s</span>' % text

  def _translateText(self, text):
    """
    @param  text  unicode
    @return  unicode
    """
    #print text
    return self.tr(text, fr=self.fr, to=self.to)

  def _translateTree(self, x):
    """
    @param  x  Node
    @return  unicode
    """
    if x.language == self.to:
      return self._renderText(x.unparseTree(self.tosep))
    elif x.language == self.fr or x.token:
      return self._translateText(x.unparseTree(self.frsep))
    elif not x.language and x.children:
      #return self.unparsesep.join(imap(self._translateTree, x.children))
      if self.escape:
        return self._translateEscape(x)
      else:
        return self._translateText(x.unparseTree(self.frsep))
    else:
      dwarn("unreachable code path")
      return x.unparseTree(self.frsep)

  def _translateEscape(self, x):
    """
    @param  x  Node
    @return  unicode
    """
    esc = {}
    t = self._prepareEscape(x, esc)
    t = self._translateText(t)
    if not t:
      return ""
    elif not esc:
      return t
    else:
      return self._restoreEscape(t, esc)

  def _restoreEscape(self, t, esc):
    """
    @param  t  unicode
    @param  esc  dict
    @return  unicode
    """
    keys = esc.keys()
    keys.sort(key=len, reverse=True)
    for k in keys:
      t = t.replace(k, self._renderText(esc[k]))
    return t

  ESCAPE_KEY = "9%i.648"
  def _prepareEscape(self, x, esc):
    """
    @param  x  Node
    @param  esc  dict
    @return  unicode
    """
    if x.token:
      text = x.token.text
      return self._escapeText(text, esc) if x.language == self.to else text
    elif x.children:
      return self.frsep.join((self._prepareEscape(it, esc) for it in x.children))
    else:
      return ''

  def _escapeText(self, text, esc):
    """
    @param  text  unicode
    @param  esc  dict
    @return  unicode
    """
    key = self.ESCAPE_KEY % len(esc)
    esc[key] = text
    return key + ' '

# EOF
