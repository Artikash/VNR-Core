# coding: utf8
# zhja.py
# 9/21/2014: jichi
# See: http://www.jcinfo.net/jp/kanji/


#JT = dict(zip(J, T)) # {unicode:unicode}
#TJ = dict(zip(T, J)) # {unicode:unicode}
TJ = {
  # Equivalent
  u"說": u"説",
  u"你": u"妳",

  # Inequivalent
  u"嗎": u"麼",
  u"啊": u"あ",
  #u"爸": ?,
  #u"哟": ?,
}

#def zhs2ja(s): # unicode -> unicode

def zht2ja(t): # unicode -> unicode
  return ''.join((TJ.get(c) or c for c in t))

if __name__ == '__main__':
  ja = u"爸說嗎"
  zh = zht2ja(ja)

  from PySide.QtCore import *
  c = QTextCodec.codecForName('sjis')
  t = c.toUnicode(c.fromUnicode(zh))
  print t

# EOF
