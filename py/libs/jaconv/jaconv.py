# coding: utf8
# jaconv.py
# 12/30/2012 jichi

if __name__ == '__main__':
  import sys
  sys.path.append("..")

import re
from sakurakit.skstr import multireplacer
from unitraits.uniconv import hira2kata
import jadef

def _makeconverter(fr, to):
  """
  @param  fr  int
  @param  to  int
  @return  function or None
  """
  s = jadef.TABLES[to]
  if fr == 'kata':
    s = hira2kata(s)
  elif fr == 'kana':
    s += '\n' + hira2kata(s)

  t = jadef.parse(s)
  return multireplacer(t)

_CONVERTERS = {}
def _convert(text, fr, to):
  """
  @param  text  unicode
  @param  fr  int
  @param  to  int
  @return  unicode
  """
  conv = _CONVERTERS.get(fr + to)
  if not conv:
    conv = _CONVERTERS[fr + to] = _makeconverter(fr, to)
  text = conv(text)
  text = text.replace(u'ッ', u'っ')
  return text

# unicode -> unicode

def hira2en(text): return _repair_reading(_repair_en(_convert(text, 'hira', 'en')))
def kata2en(text): return _repair_reading(_repair_en(_convert(text, 'kata', 'en')))
def kana2en(text): return _repair_reading(_repair_en(_convert(text, 'kana', 'en')))

def hira2ru(text): return _repair_reading(_repair_ru(_convert(text, 'hira', 'ru')))
def kata2ru(text): return _repair_reading(_repair_ru(_convert(text, 'kata', 'ru')))
def kana2ru(text): return _repair_reading(_repair_ru(_convert(text, 'kana', 'ru')))

def hira2th(text): return _repair_reading(_repair_th(_convert(text, 'hira', 'th')))
def kata2th(text): return _repair_reading(_repair_th(_convert(text, 'kata', 'th')))
def kana2th(text): return _repair_reading(_repair_th(_convert(text, 'kana', 'th')))

def hira2ko(text): return _repair_reading(_convert(text, 'hira', 'ar'))
def kata2ko(text): return _repair_reading(_convert(text, 'kata', 'ar'))
def kana2ko(text): return _repair_reading(_convert(text, 'kana', 'ar'))

def hira2ar(text): return _repair_reading(_convert(reversed(text), 'hira', 'ar'))
def kata2ar(text): return _repair_reading(_convert(reversed(text), 'kata', 'ar'))
def kana2ar(text): return _repair_reading(_convert(reversed(text), 'kana', 'ar'))

hira2romaji = hira2en
kata2romaji = kata2en
kana2romaji = kana2en

def _repair_reading(text):
  """
  @param  text
  @return  unicode
  """
  text = text.replace(u'ー', '-')
  text = text.replace(u'っ', '-')
  return text

_re_en_tsu = re.compile(ur"っ([bcdfghjklmnprstvxz])")
def _repair_en(text): # unicode -> unicode  repair xtu
  """
  @param  text
  @return  unicode
  """
  if u'っ' in text:
    text = _re_en_tsu.sub(r'\1\1', text).replace(u'っ', u'-')
  return text

_ru_i_vowel = u"ауэояё"
_re_ru_i = re.compile(ur"(?<=[%s])и" % _ru_i_vowel)
_re_ru_ii = re.compile(ur"(?<=[%s])й(и+)" % _ru_i_vowel)
_re_ru_z = re.compile(ur'\bз', re.UNICODE)
_re_ru_tsu = re.compile(ur"っ([бвгдзклмнпрстфхцчшщъыь])")
def _repair_ru(text): # unicode -> unicode  repair xtu
  """
  @param  text
  @return  unicode
  """
  if u'っ' in text:
    text = _re_ru_tsu.sub(r'\1\1', text)
  if u'и' in text:
    text = _re_ru_i.sub(u'й', text)
    if u'йи' in text:
      text = _re_ru_ii.sub(ur'\1й', text) # push i to the end
  if u'з' in text:
    text = _re_ru_z.sub(u'дз', text)
  return text

# http://en.wikipedia.org/wiki/Thai_alphabet
# Thai unicode range: U+0E00–U+0E7F
_th_b = u'(?:^|(?<![\u0e00-\u0e7f]))' # \b at the beginning
_th_e = u'(?:$|(?![\u0e00-\u0e7f]))' # \e at the beginning
_re_th = (
  (re.compile(_th_b + u'ก'), u'ค'), # k
  #(re.compile(_th_b + u'จิ'), u'ชิ'), # chi
  (re.compile(_th_b + u'ตา'), u'ทา'), # ta
  #(re.compile(_th_b + ur"ย์"), u'อิ'), # i => yi
  (re.compile(_th_b + ur"ย์"), u'อี'), # i => yi
  (re.compile(ur"คุ" + _th_e), u'ขุ'), # ku
  (re.compile(ur"า" + _th_e), u'ะ'),  # a
  (re.compile(u"คะ" + _th_e), u'กะ'), # ka (after applying a)
  (re.compile(ur"([โเ][กรตน])" + _th_e), ur'\1ะ'), # oe
)
def _repair_th(text):
  """
  @param  text
  @return  unicode
  """
  #return text
  for pat, repl in _re_th:
    text = pat.sub(repl, text)
  return text

# Names

def kana2reading(text, lang):
  """
  @param  text  unicode
  @param  lang  str
  @return  unicode or None
  """
  if lang == 'ko':
    return kana2ko(text)
  elif lang == 'th':
    return kana2th(text)
  elif lang == 'ru':
    return capitalizeromaji(kana2ru(text))
  else:
    return capitalizeromaji(kana2romaji(text))

