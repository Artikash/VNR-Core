# coding: utf8
# 1/25/2015 jichi

import re
from sakurakit.skclass import memoized
import rc, textutil

@memoized
def manager(): return RenderManager()

class RenderManager:

  def renderMapping(self, *args, **kwargs):
    return ''.join(self._iterRenderMapping(*args, **kwargs))

  def _iterRenderMapping(self, text, language, mapping, charPerLine=100, rubySize=10, colorize=False, center=True):
    """
    @param  text  unicode
    @param  language  unicode
    @param  mapping  [(unicode source, unicode target)]
    @param* charPerLine  int  maximum number of characters per line
    @param* rubySize  float
    @param* colorsize  bool
    @param* center  bool
    @yield  unicode  HTML table
    """
    i = j = 0

    line = []
    lineCount = 0 # int  estimated line width, assume ruby has half width
    color = None
    #groupColor = '#ff8' # yellow

    COLORS = (
      'rgba(255,0,0,40)', # red
      'rgba(0,255,0,40)',  # green
      'rgba(255,255,0,40)', # yellow
      "rgba(0,0,255,40)", # blue
    )

    PADDING_FACTOR = 0.1 # 1/10
    #LATIN_RUBY_WIDTH = 0.33 # = 2/6
    #KANJI_RUBY_WIDTH = 0.55 # = 1/2
    # ruby size / surface size

    # http://www.rapidtables.com/web/color/silver-color.htm
    #rubyColor = None
    #rubyColor = None if colorize else 'lightgray'
    rubyColor = None if colorize else 'silver'
    #rubyColor = None if colorize else 'darkgray'
    #rubyColor = None if colorize else 'gray'

    roundRubySize = int(round(rubySize)) or 1
    #paddingSize = int(round(rubySize * PADDING_FACTOR)) or 1 if invertRuby else 0
    paddingSize = int(round(rubySize * PADDING_FACTOR))

    #rubyWidth = LATIN_RUBY_WIDTH if not romaji else KANJI_RUBY_WIDTH
    rubyWidth = 0.8 # around 4/6, make it larger to be safer

    mappingIndex = 0
    #for paragraph in text.split('\n'):
    # No longer split paragraph
    for group, (surface, ruby, mappingIndex) in enumerate(self._iterParseMapping(text, language, mapping, mappingIndex)):
      if group == 0 and mappingIndex >= len(mapping): # mapping failed
        yield text
        line = []
        break

      if ruby and len(ruby) == 1 and textutil.ispunct(ruby): # skip punctuation
        ruby = None

      if colorize:
        color = COLORS[group % len(COLORS)] #if group is not None else None
        #if ch == cabochadef.TYPE_KANJI:
        #  i += 1
        #  color = 'rgba(255,0,0,40)' if i % 2 else 'rgba(255,255,0,40)' # red or yellow
        #elif ch == cabochadef.TYPE_RUBY:
        #  j += 1
        #  #color = "rgba(0,255,0,40)" if j % 2 else "rgba(255,0,255,40)" # green or magenta
        #  color = "rgba(0,255,0,40)" if j % 2 else "rgba(0,0,255,40)" # green or blue
        #else:
        #  color = None

      width = max(len(textutil.remove_html_tags(surface)), len(ruby)*rubyWidth if ruby else 0)
      if width + lineCount <= charPerLine:
        pass
      elif line:
        yield rc.jinja_template('html/mapping').render({
          'tuples': line,
          'rubySize': roundRubySize,
          'rubyColor': rubyColor,
          'paddingSize': paddingSize,
          'center': center,
          #'groupColor': groupColor,
        })
        line = []
        lineCount = 0
      line.append((surface, ruby, color))
      lineCount += width

    if line:
      yield rc.jinja_template('html/mapping').render({
        'tuples': line,
        'rubySize': roundRubySize,
        'rubyColor': rubyColor,
        'paddingSize': paddingSize,
        'center': center,
        #'groupColor': groupColor,
      })

  def _fixHtmlTag(self, left, right):
    """Find unbalanced close tag on the right, and move it to the left
    @param  left  unicode
    @param  right  unicode
    @return  (unicode left, unicode right) not None
    """
    # Handle Shared Dictionary escaping here
    start = right.find('<')
    try:
      if start != -1 and right[start+1] == '/':
        stop = right.find('>')
        if stop > start:
          tag = right[start+2:stop]
          if tag:
            right = right[:start] + right[stop+1]
            left += "</%s>" % tag
    except IndexError: pass
    return left, right

  def _iterParseMapping(self, text, language, mapping, mappingIndex):
    """
    @param  text  unicode
    @param  language  unicode
    @param  mapping  [(unicode source, unicode target)]
    @param  mappingIndex  int
    @yield  (unicode surface, unicode ruby, int mappingIndex)
    """
    if mappingIndex < len(mapping):
      for i,(k,v) in enumerate(mapping): # instead of doing mapping[mappingIndex:]
        if i >= mappingIndex and not self._skipMapping(v):
          if not text:
            break
          if v in text:
            left, mid, text = text.partition(v)
            if left:
              left, text = self._fixHtmlTag(left, text)
              yield left, None, mappingIndex
            mappingIndex = i
            yield mid, k, mappingIndex
    if text:
      yield text, None, mappingIndex

  @staticmethod
  def _skipMapping(text):
    """
    @param  text  unicode
    @return  bool
    """
    return (
      len(text) == 1 and ord(text) < 128 # do not map single ascii char
      or text in ("//", "id", "json", "href", "type", "term", "source", "target")
    )

# EOF
