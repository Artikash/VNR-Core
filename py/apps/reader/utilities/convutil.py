# coding: utf8
# convutil.py
# 10/18/2014 jichi

from unitraits.uniconv import hira2kata, kata2hira, thin2wide, wide2thin, wide2thin_digit
from unidecode import unidecode
from jaconv.jaconv import hira2romaji, hira2hangul, hira2thai \
                        , kata2romaji, kata2hangul, kata2thai \
                        , kana2romaji, kana2hangul, kana2thai \
                        , capitalizeromaji
from opencc.opencc import zht2zhs
from ccman import zhs2zht, zht2zhx

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

def toromaji(text, language=''): # unicode, str -> unicode
  if isinstance(text, str):
    text = text.decode('utf8', errors='ignore')
  if not text:
    return u''
  ret = unidecode(text)
  if ret[-1] == ' ':
    ret = ret[:-1]
  if language in ('ko', 'ja') and ret != text:
    ret = ret.title()
  return ret

# EOF
