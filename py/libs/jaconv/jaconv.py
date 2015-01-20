# coding: utf8
# jaconv.py
# 12/30/2012 jichi

if __name__ == '__main__':
  import sys
  sys.path.append("..")

import re
from itertools import chain, imap, izip
from sakurakit.skstr import multireplacer
import jadef

_TYPE_KANA = 0
_TYPE_HIRA = 1
_TYPE_KATA = 2

_TYPE_KANA_N = 3
_TYPE_HIRA_N = 4
_TYPE_KATA_N = 5

_TYPE_ROMAJI = 6
_TYPE_HANGUL = 7
_TYPE_THAI = 8

_TYPE_ROMAJI_RU = 10

_DEFS = { # {int type:unicode}
  _TYPE_HIRA: jadef.HIRA,
  _TYPE_KATA: jadef.KATA,
  _TYPE_HIRA_N: jadef.HIRA + jadef.HIRA_N,
  _TYPE_KATA_N: jadef.KATA + jadef.KATA_N,
  _TYPE_ROMAJI: jadef.ROMAJI,
  _TYPE_ROMAJI_RU: jadef.ROMAJI_RU,
  _TYPE_HANGUL: jadef.HANGUL + jadef.HANGUL_N,
  _TYPE_THAI: jadef.THAI,
}

def _makeconverter(fr, to):
  """
  @param  fr  int
  @param  to  int
  @return  function or None
  """
  if fr == _TYPE_KANA:
    olist = _DEFS[to].split()
    olist = chain(olist, olist)
    ilist = chain(_DEFS[_TYPE_HIRA].split(), _DEFS[_TYPE_KATA].split())
  elif fr == _TYPE_KANA_N:
    olist = _DEFS[to].split()
    olist = chain(olist, olist)
    ilist = chain(_DEFS[_TYPE_HIRA_N].split(), _DEFS[_TYPE_KATA_N].split())
  else:
    olist = _DEFS[to].split()
    ilist = _DEFS[fr].split()
  table = dict(izip(ilist, olist))
  return multireplacer(table, escape=True) # escape is needed to get rid of '-'

_CONVERTERS = {}
def _convert(text, fr, to):
  """
  @param  text  unicode
  @param  fr  int
  @param  to  int
  @return  unicode
  """
  key = fr * 10 + to
  conv = _CONVERTERS.get(key)
  if not conv:
    conv = _CONVERTERS[key] = _makeconverter(fr, to)
  return conv(text)

# unicode -> unicode

def hira2kata(text): return _convert(text, _TYPE_HIRA, _TYPE_KATA)
def kata2hira(text): return _convert(text, _TYPE_KATA, _TYPE_HIRA)

def hira2romaji(text): return _repair_romaji(_convert(text, _TYPE_HIRA, _TYPE_ROMAJI))
def kata2romaji(text): return _repair_romaji(_convert(text, _TYPE_KATA, _TYPE_ROMAJI))
def kana2romaji(text): return _repair_romaji(_convert(text, _TYPE_KANA, _TYPE_ROMAJI))

def hira2ru(text): return _repair_romaji_ru(_convert(text, _TYPE_HIRA, _TYPE_ROMAJI_RU))
def kata2ru(text): return _repair_romaji_ru(_convert(text, _TYPE_KATA, _TYPE_ROMAJI_RU))
def kana2ru(text): return _repair_romaji_ru(_convert(text, _TYPE_KANA, _TYPE_ROMAJI_RU))

def hira2hangul(text): return _convert(text, _TYPE_HIRA_N, _TYPE_HANGUL)
def kata2hangul(text): return _convert(text, _TYPE_KATA_N, _TYPE_HANGUL)
def kana2hangul(text): return _convert(text, _TYPE_KANA_N, _TYPE_HANGUL)

def hira2thai(text): return _convert(text, _TYPE_HIRA, _TYPE_THAI)
def kata2thai(text): return _convert(text, _TYPE_KATA, _TYPE_THAI)
def kana2thai(text): return _convert(text, _TYPE_KANA, _TYPE_THAI)

# repair romaji
import re
_re_romaji = re.compile(ur"っ([a-z])")
def _repair_romaji(text): # unicode -> unicode  repair xtu
  """
  @param  text
  @return  unicode
  """
  return _re_romaji.sub(r'\1\1', text).replace(u'っ', u'-')

_ru_u = u"ауэояё"
_re_ru_u = re.compile(ur"(?<=[%s])и" % _ru_u)
_re_ru_z = re.compile(ur"\bэ")
def _repair_romaji_ru(text): # unicode -> unicode  repair xtu
  """
  @param  text
  @return  unicode
  """
  if u'и' in text:
    text = _re_ru_u.sub(u'й', text)
  if u'з' in text:
    text = _re_ru_z.sub(u'дз', text)
  return text

from sakurakit import skstr
_re_capitalize = skstr.multireplacer({
  #' Da ': ' da ',
  ' De ': ' de ',
  ' Ha ': ' ha ',
  ' Na ': ' na ',
  ' No ': ' no ',
  ' Ni ': ' ni ',
  ' To ': ' to ',
  #' O ': ' o ',
  ' Wo ': ' wo ',
})
def capitalizeromaji(text):
  """
  @param  text  unicode
  @return  unicode
  """
  return _re_capitalize(text.title())

_u_prefix = "\
おこそとのほもよろを\
ごぞどぼぽ\
ょ\
"#ゅ
_re_u = re.compile(ur"(?<=[%s])う" % _u_prefix)
def simplify_kana_name(text):
  """
  @param  text  unicode
  @return  unicode
  """
  if text and len(text) > 3 and u'う' in text:
    text = _re_u.sub('', text)
  return text

if __name__ == '__main__':
  #t = u"ウェブサイトツール"
  #t = u"うぇぶさいとつーる"
  #t = u"わかってる"
  t = u'さくらこうじ'
  print hira2romaji(t)
  #print kata2romaji(t)
  #print kata2hira(t)
  #print hira2kata(t)
  #print kata2hangul(t)
  print kana2hangul(t)
  print kana2thai(t)

  from jTransliterate import JapaneseTransliterator
  def test(text):
    return JapaneseTransliterator(text).transliterate_from_hrkt_to_latn()
  print test(t)

  t = u'さま'
  t = u'ひろすえ'
  t = u'ちゃん'
  t = u'せんせい'
  print hira2ru(t) # сэнсэй
  t = u'イイズミ-ちゃん'
  print kana2ru(t) # ийдзуми-чан, supposed to be Иизуми-чан
  t = u'ぱっつぁん'
  print hira2hangul(t)
  print hira2romaji(t)
  print hira2ru(t)

  t = u'みなとそふと'
  print hira2ru(t)
  t = u'ソフトクリーム'
  print kata2ru(t) # correct translation is Софуто-куриму

# EOF

## See: http://pypi.python.org/pypi/jTransliterate
#
#kata2romaji = kana2romaji
#hira2romaji = kana2romaji
