# coding: utf8
# jaconv.py
# 12/30/2012 jichi

if __name__ == '__main__':
  import sys
  sys.path.append("..")

import re
from sakurakit.skstr import multireplacer
from unitraits.uniconv import hira2kata
import kanadef

def _makeconverter(fr, to):
  """
  @param  fr  int
  @param  to  int
  @return  function or None
  """
  s = kanadef.TABLES[to]
  if fr == 'kata':
    s = hira2kata(s)
  elif fr == 'kana':
    s += '\n' + hira2kata(s)

  t = kanadef.parse(s)
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

def hira2uk(text): return _repair_reading(_repair_uk(_convert(text, 'hira', 'uk')))
def kata2uk(text): return _repair_reading(_repair_uk(_convert(text, 'kata', 'uk')))
def kana2uk(text): return _repair_reading(_repair_uk(_convert(text, 'kana', 'uk')))

def hira2el(text): return _repair_reading(_repair_el(_convert(text, 'hira', 'el')))
def kata2el(text): return _repair_reading(_repair_el(_convert(text, 'kata', 'el')))
def kana2el(text): return _repair_reading(_repair_el(_convert(text, 'kana', 'el')))

def hira2th(text): return _repair_reading(_repair_th(_convert(text, 'hira', 'th')))
def kata2th(text): return _repair_reading(_repair_th(_convert(text, 'kata', 'th')))
def kana2th(text): return _repair_reading(_repair_th(_convert(text, 'kana', 'th')))

def hira2ko(text): return _repair_reading(_convert(text, 'hira', 'ko'))
def kata2ko(text): return _repair_reading(_convert(text, 'kata', 'ko'))
def kana2ko(text): return _repair_reading(_convert(text, 'kana', 'ko'))

def hira2ar(text): return _repair_reading(_convert(text, 'hira', 'ar'))
def kata2ar(text): return _repair_reading(_convert(text, 'kata', 'ar'))
def kana2ar(text): return _repair_reading(_convert(text, 'kana', 'ar'))

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

# http://en.wikipedia.org/wiki/Russian_alphabet
_ru_i_vowel = u"ауэояё"
_ru_consonant = u"бвгдзклмнпрстфхцчшщъыь"
_re_ru_i = re.compile(ur"(?<=[%s])и" % _ru_i_vowel)
_re_ru_ii = re.compile(ur"[ий](и+)")
_re_ru_z = re.compile(ur'\bз', re.UNICODE)
_re_ru_tsu = re.compile(ur"っ([%s])" % _ru_consonant)
def _repair_ru(text): # unicode -> unicode  repair xtu
  """
  @param  text
  @return  unicode
  """
  if u'っ' in text:
    text = _re_ru_tsu.sub(r'\1\1', text)
  if u'и' in text:
    text = _re_ru_i.sub(u'й', text)
    text = _re_ru_ii.sub(ur'\1й', text) # push i to the end
  if u'з' in text:
    text = _re_ru_z.sub(u'дз', text)
  return text

# http://en.wikipedia.org/wiki/Ukrainian_alphabet
_uk_consonant = _ru_consonant + u'ґ'
_re_uk_tsu = re.compile(ur"っ([%s])" % _uk_consonant)
def _repair_uk(text): # unicode -> unicode  repair xtu
  """
  @param  text
  @return  unicode
  """
  if u'っ' in text:
    text = _re_uk_tsu.sub(r'\1\1', text)
  return text

# http://en.wikipedia.org/wiki/Greek_alphabet
_el_consonant = u'βγδζθκλμνξπρτφχψω'
_re_el_tsu = re.compile(ur"っ([%s])" % _el_consonant)
def _repair_el(text): # unicode -> unicode  repair xtu
  """
  @param  text
  @return  unicode
  """
  if u'っ' in text:
    text = _re_el_tsu.sub(r'\1\1', text)
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
  if lang in ('ko', 'th'):
    # remove xtu in name
    text = text.replace(u'っ', '')
    text = text.replace(u'ッ', '')
    if not text:
      return ''
  if lang == 'ko':
    return kana2ko(text)
  elif lang == 'th':
    text = text.replace(u'おお', u'お')
    return kana2th(text)
  elif lang == 'ar':
    if u'ゆ' in text:
      text = text.replace(u'ゆう', u'ゆ')
      text = text.replace(u'ゆぅ', u'ゆ')
    return kana2ar(text)
  elif lang == 'ru':
    text = kana2ru(text)
  elif lang == 'uk':
    text = kana2uk(text)
  elif lang == 'el':
    text = kana2el(text)
  else:
    text = kana2romaji(text)
  return capitalizeromaji(text)

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

