# coding: utf8
# hangulparse.py
# 1/6/2015 jichi

if __name__ == '__main__': # DEBUG
  import sys
  sys.path.append("..")

from sakurakit.skclass import memoized

HANGUL_DIC_DIR = '' # to be set
HANGUL_DIC_CONV = 'dic6.txt'

def setdicdir(path):
  global HANGUL_DIC_DIR
  HANGUL_DIC_DIR = path

@memoized
def converter():
  import os
  from pyhangul import HangulHanjaConverter
  path = os.path.join(HANGUL_DIC_DIR, HANGUL_DIC_CONV)
  ret = HangulHanjaConverter()
  if os.path.exists(path):
    ret.loadFile(path)
  else:
    from sakurakit.skdebug import derror
    derror("dic path does not exist:", path)
  return ret

def to_hanja(text):
  """
  @param  text  unicode
  @return  unicode
  """
  return converter().convert(text)

def to_hanja_list(text):
  """
  @param  text  unicode
  @return  [[unicode hangul, unicode hanja]]
  """
  return converter().parse(text)

if __name__ == '__main__':
  import os
  os.environ['PATH'] = os.path.pathsep.join((
    "../../../bin",
    "../../../../Qt/PySide",
    os.environ['PATH'],
  ))
  sys.path.append("../../../bin")
  sys.path.append("../../../../Qt/PySide")

  dic = "../../../../../Dictionaries/hangul"
  setdicdir(dic)

  # http://ko.wikipedia.org/wiki/자유_콘텐츠
  # 自由 콘텐츠는 著作權이 消滅된 퍼블릭 도메인은 勿論, 著作權이 있지만 위 基準에 따라 自由롭게 利用이 許諾된 콘텐츠도 包含한다."
  t = u"자유 콘텐츠는 저작권이 소멸된 퍼블릭 도메인은 물론, 저작권이 있지만 위 기준에 따라 자유롭게 이용이 허락된 콘텐츠도 포함한다."
  #  Supposed to be 利用이 instead of 이容易
  t = u"이용이"
  t = u"공주"
  r = to_hanja(t)

  from PySide.QtGui import *
  a = QApplication(sys.argv)
  w = QLabel(r)
  w.show()
  a.exec_()

# EOF
