# coding: utf8
# richutil.py
# 6/27/2014 jichi

import pyrich

_RP = pyrich.RichRubyParser()

def renderruby(text, width, rbFont, rtFont, cellSpace=1, wordWrap=True):
  """
  @param  text  unicode
  @param  width  int
  @param  rbFont  QFontMetrics
  @param  rtFont  QFontMetrics
  @param  cellSpace  int
  @param  wordWrap  bool
  """
  return _RP.renderTable(text, width, rbFont, rtFont, cellSpace, wordWrap)

# EOF