_u_prefix = u"\
おこそとのほもよろを\
ごぞどぼぽ\
ょ\
ゅ\
"
_re_u = re.compile(ur"(?<=[%s])う" % _u_prefix)
_o_prefix = u"とど"
_re_o = re.compile(ur"(?<=[%s])お" % _o_prefix)
def simplify_kana_name(text):
  """
  @param  text  unicode
  @return  unicode
  """
  text = text.replace(u"ー", '')
  if text and len(text) > 3 and u'う' in text:
    text = _re_u.sub('', text)
  if text and len(text) > 3 and u'お' in text:
    text = _re_o.sub('', text)
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
    (u'ぐりぐり', u'guriguri'),
  ]
  for k,v in l:
    print k, kana2romaji(k), v
    assert kana2romaji(k) == v

  # Russian
  l = [
    (u'かわいい', u'каваий'), # http://ru.wikipedia.org/wiki/каваий
    (u'じい', u'дзий'), # not sure
    (u'いぇす', u'иэсу'), # not sure
    (u'ジャケット', u'дзякэтто'),
    (u'せんせい', u'сэнсэй'),
    (u'ちゃん', u'чан'),
    (u'いえやす', u'иэясу'),
    #(u'サシャ', u'саша'), # wrong, got сася
    #(u'ちょうきょう', u'чоукёу'), # not sure
  ]
  for k,v in l:
    print k, kana2ru(k), v
    assert kana2ru(k) == v

  # Ukrainian
  l = [
    (u'くん', u'кун'),
    (u'いえやす', u'іеясу'),
    (u'ほのか', u'гонока'),
    (u'ほのっか', u'гонокка'),
    (u'かわいい', u'каваіі'),
    (u'れん', u'рен'),
  ]
  for k,v in l:
    print k, kana2uk(k), v
    assert kana2uk(k) == v

  # Greek
  l = [
    (u'かわいい', u'καωαιι'), # not sure how to say it
    #(u'こんと', u'κόντο'), # failed
    #(u'たろ', u'ταρό'), # failed
    #(u'やまた', u'γιαμάντα'), # failed
    # http://el.wikipedia.org/wiki/Ιαπωνική_γραφή
    (u'イロハニホヘト', u'ιροχανιχοχετο'),  # 色は匂へと
    #(u'チリヌルヲ',  u'τσιρινουρουβο'),     # 散りぬるを failed since を is translated to ωο instead of βο
    #(u'ワカヨタレソ', u'βακαγιοταρεσο'),    # 我が世誰そ failed
    (u'ツネナラム', u'τσουνεναραμου'),      # 常ならむ
    #(u'ウヰノオクヤマ', u'ουβινοοκουγιαμα'), # 有為の奥山
    (u'ケフコエテ', u'κεφουκοετε'),         # 今日越えて
    (u'アサキユメミシ', u'ασακιγιουμεμισι'), # 浅き夢見し
    #(u'ヱヒモセス', u'βεχιμοσεσου'),        # 酔ひもせす
  ]
  for k,v in l:
    print k, kana2el(k), v
    assert kana2el(k) == v

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
    (u'かのん', u'คาน่อน'),
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
    (u'ゆうま', u'ยูมะ'),
    (u'とうこ', u'โทวโกะ'),
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

  # Arabic
  l = [
    (u'さくら', u'ساكورا'),
    (u'さと', u'ساتو'),
    (u'かがみ', u'كاغامي'),
    (u'かおる', u'كاورو'),
    (u'さい', u'ساي'),
    (u'さいと', u'سايتو'),
    (u'やまだ', u'يامادا'),
    (u'やまもと', u'ياماموتو'),
    (u'なかむら', u'ناكامورا'),
    (u'ふくだ', u'فوكودا'),
    (u'さと', u'ساتو'),
    (u'まつもと', u'ماتسوموتو'),
    (u'かよ', u'كايو'),
    (u'ぐれん', u'غورين'),
    (u'しんご', u'شينغو'),
    (u'すざく', u'سوزاكو'),
    (u'となり の ととろ', u'توناري نو توتورو'),
    (u'さおり', u'ساوري'),
    (u'ゆうま', u'يوما'),
    (u'かわいい', u'كاوايي'),
    (u'ちゃん', u'تشان'),
    (u'さま', u'ساما'),
    (u'シャナ', u'شانا'),
    (u'ぴこ', u'بيكو'),
    (u'ぺこ', u'بيكو'),
    (u'べこ', u'بيكو'),
    (u'ボコ', u'بوكو'),
    (u'さど', u'سادو'),
    (u'ぱぴ', u'بابي'),
    (u'かぶと', u'كابوتو'),
    (u'つばさ', u'تسوباسا'),
    (u'さすけ', u'ساسوكي'),
    (u'あゆみ', u'ايومي'),
    (u'めぐみ', u'ميغومي'),
    (u'かおる', u'كاورو'),
    (u'わだ', u'وادا'),
    (u'あべ', u'ابي'),
    #(u'えいじ', u'إيجي'), # failed, totally wrong
    #(u'ぜん', u'زن'), # failed because zen is not handled
    #(u'かな', u'قانا'), # failed because of wrong ka
  ]
  for k,v in l:
    print k, kana2name(k, 'ar'), v
    assert kana2name(k, 'ar') == v

# EOF

## See: http://pypi.python.org/pypi/jTransliterate
#
#kata2romaji = kana2romaji
#hira2romaji = kana2romaji
