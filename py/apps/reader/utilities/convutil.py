# coding: utf8
# convutil.py
# 10/18/2014 jichi

from unitraits.uniconv import hira2kata, kata2hira, thin2wide, wide2thin
from unidecode import unidecode
from jaconv.jaconv import kana2romaji, kana2ru, kana2ko, kana2th \
                        , kana2name, kana2reading, capitalizeromaji
from opencc.opencc import zht2zhs #, ja2zht
from ccman import zht2zhx, ja2zhs_name
from ccman import zhs2zhx as zhs2zht
from ccman import ja2zhx_name as ja2zht_name
from hangulconv.hangulconv import to_hanja as hangul2hanja
from kanjiconv.jazh import ja2zh_name_test, ja2zht_name_fix
#from pinyinconv import pinyinconv

kana2yomi = kana2reading
kata2romaji = hira2romaji = kana2romaji
kata2ru = hira2ru = kana2ru
kata2ko = hira2ko = kana2ko
kata2th = hira2th = kana2th

from msime import msime
MSIME_VALID = msime.ja_valid() # cached

#def ja2zh_name(text, simplified=False): # unicode, bool -> unicode
#  return ja2zhs_name(text) if simplified else ja2zht_name(text)

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
  ret = unidecode(text) or text
  if ret[-1] == ' ':
    ret = ret[:-1]
  if language in ('ko', 'ja') and ret != text:
    ret = ret.title()
  return ret

# EOF
