# coding: utf8
# mecabdef.py
# 11/9/2013 jichi

# ipadic encoding, either SHIFT-JIS or UTF-8
DICT_ENCODING = 'utf8'

# MeCab definitions
MECAB_BOS_NODE = 2 # MeCab.MECAB_BOS_NODE
MECAB_EOS_NODE = 3 # MeCab.MECAB_EOS_NODE

# CHAR_TYPE

CH_VERB = 2
CH_NOUN = 8
CH_MODIFIER = 6
CH_PUNCT = 3
CH_KATAGANA = 7
CH_LATIN = 4
CH_LATIN2 = 5
CH_GREEK = 9

# Ruby yomigana type
RB_KATA = 'kata'
RB_HIRA = 'hira'
RB_ROMAJI = 'romaji'
RB_RU = 'ru'
RB_UK = 'uk'
RB_EL = 'el'
RB_KO = 'ko'
RB_TH = 'th'
RB_AR = 'ar'

RB_VI = 'vi' # not implemented yet
RB_TR = 'tr' # not implemented yet

def rb_lang(ruby): # str -> str
  if ruby in (RB_KATA, RB_HIRA):
    return 'ja'
  if ruby in (RB_ROMAJI, RB_TR):
    return 'en'
  return ruby

def rb_has_space(ruby): # str -> bool
  return ruby not in (RB_HIRA, RB_KATA)
def rb_is_wide(ruby): # str -> bool
  return ruby in (RB_HIRA, RB_KATA, RB_KO)
def rb_is_thin(ruby): # str -> bool
  return not rb_is_wide(ruby)
def rb_is_wide_punct(ruby): # str -> bool
  return ruby in (RB_HIRA, RB_KATA)
def rb_is_thin_punct(ruby): # str -> bool
  return not rb_is_wide_punct(ruby)

# Surface type

SURFACE_UNKNOWN = 0
SURFACE_PUNCT = 1
SURFACE_KANJI = 2
SURFACE_NUMBER = 3
SURFACE_KANA = 4

# EOF
