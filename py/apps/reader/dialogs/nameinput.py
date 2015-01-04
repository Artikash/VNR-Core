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
#from functools import partial
from PySide.QtCore import Qt #, Signal
from Qt5 import QtWidgets
from sakurakit import skqss
from sakurakit.skclass import Q_Q, memoizedproperty
from sakurakit.sktr import tr_
from sakurakit.skwidgets import SkLayoutWidget
from mytr import my, mytr_
import rc

_RE_NAME_SPLIT = re.compile(u"[ \u3000・=＝]")
def _split_name(s): # unicode -> [unicode]
  return filter(bool, _RE_NAME_SPLIT.split(s))

NAME_EDIT_MIN_WIDTH = 100
INFO_EDIT_MIN_WIDTH = 200

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

  #def setVisible(self, value):
  #  """@reimp @public"""
  #  if value and not self.isVisible():
  #    self.__d.refresh()
  #  super(NameInput, self).setVisible(value)

  def showGameInfo(self, info):
    """
    @param  info  dataman.GameInfo
    """
    pass

#@Q_Q
class _NameInput(object):
  def __init__(self, q):
    self.tokenId = 0 # long

    self.rows = [] # [{'name':unicode, 'yomi':unicode, 'info':unicode, 'nameButton':QPushButton, 'yomiButton':QPushButton, 'nameEdit':QLabel, 'yomiEdit':QLabel, 'infoEdit':QLabel}]
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

  #def refresh(self):
  #  pass

  def iterNameYomi(self, name, yomi):
    """
    @param  name  unicode
    @param  yomi  unicode
    @yield  (unicode name, unicode yomi, unicode info)
    """
    noyomi = not yomi or yomi == name
    info = name
    if not noyomi:
      info += " (%s)" % yomi
    yield name, yomi, info

    namelist = _split_name(name)
    if len(namelist) > 1:
      if noyomi:
        yield namelist[0], None, info
        yield namelist[-1], None, info
      else:
        yomilist = _split_name(yomi)
        if len(namelist) == len(yomilist):
          yield namelist[0], yomilist[0], info
          yield namelist[-1], yomilist[-1], info

  def addName(self, name, yomi, info):
    """
    @param  name  unicode
    @param  yomi  unicode
    @param  info  unicode
    @return  kw
    """
    row = self._getNextRow()
    row['name'] = name
    row['yomi'] = yomi
    row['info'] = info
    row['nameEdit'].setText(name)
    row['yomiEdit'].setText(yomi if yomi and yomi != name else '-')
    row['infoEdit'].setText(info)

  def _getNextRow(self):
    rowCount = len(self.rows)
    if rowCount > self.visibleRowCount:
      row = self.rows[self.visibleRowCount]
      self.visibleRowCount += 1
      for w in row.itervalues():
        if isinstance(w, QtWidgets.QWidget):
          w.show()
    else:
      nameButton = self._createNameButton(rowCount)
      yomiButton = self._createYomiButton(rowCount)

      nameEdit = self._createNameEdit()
      yomiEdit = self._createYomiEdit()
      infoEdit = self._createInfoEdit()

      c = 0
      self.grid.addWidget(nameButton, rowCount, c)
      c += 1
      self.grid.addWidget(yomiButton, rowCount, c)
      c += 1
      self.grid.addWidget(nameEdit, rowCount, c)
      c += 1
      self.grid.addWidget(yomiEdit, rowCount, c)
      c += 1
      self.grid.addWidget(infoEdit, rowCount, c)

      row = {
        'name': '',
        'yomi': '',
        'info': '',
        'nameButton': nameButton,
        'yomiButton': yomiButton,
        'nameEdit': nameEdit,
        'yomiEdit': yomiEdit,
        'infoEdit': infoEdit,
      }

      self.rows.append(row)
      self.visibleRowCount += 1

    return row

  def _createNameButton(self, index):
    return self._createButton(index, mytr_("Name"))
  def _createYomiButton(self, index):
    return self._createButton(index, mytr_("Yomi"))
  def _createButton(self, index, text):
    """
    @param  index  int
    @param  text  unicode
    @return  QPushButton
    """
    ret = QtWidgets.QPushButton(text)
    return ret

  def _createNameEdit(self):
    return self._createLabel('text-info', mytr_("Kanji"), NAME_EDIT_MIN_WIDTH)
  def _createYomiEdit(self):
    return self._createLabel('text-success', mytr_("Yomi"), NAME_EDIT_MIN_WIDTH)
  def _createInfoEdit(self):
    return self._createLabel('text-muted', tr_("Comment"), INFO_EDIT_MIN_WIDTH)
  def _createLabel(self, styleclass, tip, minwidth):
    """
    @param  styleClass  str
    @param  minimumWidth  int
    @return  QLineEdit
    """
    ret = QtWidgets.QLineEdit()
    ret.setReadOnly(True)
    ret.setMinimumWidth(minwidth)
    ret.setToolTip(tip)
    skqss.class_(ret, styleclass)
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