def kana2name(text, lang):
  text = simplify_kana_name(text)
  return kana2reading(text, lang)

_re_capitalize = multireplacer({
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
ゅ\
"
_re_u = re.compile(ur"(?<=[%s])う" % _u_prefix)
def simplify_kana_name(text):
  """
  @param  text  unicode
  @return  unicode
  """
  text = text.replace(u"ー", '')
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
  #print kata2ko(t)
  print kana2ko(t)
  print kana2th(t)

  from jTransliterate import JapaneseTransliterator
  def test(text):
    return JapaneseTransliterator(text).transliterate_from_hrkt_to_latn()
  print test(t)

  t = u'イイズミ-ちゃん'
  print kana2ru(t) # ийдзуми-чан, supposed to be Иизуми-чан
  t = u'ぱっつぁん'
  print hira2ko(t)
  print hira2romaji(t)
  print hira2ru(t)

  t = u'みなとそふと'
  print hira2ru(t)
  t = u'ソフトクリーム'
  print kata2ru(t) # correct translation is Софуто-куриму

  # Romaji
  l = [
    (u'かわいい', u'kawaii'),
    (u'いぇす', u'yesu'),
    (u'ジャケット', u'jaketto'),
    (u'せんせい', u'sensei'),
    (u'ちゃん', u'chan'),
    (u'ちょうきょう', u'choukyou'),
  ]
  for k,v in l:
    print k, kana2romaji(k), v
    assert kana2romaji(k) == v

  # Russian
  l = [
    (u'かわいい', u'каваий'), # http://ru.wikipedia.org/wiki/каваий
    #(u'いぇす', u'иэсу'), # not sure
    (u'ジャケット', u'дзякэтто'),
    (u'せんせい', u'сэнсэй'),
    (u'ちゃん', u'чан'),
    (u'いえやす', u'иэясу'),
    #(u'ちょうきょう', u'чоукёу'), # not sure
  ]
  for k,v in l:
    print k, kana2ru(k), v
    assert kana2ru(k) == v

  # Korean
  l = [
    (u'しおり', u'시오리'),
    (u'いぇす', u'예스'),
  ]
  for k,v in l:
    print k, kana2ko(k), v
    assert kana2ko(k) == v

  # Thai
  l = [
    #(u'すず', u'ซูซุ'), fail
    #(u'すすら', u'ซึซึระ'), # fail
    #(u'すずしろ', u'ซุสึชิโระ'), # fail
    #(u'すずかけ', u'สุซึคาเคะ'), # fail because すす => すず
    #(u'いすず', u'อีซูซุ'),
    (u'ちはや', u'จิฮายะ'),
    (u'すかもり', u'สึคาโมริ'),
    (u'たにゃ', u'ทาเนีย'),
    (u'みかづき', u'มิคาซึกิ'),
    (u'つぐみ', u'สึกุมิ'),
    (u'かなり', u'คานาริ'),
    (u'ましろ', u'มาชิโระ'),
    (u'まどか', u'มาโดกะ'),
    #(u'かのん', u'คาน่อน'), # fail because of の
    (u'まゆ', u'มายุ'),
    (u'けせん', u'เคเซน'),
    #(u'ちばな', u'ชิบานะ'), # fail because of chi
    (u'きさき', u'คิซากิ'),
    (u'みやこ', u'มิยาโกะ'),
    (u'ふじな', u'ฟุจินะ'),
    (u'ひろはら', u'ฮิโรฮาระ'),
    (u'さぎばら', u'ซาคิบาระ'),
    (u'まる', u'มารุ'),
    (u'おてんた', u'โอเท็นตะ'),
    (u'むねちか', u'มุเนะจิกะ'),
    (u'くろば', u'คุโรบะ'),
    (u'けい', u'เคย์'),
    (u'さねあき', u'ซาเนะอากิ'),
    (u'はるかぜ', u'ฮารุคาเซะ'),
    (u'きぬむら',u'คินุมุระ'),
    (u'れんな', u'เร็นนะ'),
    #(u'はさくら', u'ฮาซากุระ'), # fail because of ku
    (u'れんか', u'เร็นกะ'),
    (u'りん', u'ริน'),
    (u'みなもり', u'มินาโมริ'),
    (u'ほのか', u'โฮโนกะ'),
    (u'あやめ', u'อายาเมะ'),
    (u'たくや', u'ทาคุยะ'),
    (u'みうら', u'มิอุระ'), # http://th.wikipedia.org/wiki/เดป้าเปเป้
    (u'よしなり', u'โยชินาริ'),
    (u'とくおか', u'โทคุโอกะ'),
    (u'まえだ', u'มาเอดะ'),
    (u'ふうちゃん', u'ฟูจัง'), # http://th.wikipedia.org/wiki/รักลวงป่วนใจ
    #(u'つきの', u'สึคิโนะ'), # fail because of ki
    #(u'てんまく', u'เทนมาขุ'), # fail because of te
    #(u'えんにし', u'เอนิชิ'), # fail because ennishi => enishi
    #美愛: u'มิจิกะ
    #かがみ: คางามิ
    #幸和: ซาจิคาซุ
    #一悟: อิจิโกะ
    #左京: ซาเคียว
    #みずのみや: มิซุโนะมิยะ
    #冬馬: โทวมะ
  ]
  for k,v in l:
    print k, hira2th(k), v
    assert kana2th(k) == v

  # Arabian
  l = [
    (u'さくら', u'ساكورا'),
  ]
  for k,v in l:
    print k, kana2ar(k), v
    assert kana2ar(k) == v

# EOF

## See: http://pypi.python.org/pypi/jTransliterate
#
#kata2romaji = kana2romaji
#hira2romaji = kana2romaji
