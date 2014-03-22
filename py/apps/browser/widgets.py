# coding: utf8
# widgets.py
# 12/13/2012 jichi

__all__ = [
  'WbAddressEdit',
  'WbTabBar',
  'WbTabWidget',
]

from PySide.QtCore import Qt, Signal
from Qt5 import QtWidgets
from sakurakit.sktr import tr_

## WbAddressEdit ##

class WbAddressEdit(QtWidgets.QComboBox):
  def __init__(self, parent=None):
    super(WbAddressEdit, self).__init__(parent)
    self.setInsertPolicy(QtWidgets.QComboBox.InsertAtTop)
    self.setEditable(True)
    self.lineEdit().returnPressed.connect(self.enter)

  textEntered = Signal(unicode)

  def enter(self):
    t = self.currentText().strip()
    if t:
      self.textEntered.emit(t)

  def setUrl(self, url):
    if not self.hasFocus():
      self.setEditText(url)

## WbTabWidget ##

class WbTabBar(QtWidgets.QTabBar):
  def __init__(self, parent=None):
    super(WbTabBar, self).__init__(parent)
    self.setTabsClosable(True)

  doubleClickedAt = Signal(int) # index

  ## Events ##
  def mouseDoubleClickEvent(self, e):
    """@reimp"""
    if e.button() == Qt.LeftButton: #and not e.modifiers():
      index = self.tabAt(e.globalPos())
      if index >= 0:
        self.doubleClickedAt.emit(index)
      e.accept();
    else:
      super(WbTabBar, self).mouseDoubleClickEvent(e)

class WbTabWidget(QtWidgets.QTabWidget):
  def __init__(self, parent=None):
    super(WbTabWidget, self).__init__(parent)

  doubleClicked = Signal()
  rightButtonClicked = Signal()

  ## Events ##

  def mouseDoubleClickEvent(self, e):
    """@reimp"""
    if e.button() == Qt.LeftButton: # and not e.modifiers():
      self.doubleClicked.emit()
      e.accept()
    else:
      super(WbTabWidget, self).mouseDoubleClickEvent(e)

  def mouseReleaseEvent(self, e):
    """@reimp"""
    if e.button() == Qt.RightButton:
      self.rightButtonClicked.emit()
    super(WbTabWidget, self).mouseReleaseEvent(e)

  ## Actions ##

  def newTab(self, view, index=-1, focus=True):
    """
    @param  view  QWidget
    @param  index  int
    @param  focus  bool
    """
    title = tr_("Empty")
    if index < 0 or index >= self.count():
      index = self.addTab(view, title)
    else:
      self.insertTab(index, view, title)
    if focus:
      self.focusTab(index)

  def isEmpty(self): return self.count() <= 0

  def focusTab(self, index):
    """
    @param  index  int
    """
    if index >= 0 and index < self.count():
      self.setCurrentIndex(index)

# EOF
