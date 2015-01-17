# coding: utf8
# convutil.py
# 10/18/2014 jichi

from unitraits.uniconv import hira2kata, kata2hira, thin2wide, wide2thin, wide2thin_digit
from unidecode import unidecode
from jaconv import jaconv
from jaconv.jaconv import hira2romaji, hira2hangul, hira2thai \
                        , kata2romaji, kata2hangul, kata2thai \
                        , kana2romaji, kana2hangul, kana2thai \
                        , capitalizeromaji
from opencc.opencc import zht2zhs #, ja2zht
from ccman import zhs2zht, zht2zhx
from hangulconv import hangulconv
#from pinyinconv import pinyinconv

from msime import msime
MSIME_VALID = msime.ja_valid() # cached

def kana2yomi(text, lang):
  """
  @param  text  unicode
  @param  lang  str
  @return  unicode or None
  """
  if lang == 'ko':
    return kana2hangul(text)
  elif lang == 'th':
    return kana2thai(text)
  else:
    return capitalizeromaji(kana2romaji(text))

def yomi2kanji(text):
  """
  @param  text  unicode
  @return  unicode or None
  """
  if MSIME_VALID:
    return msime.to_kanji(text)

def kana2name(text, lang):
  text = jaconv.simplify_kana_name(text)
  return kana2yomi(text, lang)

def toroman(text, language=''): # unicode, str -> unicode
  """
  @param  text  unicode
  @param* language  str
  @return  unicode or None
  """
  if isinstance(text, str):
    text = text.decode('utf8', errors='ignore')
  if not text:
    return u''
  #if language.startswith('zh'):
  #  ret = ja2zht(text)
  #  #ret = ko2zht(ret)
  #  ret = pinyinconv.to_pinyin(ret, capital=True)
  #else:
  ret = unidecode(text)
  if ret[-1] == ' ':
    ret = ret[:-1]
  if language in ('ko', 'ja') and ret != text:
    ret = ret.title()
  return ret

"""
@param  text  unicode
@return  unicode or None
"""
hangul2hanja = hangulconv.to_hanja

# EOF
