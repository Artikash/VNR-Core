# coding: utf8
# mecabcsv.py
# 11/9/2013 jichi
#
# See:
# http://tseiya.hatenablog.com/entry/2012/09/19/191114
# http://yukihir0.hatenablog.jp/entry/20110201/1296565687
# http://mecab.googlecode.com/svn/trunk/mecab/doc/dic.html
# http://mecab.googlecode.com/svn/trunk/mecab/doc/dic-detail.html
#
# Example csv:
# ユーザ設定,,,10,名詞,一般,*,*,*,*,ユーザ設定,ユーザセッテイ,ユーザセッテイ,追加エントリ

if __name__ == '__main__': # DEBUG
  import sys
  sys.path.append("..")

import codecs
from sakurakit.skdebug import dwarn
from cconv import cconv
import mecabfmt

def costof(surf):
  """
  @param  surf  unicode
  """
  # http://tseiya.hatenablog.com/entry/2012/09/19/191114
  # http://yukihir0.hatenablog.jp/entry/20110201/1296565687
  return int(max(-36000, -400*len(surf)**1.5))

def writecsv(dic, path, mode='w', encoding='utf8', fmt=mecabfmt.DEFAULT):
  """
  @param  dic  [unicode surface, unicode yomi]
  @return  bool
  """
  try:
    fout = codecs.open(path, mode, encoding) # enforce utf8 encoding
    for surf,yomi in dic:
      cost = costof(surf)
      hira = cconv.kata2hira(yomi)
      kata = cconv.hira2kata(yomi)
      line = fmt.csv(surf, cost, hira, kata) + '\n'
      fout.write(line)
    fout.close()
    return True
  except Exception, e:
    dwarn(e)
    return False

if __name__ == '__main__': # DEBUG
  dic = {u"遊星が": u"ユウセイガ"}
  path = "test.csv"
  ok = writecsv(dic.iteritems(), path)
  print ok

# EOF
