# coding: utf8
# jpchars.py
# 12/30/2012 jichi

# UTF-8

U8_HIRA_FIRST = 12353
U8_HIRA_LAST = 12438

U8_KATA_FIRST = 12449
U8_KATA_LAST = 12534

# TODO: Move these functions to other files
def u8_any(text, first, last):
  """
  @param  text  unicode
  @param  first  int
  @param  last  int
  @return  bool
  """
  for c in text:
    u8 = ord(c)
    if u8 >= first and u8 <= last:
      return True
  return False

def u8_all(text, first, last):
  """
  @param  text  unicode
  @param  first  int
  @param  last  int
  @return  bool
  """
  for c in text:
    u8 = ord(c)
    if u8 < first or u8 > last:
      return False
  return True

## Character traits ##

s_hira = u"\
がぎぐげござじずぜぞだぢづでどばびぶべぼぱぴぷぺぽ\
あいうえおかきくけこさしすせそたちつてと\
なにぬねのはひふへほまみむめもやゆよらりるれろ\
わをんぁぃぅぇぉゃゅょっ"

s_kata = u"\
ガギグゲゴザジズゼゾダヂヅデドバビブベボパピプペポ\
アイウエオカキクケコサシスセソタチツテト\
ナニヌネノハヒフヘホマミムメモヤユヨラリルレロ\
ワヲンァィゥェォャュョッ"

l_hira = list(s_hira)
l_kata = list(s_kata)

#re_hira = re.compile(r"[%s]" % s_hira)
#re_kata = re.compile(r"[%s]" % s_kata)

#re_hira_all = re.compile(r"[%s]+" % s_hira)
#re_kata_all = re.compile(r"[%s]+" % s_kata)

def anyhira(text): return u8_any(text, U8_HIRA_FIRST, U8_HIRA_LAST) #return bool(re_hira.search(text))
def anykata(text): return u8_any(text, U8_KATA_FIRST, U8_KATA_LAST) #return bool(re_kata.search(text))
def allhira(text): return u8_all(text, U8_HIRA_FIRST, U8_HIRA_LAST) #return bool(re_hira_all.match(text))
def allkata(text): return u8_all(text, U8_KATA_FIRST, U8_KATA_LAST) #return bool(re_kata_all.match(text))

#re_not_hira = re.compile(r"[^%s]" % s_hira)
#re_not_kata = re.compile(r"[^%s]" % s_kata)

import re

s_punc = u"\
、。？！…‥：・／\
―＝＄￥\
【】「」『』（）｛｝〈〉［］＜＞\
●◯■□◆◇★☆♥♡\
ー─～〜\
×\
　\
"

set_punc = frozenset(s_punc)
re_punc = re.compile(r"[%s]" % s_punc)

def containspunc(text):
  """
  @param  text  unicode
  @return  bool
  """
  return bool(re_punc.search(text))

if __name__ == '__main__':
  print containshira(u"ひらがな hello")

# EOF
