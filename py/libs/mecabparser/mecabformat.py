# coding: utf8
# mecabformat.py
# 2/24/2014 jichi
# Dictionary feature format.
# Comparison of different mecab dictionaries
# http://www.mwsoft.jp/programming/munou/mecab_dic_perform.html

if __name__ == '__main__': # DEBUG
  import sys
  sys.path.append("..")

class UniDicFormatter(object):

  SEP = ',' # column separator
  EMPTY_COL = '*' # empty column

  # Columns starts from 0

  COL_BASIC = 5 # The basic columns that are shared by are dict holding the roles of the word

  COL_SURFACE = COL_BASIC + 1
  COL_KANJI = COL_BASIC + 2 # converted kanji
  COL_SURFACE = COL_KANJI + 1
  COL_ORIGIN = 12 # the origination of the phrase

  COL_KATA = COL_BASIC + 1
  COL_KATA0 = 17 # the original kata column without EDICT that should be checked first

  COL_ID = -2 # the type such as edict
  COL_TYPE = -1 # the type such as edict

  def getcol(self, f, col):
    """
    @param  f  unicode or list feature
    @param  col  int  column
    @return  unicode not none
    """
    if isinstance(f, basestring):
      f = f.replace(self.EMPTY_COL, '').split(self.SEP)
    return f[col] if col < len(f) else ''

  def getkata(self, f):
    return self.getcol(f, self.COL_KATA0) or self.getcol(f, self.COL_KATA)

  def getsurface(self, f): return self.getcol(f, self.COL_SURFACE)
  def getkanji(self, f): return self.getcol(f, self.COL_KANJI)
  def getorigin(self, f): return self.getcol(f, self.COL_ORIGIN)

  def gettype(self, f): return self.getcol(f, self.COL_TYPE)
  def getid(self, f): # unicode -> int or 0
    try: return int(self.getcol(f, self.COL_ID))
    except: return 0

UNIDIC_FORMATTER = UniDicFormatter()

# EOF


# http://chocolapod.sakura.ne.jp/blog/entry/56
# ジョジョの奇妙な冒険,,,0,名詞,固有名詞,一般,*,*,*,ジョジョノキミョウナボウケン,ジョジョの奇妙な冒険,ジョジョの奇妙な冒険,ジョジョノキミョーナボウケン,ジョジョの奇妙な冒険,ジョジョノキミョーナボウケン,*,*,*,*
#def csv(cls, surf, cost, hira, kata): # override
#  # ジョジョの奇妙な冒険,,,0,名詞,固有名詞,一般,*,*,*,ジョジョノキミョウナボウケン,ジョジョの奇妙な冒険,ジョジョの奇妙な冒険,ジョジョノキミョーナボウケン,ジョジョの奇妙な冒険,ジョジョノキミョーナボウケン,*,*,*,*
#  #return u"%s,,,%s,名詞,固有名詞,人名,姓,*,*,%s,%s,%s,%s,%s,%s,*,*,*,*" % (surf, cost, kata, surf, surf, kata, surf, kata)
#  # FIXME: The above does not work
#  return u"%s,,,%s,名詞,固有名詞,一般,*,*,*,%s,%s,%s,%s,%s,%s,*,*,*,固" % (surf, cost, kata, surf, surf, kata, surf, kata)

# Example: 名詞,一般,*,*,*,*,憎しみ,ニクシミ,ニクシミ
#class IPADIC(DEFAULT):
#
#  @classmethod
#  def getkata(cls, f): # override, more efficient
#    i = f.rfind(',')
#    if i > 0:
#      j = f.rfind(',', 0, i)
#      if j > 0:
#        return f[j+1:i]
#
#FORMATS = {
#  'ipadic': IPADIC,
#  'unidic': UNIDIC,
#  'unidic-mlj': UNIDIC,
#}
#def getfmt(name):
#  return FORMATS.get(name) or DEFAULT
