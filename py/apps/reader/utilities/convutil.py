# coding: utf8
# convutil.py
# 10/18/2014 jichi

from unitraits.uniconv import hira2kata, kata2hira, thin2wide, wide2thin, wide2thin_digit
from unidecode import unidecode
from jaconv import jaconv
from jaconv.jaconv import hira2romaji, hira2ru, hira2hangul, hira2thai \
                        , kata2romaji, kata2ru, kata2hangul, kata2thai \
                        , kana2romaji, kana2ru, kana2hangul, kana2thai \
                        , capitalizeromaji \
                        , kana2name
from jaconv.jaconv import kana2reading as kana2yomi
from opencc.opencc import zht2zhs #, ja2zht
from ccman import zhs2zht, zht2zhx
from hangulconv.hangulconv import to_hanja as hangul2hanja
#from pinyinconv import pinyinconv

from msime import msime
MSIME_VALID = msime.ja_valid() # cached

def yomi2kanji(text):
  """
  @param  text  unicode
  @return  unicode or None
  """
  if MSIME_VALID:
    return msime.to_kanji(text)

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

# EOF
