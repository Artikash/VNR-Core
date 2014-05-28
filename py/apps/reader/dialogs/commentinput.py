# coding: utf8
# commentinput.py
# 5/28/2014 jichi

__all__ = ['CommentInputDialog']

if __name__ == '__main__':
  import sys
  sys.path.append('..')
  import debug
  debug.initenv()

from Qt5 import QtWidgets
from sakurakit import skqss
from sakurakit.skdebug import dprint
from sakurakit.sktr import tr_
from mytr import mytr_

class CommentInputDialog(QtWidgets.QDialog):

  def __init__(self, parent=None):
    super(CommentInputDialog, self).__init__(parent)
    skqss.class_(self, 'texture')
    self.setWindowTitle(tr_("Comment"))
    #self.setWindowIcon(rc.icon('window-shortcuts'))
    self.__d = _CommentInputDialog(self)
    #self.resize(300, 250)
    #self.statusBar() # show status bar

  def __del__(self):
    """@reimp"""
    dprint("pass")

  @classmethod
  def getComment(cls, parent=None, default=""):
    """
    @param  parent  QWidget
    @param  default  str
    @return  bool ok, unicode comment, {type:str, append:bool}
    """
    w = cls(parent)
    ok = True
    comment = default
    opt = {
      'type': 'comment',
      'append': True,
    }
    r = w.exec_()
    if parent:
      w.setParent(None)
    ok = r == 0
    return ok, comment, opt

#@Q_Q
class _CommentInputDialog(object):
  def __init__(self, q):
    self._createUi(q)

  def _createUi(self, q):
    self.edit = QtWidgets.QLineEdit()

    self.okButton = QtWidgets.QPushButton(tr_("OK"))
    self.cancelButton = QtWidgets.QPushButton(tr_("Cancel"))

    grid = QtWidgets.QGridLayout()
    r = 0
    self.commentButton = QtWidgets.QRadioButton(tr_("Comment"))
    grid.addWidget(self.commentButton, r, 0)
    self.updateCommentButton = QtWidgets.QRadioButton(mytr_("Update reason"))
    grid.addWidget(self.updateCommentButton, r, 1)

    r += 1
    self.appendButton = QtWidgets.QRadioButton(tr_("Append"))
    grid.addWidget(self.appendButton, r, 0)
    self.overwriteButton = QtWidgets.QRadioButton(tr_("Overwrite"))
    grid.addWidget(self.overwriteButton, r, 1)

    layout = QtWidgets.QVBoxLayout()
    layout.addWidget(self.edit)

    optionGroup = QtWidgets.QGroupBox(tr_("Option"))
    optionGroup.setLayout(grid)
    layout.addWidget(optionGroup)

    buttonBox = QtWidgets.QDialogButtonBox(
        QtWidgets.QDialogButtonBox.Ok|
        QtWidgets.QDialogButtonBox.Cancel)
    layout.addWidget(buttonBox)

    okButton = buttonBox.button(QtWidgets.QDialogButtonBox.Ok)
    cancelButton = buttonBox.button(QtWidgets.QDialogButtonBox.Cancel)

    q.setLayout(layout)

if __name__ == '__main__':
  a = debug.app()
  r = CommentInputDialog.getComment()
  #a.exec_()

# EOF
