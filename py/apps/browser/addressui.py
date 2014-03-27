# coding: utf8
# addressui.py
# 12/13/2012 jichi

__all__ = ['WbAddressEdit']

from PySide import QtGui
from PySide.QtCore import Qt, Signal
from Qt5 import QtWidgets
from sakurakit.skclass import Q_Q

class WbAddressEdit(QtWidgets.QComboBox):
  def __init__(self, parent=None):
    super(WbAddressEdit, self).__init__(parent)
    self.__d = _WbAddressEdit(self)
    self.setInsertPolicy(QtWidgets.QComboBox.InsertAtTop)
    self.setEditable(True)
    #self.currentIndexChanged.connect(self.enter) # recursion
    self.lineEdit().returnPressed.connect(self.enter)

    self.completer().setCompletionMode(QtWidgets.QCompleter.PopupCompletion)

    self.maxCount = 20 # int  maximum number of items

    self.setToolTip("Ctrl+L, Alt+D")

  textEntered = Signal(unicode)

  def enter(self):
    t = self.currentText().strip()
    if t:
      self.textEntered.emit(t)

  def focus(self):
    self.setFocus()
    self.lineEdit().selectAll()

  #def setTextIfInactive(self, text): # unicode ->
  #  if not self.hasFocus():
  #    self.setEditText(text)

  def addText(self, text): # unicode ->
    index = self.findText(text)
    if index >= 0:
      self.removeItem(index)
    self.insertItem(0, text) # FIXME: This will change current item!
    if self.count() > self.maxCount:
      self.removeItem(self.maxCount)

  def setProgress(self, v): # int [0,100]
    d = self.__d
    if d.progress != v:
      d.progress = v
      d.refrehPallete()

@Q_Q
class _WbAddressEdit(object):

  def __init__(self):
    self.progress = 100 # int [0,100]

  def refrehPallete(self):
    q = self.q
    if self.progress == 100:
      q.setStyleSheet(q.styleSheet()) # invalidate pallete
    else:
      w = q.width()
      X_INIT = 0.1
      x = X_INIT + self.progress *((1 - X_INIT)/100.0)
      g = QtWidgets.QLinearGradient(0, 0, w*x, 0) # // horizental
      g.setColorAt(0, Qt.transparent)
      g.setColorAt(0.99, QtGui.QColor(113, 201, 244, 180)) # #71c9f4, blue
      g.setColorAt(1, Qt.transparent)
      p = QtWidgets.QPalette()
      p.setBrush(QtWidgets.QPalette.Base, g)
      #q.lineEdit().setPalette(p)
      q.setPalette(p)

# EOF
