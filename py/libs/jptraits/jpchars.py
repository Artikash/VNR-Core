# coding: utf8
# jpchars.py
# 12/30/2012 jichi

from unitraits import unichars

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

def anyhira(text): return unichars.ordany(text, unichars.ORD_HIRA_FIRST, unichars.ORD_HIRA_LAST) #return bool(re_hira.search(text))
def anykata(text): return unichars.ordany(text, unichars.ORD_KATA_FIRST, unichars.ORD_KATA_LAST) #return bool(re_kata.search(text))
def allhira(text): return unichars.ordall(text, unichars.ORD_HIRA_FIRST, unichars.ORD_HIRA_LAST) #return bool(re_hira_all.match(text))
def allkata(text): return unichars.ordall(text, unichars.ORD_KATA_FIRST, unichars.ORD_KATA_LAST) #return bool(re_kata_all.match(text))

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
