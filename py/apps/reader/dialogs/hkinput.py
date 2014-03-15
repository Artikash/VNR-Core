# coding: utf8
# hkinput.py
# 3/15/2014 jichi

__all__ = ['HotkeyInput']

if __name__ == '__main__':
  import sys
  sys.path.append('..')
  import debug
  debug.initenv()

from functools import partial
from PySide.QtCore import Qt, Signal
from Qt5 import QtWidgets
from sakurakit import skqss
from sakurakit.skclass import Q_Q, memoizedproperty
from sakurakit.sktr import tr_
from sakurakit.skwidgets import shortcut
from mytr import mytr_
import i18n, rc

@Q_Q
class _HotkeyInput(object):
  def __init__(self, q):
    self.defaultValue = ''
    self.keyButtons = [] # QPushButton
    self._createUi(q)

  def _createUi(self, q):
    grid = QtWidgets.QGridLayout()
    grid.setHorizontalSpacing(0)
    grid.setVerticalSpacing(0)

    # These keys must be consistent with pyhk

    KEYBOARD  = (
      ('Escape',  'F1', 'F2', 'F3', 'F4', 'F5', 'F6', 'F7', 'F8', 'F9', 'F10', 'F11', 'F12'), #'Escape'
      ('`',       '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 'Back'),
      ('Capital', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '\\'),
      (None,      'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', "'"), # '\n',
      (None,      'Z', 'X', 'C', 'V', 'B', 'N', 'M', ',', '.', '/'),
      ('Up', 'Down', 'Left', 'Right'),
    )

    r = 0
    for keys in KEYBOARD:
      for col,key in enumerate(keys):
        if key:
          grid.addWidget(self.createKeyButton(key), r, col)
      r += 1

    WIDE_COL = 2
    KEYBOARD2  = (
      ('Space', 'Insert', 'Delete', 'Home', 'End', 'Prior', 'Next'),
      ('mouse middle', 'mouse right'),
    )
    for keys in KEYBOARD2:
      for i,key in enumerate(keys):
        col = i * WIDE_COL
        if key:
          grid.addWidget(self.createKeyButton(key), r, col, 1, WIDE_COL)
      r += 1

    layout = QtWidgets.QVBoxLayout()
    layout.addLayout(grid)

    row = QtWidgets.QHBoxLayout()
    row.addStretch()
    row.addWidget(self.delButton)
    row.addWidget(self.cancelButton)
    row.addWidget(self.saveButton)
    layout.addLayout(row)

    q.setLayout(layout)

    shortcut('ctrl+s', self.save, parent=q)

  def createKeyButton(self, key): # str -> QPushButton
    ret = QtWidgets.QPushButton(i18n.key_name(key))
    ret.setToolTip(key)
    skqss.class_(ret, 'btn btn-default')
    ret.setCheckable(True)
    ret.value = key
    ret.clicked.connect(partial(self.setCurrentValue, key))
    self.keyButtons.append(ret)
    return ret

  @memoizedproperty
  def delButton(self):
    ret = QtWidgets.QPushButton(tr_("Delete"))
    skqss.class_(ret, 'btn btn-danger')
    ret.clicked.connect(self.delete)
    return ret

  @memoizedproperty
  def cancelButton(self):
    ret = QtWidgets.QPushButton(tr_("Cancel"))
    skqss.class_(ret, 'btn btn-default')
    ret.clicked.connect(self.q.hide)
    return ret

  @memoizedproperty
  def saveButton(self):
    ret = QtWidgets.QPushButton(tr_("Save"))
    ret.setToolTip("Ctrl+S")
    skqss.class_(ret, 'btn btn-primary')
    ret.clicked.connect(self.save)
    ret.setDefault(True)
    return ret

  def delete(self):
    self.q.hide()
    if self.currentValue():
      self.q.valueChanged.emit('')

  def save(self):
    self.q.hide()
    v = self.currentValue()
    if self.defaultValue != v:
      self.q.valueChanged.emit(v)

  def currentValue(self): # -> str
    for it in self.keyButtons:
      if it.isChecked():
        return it.value
    return ''

  def setCurrentValue(self, value): # str ->
    for it in self.keyButtons:
      if value == it.value:
        it.setChecked(True)
        #it.setFocus(Qt.MouseFocusReason)
      elif it.isChecked():
        it.setChecked(False)

  def refresh(self):
    self.setCurrentValue(self.defaultValue)

class HotkeyInput(QtWidgets.QDialog):
  valueChanged = Signal(str) # hotkey

  def __init__(self, parent=None):
    WINDOW_FLAGS = Qt.Dialog | Qt.WindowMinMaxButtonsHint
    super(HotkeyInput, self).__init__(parent, WINDOW_FLAGS)
    skqss.class_(self, 'texture')
    self.setWindowTitle(mytr_("Shortcuts"))
    self.setWindowIcon(rc.icon('window-shortcuts'))
    self.__d = _HotkeyInput(self)
    #self.resize(300, 250)
    #self.statusBar() # show status bar

  def setValue(self, v):
    self.__d.defaultValue = v

  def setVisible(self, t):
    if t:
      self.__d.refresh()
    super(HotkeyInput, self).setVisible(t)

if __name__ == '__main__':
  a = debug.app()
  w = HotkeyInput()
  w.show()
  a.exec_()

# EOF
