# coding: utf8
# nameinput.py
# 1/3/2015 jichi

__all__ = 'NameInput',

if __name__ == '__main__':
  import sys
  sys.path.append('..')
  import debug
  debug.initenv()

#import re
#from functools import partial
from PySide.QtCore import Qt #, Signal
from Qt5 import QtWidgets
from sakurakit import skqss
from sakurakit.skclass import Q_Q, memoizedproperty
from sakurakit.sktr import tr_
from sakurakit.skwidgets import SkLayoutWidget
from mytr import my #, mytr_
import rc

#class NameInput(QtWidgets.QDialog):
class NameInput(QtWidgets.QScrollArea):
  #termEntered = Signal(QtCore.QObject) # Term

  def __init__(self, parent=None):
    WINDOW_FLAGS = Qt.Dialog|Qt.WindowMinMaxButtonsHint
    super(NameInput, self).__init__(parent)
    self.setWindowFlags(WINDOW_FLAGS)
    skqss.class_(self, 'texture')
    self.setWindowTitle(my.tr("Create dictionary entry"))
    self.setWindowIcon(rc.icon('window-name'))
    self.setWidgetResizable(True) # automatically stretch widgets
    self.__d = _NameInput(self)
    self.resize(600, 300)

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
    d = self.__d
    for it in d.iterNameYomi(name, yomi):
      d.addName(*it)

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

#@Q_Q
class _NameInput(object):
  def __init__(self, q):
    self.tokenId = 0 # long

    self.rows = [] # [{'name':unicode, 'yomi':unicode, 'info':unicode, 'nameButton':QPushButton, 'yomiButton':QPushButton, 'nameLabel':QLabel, 'yomiLabel':QLabel, 'infoLabel':QLabel}]
    self.visibleRowCount = 0 # int

    self.grid = QtWidgets.QGridLayout()

    self._createLayout(q)

  def _createLayout(self, q):
    #q.setLayout(self.grid)
    w = SkLayoutWidget(self.grid)
    q.setWidget(w)

  def clear(self):
    if self.visibleRowCount:
      for i in range(self.visibleRowCount):
        for w in self.rows[i].itervalues():
          if isinstance(w, QtWidgets.QWidget):
            w.hide()
      self.visibleRowCount = 0

  def refresh(self):
    pass

  def iterNameYomi(self, name, yomi):
    """
    @param  name  unicode
    @param  yomi  unicode
    @yield  (unicode name, unicode yomi, unicode info)
    """
    yield name, yomi, 'info'

  def addName(self, name, yomi, info):
    """
    @param  name  unicode
    @param  yomi  unicode
    @param  info  unicode
    """
    nameButton = self._createNameButton()
    yomiButton = self._createYomiButton()

    nameLabel = self._createNameLabel(name)
    yomiLabel = self._createYomiLabel(yomi if yomi and yomi != name else '')
    infoLabel = self._createInfoLabel(info)

    rowCount = len(self.rows)
    c = 0
    self.grid.addWidget(nameButton, rowCount, c)
    c += 1
    self.grid.addWidget(yomiButton, rowCount, c)
    c += 1
    self.grid.addWidget(nameLabel, rowCount, c)
    c += 1
    self.grid.addWidget(yomiLabel, rowCount, c)
    c += 1
    self.grid.addWidget(infoLabel, rowCount, c)

    row = {
      'name': name,
      'yomi': yomi,
      'info': info,
      'nameButton': nameButton,
      'yomiButton': yomiButton,
      'nameLabel': nameLabel,
      'yomiLabel': yomiLabel,
      'infoLabel': infoLabel,
    }

    self.rows.append(row)
    self.visibleRowCount += 1

  def _createNameButton(self):
    ret = QtWidgets.QPushButton(u"+kanji")
    return ret

  def _createYomiButton(self):
    ret = QtWidgets.QPushButton(u"+yomi")
    return ret

  def _createNameLabel(self, t):
    return self._createLabel(t)
  def _createYomiLabel(self, t):
    return self._createLabel(t)
  def _createInfoLabel(self, t):
    return self._createLabel(t)

  def _createLabel(self, t):
    ret = QtWidgets.QLineEdit(t)
    ret.setReadOnly(True)
    return ret

if __name__ == '__main__':
  names = [
    (u"桜小路 ルナ", u"さくらこうじ ルナ"),
    (u"エスト・ギャラッハ・アーノッツ", ""),
  ]
  a = debug.app()
  w = NameInput()
  w.setNames(names)
  w.show()
  a.exec_()

# EOF
