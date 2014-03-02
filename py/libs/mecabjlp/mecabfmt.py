# coding: utf8
# mecabfmt.py
# 2/24/2014 jichi
#
# Comparison of different mecab dictionaries
# http://www.mwsoft.jp/programming/munou/mecab_dic_perform.html

if __name__ == '__main__': # DEBUG
  import sys
  sys.path.append("..")

class DEFAULT(object):

  SPLIT = ','

  # Columns starts from 0

  COL_BASIC = 5 # The basic columns that are shared by are dict

  COL_SURFACE = COL_BASIC + 1
  COL_KATA = COL_SURFACE + 1
  #COL_YOMI = COL_KATA + 1 # not used

  @classmethod
  def getcol(cls, f, col): # unicode, int -> unicode
    l = f.split(cls.SPLIT)
    if len(l) >= col + 1:
      return l[col]

  @classmethod
  def getkata(cls, f): # unicode feature -> unicode or None
    return cls.getcol(f, cls.COL_KATA)

  @classmethod
  def getsurface(cls, f): # unicode feature -> unicode or None
    return cls.getcol(f, cls.COL_SURFACE)

# Example: 名詞,一般,*,*,*,*,憎しみ,ニクシミ,ニクシミ
class IPADIC(DEFAULT):

  @classmethod
  def getkata(cls, f): # override, more efficient
    i = f.rfind(',')
    if i > 0:
      j = f.rfind(',', 0, i)
      if j > 0:
        return f[j+1:i]

# Example: 名詞,普通名詞,一般,*,*,*,スモモ,李,すもも,スモモ,すもも,スモモ,和,*,*,*,*,スモモ,スモモ,スモモ,スモモ,*,*,0,C2,*
class UNIDIC(DEFAULT):
  COL_KANJI = DEFAULT.COL_BASIC + 2
  COL_SURFACE = COL_KANJI + 1
  COL_ORIG = 12 # Columns starts from 0
  COL_KATA = 17 # override

  @classmethod
  def getkanji(cls, f): # unicode feature -> unicode or None
    return cls.getcol(f, cls.COL_KANJI)

  @classmethod
  def getorig(cls, f): # unicode feature -> unicode or None
    return cls.getcol(f, cls.COL_ORIG)

FORMATS = {
  'ipadic': IPADIC,
  'unidic': UNIDIC,
  'unidic-mlj': UNIDIC,
}
def getfmt(name):
  return FORMATS.get(name) or DEFAULT

# EOF
