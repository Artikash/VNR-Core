# coding: utf8
# jpchars.py
# 12/30/2012 jichi

import re

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

re_hira = re.compile(r"[%s]" % s_hira)
re_kata = re.compile(r"[%s]" % s_kata)

re_hira_all = re.compile(r"[%s]+" % s_hira)
re_kata_all = re.compile(r"[%s]+" % s_kata)

def _contains_u8(text, start, stop):
  """
  @param  text  unicode
  @param  start  int
  @param  stop  int
  @return  bool
  """
  for c in text:
    u8 = ord(c)
    if u8 >= start and u8 <= stop:
      return True
  return False

def containshira(text): return _contains_u8(text, 12353, 12438) #return bool(re_hira.search(text))
def containskata(text): return _contains_u8(text, 12449, 12534) #return bool(re_kata.search(text))

#re_not_hira = re.compile(r"[^%s]" % s_hira)
#re_not_kata = re.compile(r"[^%s]" % s_kata)

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
