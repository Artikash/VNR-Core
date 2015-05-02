# coding: utf8
# convutil.py
# 10/18/2014 jichi

import re
from unitraits.uniconv import hira2kata, kata2hira, thin2wide, wide2thin
from unidecode import unidecode
from jaconv.jaconv import kana2romaji, kana2ru, kana2ko, kana2th, kana2ar \
                        , kana2name, kana2reading, capitalizeromaji
from opencc.opencc import zht2zhs #, ja2zht
from ccman import zht2zhx, ja2zhs_name
from ccman import zhs2zhx as zhs2zht
from ccman import ja2zhx_name as ja2zht_name
from hangulconv.hanjaconv import to_hanja as hangul2hanja
from kanjiconv.jazh import ja2zh_name_test, ja2zht_name_fix
import config
#from pinyinconv import pinyinconv

kana2yomi = kana2reading
kata2romaji = hira2romaji = kana2romaji
kata2ru = hira2ru = kana2ru
kata2ko = hira2ko = kana2ko
kata2th = hira2th = kana2th
kata2ar = hira2ar = kana2ar

_alphabet_el_re = re.compile(ur'σ\b', re.UNICODE)
def toalphabet(text, to='en', fr='en'):
  """
  @param  text  unicode
  @param* to  str
  @param* fr  str  currently not used
  @return  unicode
  """
  if fr == 'en':
    if to in config.ALPHABET_LANGUAGE_SET:
      from transliterate import translit
      text = translit(text, to)
      if to == 'el' and u'σ' in text:
        text = _alphabet_el_re.sub(u'ς', text) # replace σ with ς at last
  elif fr == 'ru':
    if to == 'uk':
      from unitraits import cyrilchars
      return cyrilchars.ru2uk(text)
  # Disabled
  #elif fr == 'uk':
  #  if to == 'ru':
  #    from unitraits import cyrilchars
  #    return cyrilchars.uk2ru(text)
  return text

#MSIME_VALID = False
#def ja2zh_name(text, simplified=False): # unicode, bool -> unicode
#  return ja2zhs_name(text) if simplified else ja2zht_name(text)
MSIME_VALID = None
def yomi2kanji(text):
  """
  @param  text  unicode
  @return  unicode or None
  """
  global MSIME_VALID
  if MSIME_VALID is None:
    from msime import msime
    MSIME_VALID = msime.ja_valid() # cached
  if MSIME_VALID:
    from msime import msime
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
