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
def allkanji(text): return unichars.ordall(text, unichars.ORD_KANJI_FIRST, unichars.ORD_KANJI_LAST)
def allhira(text): return unichars.ordall(text, unichars.ORD_HIRA_FIRST, unichars.ORD_HIRA_LAST) #return bool(re_hira_all.match(text))
def allkata(text): return unichars.ordall(text, unichars.ORD_KATA_FIRST, unichars.ORD_KATA_LAST) #return bool(re_kata_all.match(text))
def anykanji(text): return unichars.ordany(text, unichars.ORD_KANJI_FIRST, unichars.ORD_KANJI_LAST)

def anykana(text): return anyhira(text) or anykata(text)
def allkana(text):
  for it in text:
    c = ord(it)
    if not (unichars.ORD_KATA_FIRST <= c and c <= unichars.ORD_KATA_LAST or
            unichars.ORD_HIRA_FIRST <= c and c <= unichars.ORD_HIRA_LAST):
      return False
  return True

def ishirachar(ch): return len(ch) == 1 and unichars.ORD_HIRA_FIRST <= ord(ch) and ord(ch) <= unichars.ORD_HIRA_LAST
def iskatachar(ch): return len(ch) == 1 and unichars.ORD_KATA_FIRST <= ord(ch) and ord(ch) <= unichars.ORD_KATA_LAST
def iskanjichar(ch): return len(ch) == 1 and unichars.ORD_KANJI_FIRST <= ord(ch) and ord(ch) <= unichars.ORD_KANJI_LAST
def iskanachar(ch): return len(ch) == 1 and (
    unichars.ORD_KATA_FIRST <= ord(ch) and ord(ch) <= unichars.ORD_KATA_LAST or
    unichars.ORD_HIRA_FIRST <= ord(ch) and ord(ch) <= unichars.ORD_HIRA_LAST)

#re_not_hira = re.compile(r"[^%s]" % s_hira)
#re_not_kata = re.compile(r"[^%s]" % s_kata)

lquotes = u'([<{（｛「『【＜《‘“'
rquotes = u')]>}）｝」』】＞》’”'

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

CAT_DIGIT = 'digit'
CAT_ALPHA = 'alpha'
CAT_HIRA = 'hira'
CAT_KATA = 'kata'
CAT_KANJI = 'kanji'
def getcat(ch):
  """
  @param  ch  str
  @return  str
  """
  if ch and len(ch) == 1:
    v = ord(ch)
    if v >= unichars.ORD_KANJI_FIRST and v <= unichars.ORD_KANJI_LAST:
      return CAT_KANJI
    if v >= unichars.ORD_HIRA_FIRST and v <= unichars.ORD_HIRA_LAST:
      return CAT_HIRA
    if v >= unichars.ORD_KATA_FIRST and v <= unichars.ORD_KATA_LAST:
      return CAT_KATA
    if v >= unichars.ORD_DIGIT_FIRST and v <= unichars.ORD_DIGIT_LAST:
      return CAT_DIGIT
    if v >= unichars.ORD_IALPHA_FIRST and v <= unichars.ORD_IALPHA_LAST or v >= unichars.ORD_UALPHA_FIRST and v <= unichars.ORD_UALPHA_LAST:
      return CAT_ALPHA
  return ''

# Alpha, Number, Punctuation, Space
re_any_anp = re.compile(ur"[a-zA-Z0-9%s]" % (s_punct + '.'))
re_all_anp = re.compile(ur"^[a-zA-Z0-9%s]+$" % (s_punct + '.'))
re_any_anps = re.compile(ur"[a-zA-Z0-9%s]" % (s_punct + '.'))
re_all_anps = re.compile(ur"^(?:[a-zA-Z0-9%s]|\s)+$" % (s_punct + '.'))
def anyanp(text): return bool(re_any_anp.search(text))
def allanp(text): return bool(re_all_anp.match(text))
def anyanps(text): return bool(re_any_anps.search(text))
def allanps(text): return bool(re_all_anps.match(text))


from sakurakit import skstr
WIDE2THIN_PUNCT = {
  u'‘': " '",
  u'’': "' ",
  u'“': ' "',
  u'”': '" ',
  u'。': '. ',
  u'、': ', ',
  u'？': '? ',
  u'！': '! ',
  u'〜': '~',
  u'／': '/',
  u'＝': '=',
  u'ー': '-',
  u'　': ' ',
}
wide2thin_punct = skstr.multireplacer(WIDE2THIN_PUNCT) # unicode -> unicode

# EOF
