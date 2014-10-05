# coding: utf8
# syntaxtest.py
# 10/4/2014 jichi

__all__ = 'SyntaxTester',

if __name__ == '__main__':
  import sys
  sys.path.append('..')
  import debug
  debug.initenv()

import re
from PySide.QtCore import Qt
from Qt5 import QtWidgets
from sakurakit import skqss
from sakurakit.skclass import memoizedproperty
from sakurakit.skdebug import dprint
from sakurakit.sktr import tr_
from mytr import my
import rc

class _SyntaxTester(object):

  def __init__(self, q):
    self._createUi(q)
    self._refresh()

  def _createUi(self, q):
    self.sourceView.setPlainText(u"""こんにちは""")

    self.patternEdit.setText(u"白")
    self.replaceEdit.setText("White")

    for sig in (
        self.sourceView.textChanged,
        self.patternEdit.textChanged,
        self.replaceEdit.textChanged,
        ):
      sig.connect(self._refresh)

    layout = QtWidgets.QVBoxLayout()

    grid = QtWidgets.QGridLayout()

    # 0
    grid.addWidget(QtWidgets.QLabel(tr_("Pattern") + ":"), 0, 0)
    grid.addWidget(self.patternEdit, 0, 1)

    # 1
    grid.addWidget(QtWidgets.QLabel(tr_("Translation") + ":"))
    grid.addWidget(self.replaceEdit)

    layout.addLayout(grid)

    splitter = QtWidgets.QSplitter(Qt.Vertical)
    splitter.addWidget(self.sourceView)
    splitter.addWidget(self.syntaxView)
    splitter.addWidget(self.targetView)
    layout.addWidget(splitter)
    q.setLayout(layout)

  def _refresh(self):
    """
    @param  text  unicode
    @return  unicode
    """
    pass

  @memoizedproperty
  def sourceView(self):
    ret = QtWidgets.QPlainTextEdit()
    skqss.class_(ret, 'normal')
    ret.setToolTip(tr_("Input"))
    return ret

  @memoizedproperty
  def syntaxView(self):
    ret = QtWidgets.QPlainTextEdit()
    ret.setReadOnly(True)
    skqss.class_(ret, 'normal')
    ret.setToolTip(my.tr("Syntax parse tree"))
    return ret

  @memoizedproperty
  def targetView(self):
    ret = QtWidgets.QTextEdit()
    ret.setReadOnly(True)
    skqss.class_(ret, 'texture')
    ret.setToolTip(tr_("Output"))
    return ret

  @memoizedproperty
  def patternEdit(self):
    ret = QtWidgets.QLineEdit()
    skqss.class_(ret, 'normal')
    ret.setToolTip(my.tr("Subtree to match"))
    ret.setPlaceholderText(ret.toolTip())
    return ret

  @memoizedproperty
  def replaceEdit(self):
    ret = QtWidgets.QLineEdit()
    skqss.class_(ret, 'normal')
    ret.setToolTip(my.tr("Subtree to create"))
    ret.setPlaceholderText(ret.toolTip())
    return ret

  def refreshEnabled(self):
    pass

# I have to use QMainWindow, or the texture will not work
class SyntaxTester(QtWidgets.QDialog):

  def __init__(self, parent=None):
    WINDOW_FLAGS = Qt.Dialog | Qt.WindowMinMaxButtonsHint
    super(SyntaxTester, self).__init__(parent, WINDOW_FLAGS)
    skqss.class_(self, 'texture')
    self.__d = _SyntaxTester(self)
    self.setWindowTitle(my.tr("Test Japanese Syntax"))
    self.setWindowIcon(rc.icon('window-regexp'))
    self.resize(380, 350)
    dprint("pass")

  def setVisible(self, t):
    """@reimp"""
    if t:
      self.__d.refreshEnabled()
    super(SyntaxTester, self).setVisible(t)

if __name__ == '__main__':
  a = debug.app()
  w = SyntaxTester()
  w.show()
  a.exec_()

# EOF
