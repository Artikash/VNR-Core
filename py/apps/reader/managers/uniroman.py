# coding: utf8
# uniroman.py
# 1/5/2014 jichi
import re
from unidecode import unidecode
from unitraits import jpchars
from hangulconv import hangulconv
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

def _iterparseruby(text, language, **kwargs):
  """
  @param  text  unicode
  @param  language  str
  @param* kwargs  passed to Korean
  @yield  (unicode surface, unicode yomi or None, int groupId or None)
  """
  return _iterparseruby_ko(text, **kwargs) if language == 'ko' else _iterparseruby_default(text, language)
  #return _iterparseruby_default(text, language)

def _iterparseruby_default(text, language):
  """
  @param  text  unicode
  @param  language  str
  @yield  (unicode surface, unicode yomi or None, int groupId or None)
  """
  for group, surface in enumerate(_splittext(text, language)):
    if len(surface) == 1 and surface in _s_punct:
      group = None
      yomi = None
    else:
      yomi = _toroman(surface, language)
      if yomi == surface:
        yomi = None
    yield surface, yomi, group

def _iterparseruby_ko(text, romajaRubyEnabled=True, hanjaRubyEnabled=True):
  """
  @param  text  unicode
  @param* romajaRubyEnabled  bool
  @param* hanjaRubyEnabled  bool
  @yield  (unicode surface, unicode yomi or None, int groupId or None)
  """
  for group, l in enumerate(hangulconv.to_hanja_list(text)):
    for surface,yomi in l:
      if len(surface) == 1 and surface in _s_punct:
        group = None
        yomi = None
      else:
        if hanjaRubyEnabled and romajaRubyEnabled:
          if not yomi:
            yomi = _toroman(surface, 'ko')
        elif romajaRubyEnabled:
          yomi = _toroman(surface, 'ko')
        elif hanjaRubyEnabled:
          pass
        else: # none is enabled
          yomi = None
        if yomi and yomi == surface:
          yomi = None
      if group is not None:
        group /= 2
      yield surface, yomi, group

def _iterrendertable(text, language, charPerLine=100, rubySize=10, colorize=False, center=True, invertRuby=False, **kwargs):
  """
  @param  text  unicode
  @param  language  unicode
  @param* charPerLine  int  maximum number of characters per line
  @param* rubySize  float
  @param* colorsize  bool
  @param* center  bool
  @param* invertRuby  bool
  @param* features  {unicode surface:(unicode feature, fmt)} or None
  @param* kwargs  passed to Korean
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

  roundRubySize = int(round(rubySize)) or 1
  paddingSize = int(round(rubySize * PADDING_FACTOR)) or 1 if invertRuby else 0
  if paddingSize and language == 'ko' and not kwargs.get('romajaRubyEnabled'):
    paddingSize = 0

  romaji = True # this value is not changed
  yomiWidth = LATIN_YOMI_WIDTH if not romaji else KANJI_YOMI_WIDTH

  for paragraph in text.split('\n'):
    for surface, yomi, group in _iterparseruby(paragraph, language, **kwargs):

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
          'rubySize': roundRubySize,
          'paddingSize': paddingSize,
          'center': center,
          'groupColor': groupColor,
        })
        line = []
        lineCount = 0
      if invertRuby and yomi:
        #if surface:
        #  surface = wide2thin(surface)
        #if romaji and len(yomi) > 2:
        #  yomi = yomi.title()
        t = yomi, surface, color, group
      else:
        t = surface, yomi, color, group
      line.append(t)
      lineCount += width

  if line:
    yield rc.jinja_template('html/furigana').render({
      'tuples': line,
      'rubySize': roundRubySize,
      'paddingSize': paddingSize,
      'center': center,
      'groupColor': groupColor,
    })

def rendertable(text, language, **kwargs):
  return ''.join(_iterrendertable(text, language, **kwargs))

# EOF
