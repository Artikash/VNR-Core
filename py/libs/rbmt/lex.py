# coding: utf8
# lex.py
# 8/12/2014 jichi
# Lexical utilities

from jptraits import jpchars
from unitraits import unichars

# Reading type
TYPE_KANJI = 'kanji'
TYPE_RUBY = 'ruby'
TYPE_PUNCT = 'punct'
TYPE_LATIN = 'latin'
#TYPE_NUM = 5

def _ordall2(text, start1, stop1, start2, stop2):
  """
  @param  text  unicode
  @param  start1  int
  @param  stop1  int
  @param  start2  int
  @param  stop2  int
  @return  bool
  """
  for c in text:
    u8 = ord(c)
    if not (u8 >= start1 and u8 <= stop1 or u8 >= start2 and u8 <= stop2):
      return False
  return True

def is_ruby_text(text):
  """
  @param  text  unicode
  @param  start  int
  @param  stop  int
  @return  bool
  """
  return _ordall2(text, unichars.ORD_HIRA_FIRST, unichars.ORD_HIRA_LAST, unichars.ORD_KATA_FIRST, unichars.ORD_KATA_LAST)

def is_latin_text(text):
  """
  @param  text  unicode
  @param  start  int
  @param  stop  int
  @return  bool
  """
  return _ordall2(text, 32, unichars.ORD_THIN_LAST, unichars.ORD_WIDE_FIRST, unichars.ORD_WIDE_LAST) # 32 is space

def text_type(text):
  """
  @param  text  unicode
  @return  int
  """
  if len(text) == 1 and text in jpchars.set_punc:
    return TYPE_PUNCT
  elif is_ruby_text(text):
    return TYPE_RUBY
  elif is_latin_text(text):
    return TYPE_LATIN
  else:
    return TYPE_KANJI

# EOF
