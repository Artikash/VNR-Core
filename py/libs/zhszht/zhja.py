# coding: utf8
# zhja.py
# 9/21/2014: jichi
# See: http://www.jcinfo.net/jp/kanji/

#JT = dict(zip(J, T)) # {unicode:unicode}
#TJ = dict(zip(T, J)) # {unicode:unicode}
TJ = {
  # Equivalent
  u"—": u"ー",

  u"說": u"説",
  u"你": u"妳",
  u"团": u"団",
  u"蔥": u"葱",
  u"擊": u"撃",
  u"絕": u"絶",
  u"佈": u"布",
  u"誒": u"欸",
  u"幫": u"幇",

  # Inequivalent
  u"嗎": u"麼",
  u"啊": u"あ",
  u"哟": u"よ",
  u"吧": u"巴",
  u"哎": u"欸",
  u"呃": u"额",

  u"爸": u"巴",
}

#def zhs2ja(s): # unicode -> unicode

def zht2ja(t): # unicode -> unicode
  return ''.join((TJ.get(c) or c for c in t))

if __name__ == '__main__':
  ja = u"渴餓額姐姐弟弟"
  zh = zht2ja(ja)

  from PySide.QtCore import *
  c = QTextCodec.codecForName('sjis')
  t = c.toUnicode(c.fromUnicode(zh))
  print t

# EOF
