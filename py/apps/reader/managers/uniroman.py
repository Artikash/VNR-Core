# coding: utf8
# uniroman.py
# 1/5/2014 jichi
import re
from unidecode import unidecode
from unitraits import jpchars
import rc

__other_punct = u'《》“”‘’"\'，,? 　' # Chinese/English punctuations
_s_punct = jpchars.s_punct + __other_punct + '.()'
_re_punct = re.compile(ur"([%s\.\(\)])" % (''.join(jpchars.s_punct) + __other_punct))
def _splittext(text, language):
  """
  @param  text  unicode
  @param  text  str
  @yield  unicode
  """
  if language.startswith('zh'):
    return text
  else:
    return filter(bool, _re_punct.split(text)) # split by space and punctuation

def _toroman(text, language=''):
  """
  @param  text  unicode
  @param* language  str
  """
  if not text or not isinstance(text, unicode):
    return text
  text = unidecode(text)
  if language == 'ko':
    text = text.title()
  return text

def _iterrendertable(text, language, charPerLine=100, rubySize=10, colorize=False, center=True, invertRuby=False):
  """
  @param  text  unicode
  @param  language  unicode
  @param* charPerLine  int  maximum number of characters per line
  @param* rubySize  float
  @param* colorsize  bool
  @param* center  bool
  @param* invertRuby  bool
  @param* features  {unicode surface:(unicode feature, fmt)} or None
  @yield  unicode  HTML table
  """
  i = j = 0

  line = []
  lineCount = 0 # int  estimated line width, assume ruby has half width
  color = None
  groupColor = None if language.startswith('zh') else '#ff8' # yellow

  COLORS = (
    'rgba(255,0,0,40)', # red
    'rgba(0,255,0,40)',  # green
    'rgba(255,255,0,40)', # yellow
    "rgba(0,0,255,40)", # blue
  )

  PADDING_FACTOR = 0.3
  LATIN_YOMI_WIDTH = 0.33 # = 2/6
  KANJI_YOMI_WIDTH = 0.55 # = 1/2
  # yomi size / surface size
  yomiWidth = LATIN_YOMI_WIDTH #if else KANJI_YOMI_WIDTH

  for paragraph in text.split('\n'):
    for group, surface in enumerate(_splittext(paragraph, language)):
      if len(surface) == 1 and surface in _s_punct:
        group = None
        yomi = None
      else:
        yomi = _toroman(surface, language)
        if yomi == surface:
          yomi = None

      if colorize:
        color = COLORS[group % len(COLORS)] if group is not None else None
        #if ch == cabochadef.TYPE_KANJI:
        #  i += 1
        #  color = 'rgba(255,0,0,40)' if i % 2 else 'rgba(255,255,0,40)' # red or yellow
        #elif ch == cabochadef.TYPE_RUBY:
        #  j += 1
        #  #color = "rgba(0,255,0,40)" if j % 2 else "rgba(255,0,255,40)" # green or magenta
        #  color = "rgba(0,255,0,40)" if j % 2 else "rgba(0,0,255,40)" # green or blue
        #else:
        #  color = None

      width = max(len(surface), len(yomi)*yomiWidth if yomi else 0)
      if width + lineCount <= charPerLine:
        pass
      elif line:
        yield rc.jinja_template('html/furigana').render({
          'tuples': line,
          'rubySize': int(round(rubySize)) or 1,
          'paddingSize': int(round(rubySize * PADDING_FACTOR)) or 1 if invertRuby else 0,
          'center': center,
          'groupColor': groupColor,
        })
        line = []
        lineCount = 0
      if invertRuby and yomi:
        #if surface:
        #  surface = wide2thin(surface)
        if furiType == defs.FURI_ROMAJI and len(yomi) > 2:
          yomi = yomi.title()
        t = yomi, surface, color, group
      else:
        t = surface, yomi, color, group
      line.append(t)
      lineCount += width

  if line:
    yield rc.jinja_template('html/furigana').render({
      'tuples': line,
      'rubySize': int(round(rubySize)) or 1,
      'paddingSize': int(round(rubySize * PADDING_FACTOR)) or 1 if invertRuby else 0,
      'center': center,
      'groupColor': groupColor,
    })

def rendertable(text, language, **kwargs):
  return ''.join(_iterrendertable(text, language, **kwargs))

# EOF
