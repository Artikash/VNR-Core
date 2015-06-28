# coding: utf8
# richutil.py
# 6/27/2014 jichi

import re
import pyrich

_RP = pyrich.RichRubyParser()

#_rx_ruby = re.compile('{.+|.+}')
def containsRuby(text):
  """
  @param  text  unicode
  @return  bool
  """
  #return '{' in text and bool(_rx_ruby.search(text))
  if text:
    i = text.find('[')
    if i != -1:
      i = text.find('|', i)
      if i != -1:
        i = text.find(']', i)
        if i != -1:
          return True
  return False

def renderRubyToHtmlTable(text, width, rbFont, rtFont, cellSpace=1, wordWrap=True):
  """
  @param  text  unicode
  @param  width  int
  @param  rbFont  QFontMetrics
  @param  rtFont  QFontMetrics
  @param  cellSpace  int
  @param  wordWrap  bool
  """
  if not containsRuby(text):
    return text
  return _RP.renderToHtmlTable(text, width, rbFont, rtFont, cellSpace, wordWrap)

_rx_ruby1 = re.compile(r'\[(.+?)\|.+?\]')
def removeRuby(text):
  """
  @param  text  unicode
  @return  unicode
  """
  if not containsRuby(text):
    return text
  return _rx_ruby1.sub(r'\1', text)

_rx_ruby2 = re.compile(r'\[(.+?)\|(.+?)\]')
def renderRubyToPlainText(text):
  """
  @param  text  unicode
  @return  unicode
  """
  if not containsRuby(text):
    return text
  return _rx_ruby2.sub(r'\1(\2)', text)

# EOF
