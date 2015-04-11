# coding: utf8
# mdcompile.py
# 11/9/2013 jichi
#
# See:
# http://tseiya.hatenablog.com/entry/2012/09/19/191114
# http://mecab.googlecode.com/svn/trunk/mecab/doc/dic.html
#
# Example:
# mecab-dict-index -d /usr/local/Cellar/mecab/0.993/lib/mecab/dic/ipadic -u original.dic -f utf8 -t utf8 out.csv

if __name__ == '__main__': # DEBUG
  import sys
  sys.path.append("..")

import os, subprocess
from sakurakit.skdebug import dwarn
from sakurakit import skfileio
from unitraits.uniconv import hira2kata, kata2hira

def costof(size):
  """
  @param  size  int  surface size
  @return  int  negative  the smaller the more important
  """
  # Ruby: http://qiita.com/ynakayama/items/388c82cbe14c65827769
  # Python:
  # - http://tseiya.hatenablog.com/entry/2012/09/19/191114
  # - http://yukihir0.hatenablog.jp/entry/20110201/1296565687
  return -min(36000, int(400*size**1.5))

# Example  きす,5131,5131,887,名詞,普通名詞,サ変可能,*,*,*,キス,キス-kiss,キス,キス,キス,キス,外,*,*,*,*
UNIDIC_FMT = '{surf},0,0,{cost},*,*,*,*,*,*,{kata},{kanji},{surf},*,*,*,{content},*,*,{id},{type}\n'

# 名詞,普通名詞,一般,*,*,*,ゴメン,御免,御免,ゴメン,御免,ゴメン,漢,*,*,*,*,ゴメン,ゴメン,ゴメン,ゴメン,*,*,0,C2,*
# 動詞,非自立可能,*,*,五段-ラ行,命令形,ナサル,為さる,なさい,ナサイ,なさる,ナサル,和,*,*,*,*,ナサイ,ナサル,ナサイ,ナサル,*,*,2,C1,*
def assemble(entries, fmt=UNIDIC_FMT, id='*', type='*', surfacefilter=None):
  """
  @param  id  long  sql role id
  @param  entries  [unicode surface, unicode reading]
  @param* fmt  unicode
  @param* surfacefilter  unicode -> bool  whether keep certain surface
  @yield  unicode
  """
  surfaces = set()
  for surf, yomi in entries:
    if surf not in surfaces:
      surfaces.add(surf)
      if not surfacefilter or surfacefilter(surf):
        hira = kata2hira(yomi) if yomi else ''
        kata = hira2kata(yomi) if yomi else ''
        kanji = '*' # not implemened
        content = surf
        cost = costof(len(surf))
        yield fmt.format(surf=surf, kata=kata, kanji=kanji, content=content, cost=cost, type=type, id=id)
        for kana in (yomi, hira, kata):
          if kana and kana not in surfaces:
            surfaces.add(kana)
            if not surfacefilter or surfacefilter(kana):
              cost = costof(len(kana))
              yield fmt.format(surf=kana, kata=kata, kanji=kanji, content=content, cost=cost, type=type, id=id)

MIN_CSV_SIZE = 10 # minimum CSV file size
def compile(dic, csv, exe='mecab-dict-index', dicdir='', call=subprocess.call):
  """csv2dic. This process would take several seconds
  @param  dic  unicode  path
  @param  csv  unicode  path
  @param* exe  unicode  path
  @param* dicdir  unicode
  @param* call  launcher function
  @return  bool
  """
  # MeCab would crash for empty sized csv
  if skfileio.filesize(csv) < MIN_CSV_SIZE:
    dwarn("insufficient input csv size", csv)
    return False
  args = [
    exe,
    '-f', 'utf8', # from utf8
    '-t', 'utf8', # to utf8
    '-u', dic,
    csv,
  ]
  if dicdir:
    args.extend((
      '-d', dicdir
    ))
  return call(args) in (0, True) and os.path.exists(dic)

if __name__ == '__main__':
  os.environ['PATH'] += os.path.pathsep + '../../../../MeCab/bin'

  csvpath = 'edict.csv'

  def test_assemble():
    dbpath = '../dictp/edict.db'
    import sqlite3
    from dictdb import dictdb
    from dictp import edictp

    with sqlite3.connect(dbpath) as conn:
      cur = conn.cursor()
      q = dictdb.iterwords(cur)
      with open(csvpath, 'w') as f:
        for i,word in enumerate(q):
          id = i + 1
          entries = edictp.parseword(word)
          lines = assemble(entries, id=id, type='edict')
          f.writelines(lines)

  def test_compile():
    dicdir = "../../../../../../Caches/Dictionaries/UniDic"
    #dicdir = "/opt/local/lib/mecab/dic/unidic-utf8"
    print compile('edict.dic', csvpath, dicdir=dicdir)
    #print compile('test.dic', 'test.csv', dicdir=dicdir)

  test_assemble()
  test_compile()

# EOF
