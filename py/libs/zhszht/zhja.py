# coding: utf8
# zhja.py
# 9/21/2014: jichi
# See: http://www.jcdic.com/chinese_convert/index.php

#JT = dict(zip(J, T)) # {unicode:unicode}
#TJ = dict(zip(T, J)) # {unicode:unicode}
TJ = {
  # Equivalent conversion
  u"—": u"ー",

  u"內": u"内",
  u"晚": u"晩",
  u"說": u"説",
  u"悅": u"悦",
  u"团": u"団",
  u"蔥": u"葱",
  u"擊": u"撃",
  u"絕": u"絶",
  u"佈": u"布",
  u"誒": u"欸",
  u"幫": u"幇",
  u"雞": u"鶏",
  u"歲": u"歳",
  u"咖": u"珈",
  u"啡": u"琲",
  u"僱": u"雇",
  u"錄": u"録",
  u"莆": u"蒲",

  u"你": u"ｲ尓",
  u"妳": u"ｲ尓",

  u"她": u"他",

  u"脖": u"月孛",

  # Inequivalent conversion
  u"啊": u"哦",
  u"噢": u"哦",
  u"喲": u"哦",
  u"喔": u"哦",
  u"啦": u"拉",
  u"哎": u"欸",
  u"呃": u"額",
  u"嗎": u"嘛",

  u"吧": u"巴",
  u"啪": u"巴",
  u"嗯": u"恩",
  u"噗": u"璞",
  u"呲": u"兹",

  u"咕": u"呼",
  u"噜": u"魯",

  u"爸": u"巴",

  # Not needed, but to simplified version
  #u"號": u"号",
  #u"體": u"体",
}

#def zhs2ja(s): # unicode -> unicode

def zht2ja(t): # unicode -> unicode
  return ''.join((TJ.get(c) or c for c in t))

if __name__ == '__main__':
  ja = u"「嗯你妳會認為我的事相當自私？」"
  zh = zht2ja(ja)
  print u"歲"== u"歳"
  print u"內"== u"内"
  print u"晚" == u"晩"

  from PySide.QtCore import *
  c = QTextCodec.codecForName('sjis')
  t = c.toUnicode(c.fromUnicode(zh))
  print t

# EOF
