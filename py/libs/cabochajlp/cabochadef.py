# coding: utf8
# cabochadef.py
# 6/13/2014 jichi

from jptraits import jpchars

# ipadic encoding, either SHIFT-JIS or UTF-8
DICT_ENCODING = 'utf8'

# Reading type
TYPE_KANJI = 1
TYPE_RUBY = 2
TYPE_PUNCT = 3
TYPE_LATIN = 4
#TYPE_NUM = 5

def _surface_in_range2(text, start1, stop1, start2, stop2):
  """
  @param  text  unicode
  @param  start  int
  @param  stop  int
  @return  bool
  """
  for c in text:
    u8 = ord(c)
    if not (u8 >= start1 and u8 <= stop1 or u8 >= start2 and u8 <= stop2):
      return False
  return True

def is_ruby_surface(text):
  """
  @param  text  unicode
  @param  start  int
  @param  stop  int
  @return  bool
  """
  return _surface_in_range2(text, 12353, 12438, 12449, 12534)

def is_latin_surface(text):
  """
  @param  text  unicode
  @param  start  int
  @param  stop  int
  @return  bool
  """
  return _surface_in_range2(text, 32, 126, 65281, 65374) # 32 is space

def surface_type(text):
  """
  @param  text  unicode
  @return  int
  """
  if len(text) == 1 and text in jpchars.set_punc:
    return TYPE_PUNCT
  elif is_ruby_surface(text):
    return TYPE_RUBY
  elif is_latin_surface(text):
    return TYPE_LATIN
  else:
    return TYPE_KANJI

# EOF
