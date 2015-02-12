# coding: utf8
# jpchars.py
# 12/30/2012 jichi

import unichars

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

def allkana(text): return allhira(text) or allkata(text)
def anykana(text): return anyhira(text) or anykata(text)

def ishirachar(ch): return len(ch) == 1 and unichars.ORD_HIRA_FIRST <= ord(ch) and ord(ch) <= unichars.ORD_HIRA_LAST
def iskatachar(ch): return len(ch) == 1 and unichars.ORD_KATA_FIRST <= ord(ch) and ord(ch) <= unichars.ORD_KATA_LAST
def iskanachar(ch): return len(ch) == 1 and (
    unichars.ORD_KATA_FIRST <= ord(ch) and ord(ch) <= unichars.ORD_KATA_LAST or
    unichars.ORD_HIRA_FIRST <= ord(ch) and ord(ch) <= unichars.ORD_HIRA_LAST)

#re_not_hira = re.compile(r"[^%s]" % s_hira)
#re_not_kata = re.compile(r"[^%s]" % s_kata)

import re

s_kanji = ur"一-龠"

s_punct = u"\
、。？！…‥：・／\
―＝＄￥\
【】「」『』（）｛｝〈〉［］＜＞””\
●◯■□◆◇★☆♥♡\
ー─～〜\
×\
　\
，《》‘’“”\
"

set_punct = frozenset(s_punct)
re_punct = re.compile(r"[%s]" % ''.join(s_punct))
re_all_punct = re.compile(r"^[%s]+$" % ''.join(s_punct))
re_not_punct = re.compile(r"[^%s]" % ''.join(s_punct))

def anypunct(text):
  """
  @param  text  unicode
  @return  bool
  """
  return bool(re_punct.search(text))

def notallpunct(text):
  """
  @param  text  unicode
  @return  bool
  """
  return bool(re_not_punct.search(text))

def allpunct(text):
  """
  @param  text  unicode
  @return  bool
  """
  return bool(re_all_punct.match(text))

def ispunct(ch):
  """
  @param  ch  unicode
  @return  bool
  """
  return ch in s_punct

# EOF
