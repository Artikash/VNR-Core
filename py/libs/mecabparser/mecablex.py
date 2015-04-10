# coding: utf8
# mecablex.py
# 2/24/2014 jichi

import re
from unitraits import jpchars

ORD_THIN_DIGIT_FIRST = ord('0')
ORD_THIN_DIGIT_LAST = ord('9')
ORD_WIDE_DIGIT_FIRST = ord(u'０')
ORD_WIDE_DIGIT_LAST = ord(u'９')
def alldigit(text):
  """
  @param  text  unicode
  @return  bool
  """
  if not text:
    return False
  for ch in text:
    ch = ord(ch)
    if not (ch >= ORD_THIN_DIGIT_FIRST and ch <= ORD_THIN_DIGIT_LAST or ch >= ORD_WIDE_DIGIT_FIRST and ch <= ORD_WIDE_DIGIT_LAST):
      return False
  return True

SURFACE_ANY = 0
SURFACE_PUNCT = 1
SURFACE_KANJI = 2
SURFACE_NUMBER = 3
SURFACE_KANA = 4

def getsurfacetype(text): # unicode -> int
  if jpchars.allpunct(text):
    return SURFACE_PUNCT
  if jpchars.anykanji(text):
    return SURFACE_KANJI
  if alldigit(text):
    return SURFACE_NUMBER
  if jpchars.anykana(text):
    return SURFACE_KANA
  return SURFACE_ANY

# EOF

