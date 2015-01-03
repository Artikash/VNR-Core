# coding: utf8
# nameinput.py
# 1/3/2015 jichi

__all__ = 'NameInput',

if __name__ == '__main__':
  import sys
  sys.path.append('..')
  import debug
  debug.initenv()

import re
from functools import partial
from PySide.QtCore import Qt #, Signal
from Qt5 import QtWidgets
from sakurakit import skdatetime, skevents, skqss
from sakurakit.skclass import Q_Q, memoizedproperty
from sakurakit.sktr import tr_
from mytr import my, mytr_
import config, convutil, dataman, growl, i18n, rc

#COMBOBOX_MAXWIDTH = 100
COMBOBOX_MAXWIDTH = 80

RE_SHORT_HIRAGANA = re.compile(u'^[あ-ん]{1,3}$')

#@Q_Q
class _NameInput(object):
  def __init__(self, q):
    self.tokenId = 0 # long

    self.grid = QtWidgets.QGridLayout()
    q.setLayout(self.grid)

  def clear(self):
    pass

  def refresh(self):
    pass

class NameInput(QtWidgets.QDialog):
  #termEntered = Signal(QtCore.QObject) # Term

  def __init__(self, parent=None):
    WINDOW_FLAGS = Qt.Dialog|Qt.WindowMinMaxButtonsHint
    super(NameInput, self).__init__(parent, WINDOW_FLAGS)
    skqss.class_(self, 'texture')
    self.setWindowTitle(my.tr("Create dictionary entry"))
    self.setWindowIcon(rc.icon('window-name'))
    self.__d = _NameInput(self)
    self.resize(300, 320)

  def clear(self):
    self.__d.clear()

  def setGameName(self, name):
    """
    @param  tokenId  long
    """
    self.setWindowTitle("%s - %s" %
        (my.tr("Create dictionary entry"), name))

  def setGameTokenId(self, tokenId):
    """
    @param  tokenId  long
    """
    self.__d.tokenId = tokenId

  def addName(self, name, yomi):
    """
    @param  name  unicode
    @param  yomi  unicode
    """
    pass

  def setNames(self, l):
    """
    @param  l  [unicode name, unicode yomi]
    """
    self.clear()
    for name,yomi in l:
      self.addName(name, yomi)

  def setVisible(self, value):
    """@reimp @public"""
    if value and not self.isVisible():
      self.__d.refresh()
    super(NameInput, self).setVisible(value)

  def showGameInfo(self, info):
    """
    @param  info  dataman.GameInfo
    """
    pass

if __name__ == '__main__':
  names = [
    (u"桜小路 ルナ", u"さくらこうじ ルナ"),
    (u"エスト・ギャラッハ・アーノッツ", None),
  ]
  a = debug.app()
  w = NameInput()
  w.setNames(names)
  w.show()
  a.exec_()

# EOF
