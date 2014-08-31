# coding: utf8
# postinput.py
# 8/30/2014 jichi

__all__ = ['PostInputManager', 'PostInputManagerBean']

if __name__ == '__main__':
  import sys
  sys.path.append('..')
  import debug
  debug.initenv()

import json
from PySide.QtCore import Qt, Signal, Slot, QObject
from Qt5 import QtWidgets
from sakurakit import skqss, skwidgets
from sakurakit.skclass import Q_Q, memoizedproperty
from sakurakit.skdebug import dwarn
from sakurakit.sktr import tr_
from mytr import mytr_, my
import config, growl, i18n, rc

TEXTEDIT_MINIMUM_HEIGHT = 50

@Q_Q
class _PostInput(object):
  def __init__(self, q):
    self.clear()

    self._createUi(q)

    skwidgets.shortcut('ctrl+s', self._save, parent=q)

  def clear(self):
    self.postLanguage = ''
    self.postContent = ''

  def _createUi(self, q):
    layout = QtWidgets.QVBoxLayout()

    row = QtWidgets.QHBoxLayout()
    row.addWidget(QtWidgets.QLabel(tr_("Language") + ":"))
    row.addWidget(self.languageEdit)
    row.addStretch()
    layout.addLayout(row)

    layout.addWidget(self.contentEdit)

    row = QtWidgets.QHBoxLayout()
    row.addStretch()
    #row.addWidget(self.cancelButton)
    row.addWidget(self.saveButton)
    layout.addLayout(row)

    layout.setContentsMargins(5, 5, 5, 5)
    q.setLayout(layout)

  @memoizedproperty
  def saveButton(self):
    ret = QtWidgets.QPushButton(tr_("Submit"))
    skqss.class_(ret, 'btn btn-primary')
    ret.setToolTip(tr_("Submit") + " (Ctrl+S)")
    ret.setDefault(True)
    ret.clicked.connect(self._save)
    ret.clicked.connect(self.q.hide) # save and hide
    return ret

  @memoizedproperty
  def cancelButton(self):
    ret = QtWidgets.QPushButton(tr_("Cancel"))
    skqss.class_(ret, 'btn btn-default')
    ret.setToolTip(tr_("Cancel"))
    ret.clicked.connect(self.q.hide)
    return ret

  @memoizedproperty
  def contentEdit(self):
    ret = QtWidgets.QTextEdit()
    skqss.class_(ret, 'texture')
    ret.setToolTip(tr_("Content"))
    ret.setAcceptRichText(False)
    ret.setMinimumHeight(TEXTEDIT_MINIMUM_HEIGHT)
    ret.textChanged.connect(self._onContentChanged)
    return ret

  @memoizedproperty
  def spellHighlighter(self):
    import spell
    return spell.SpellHighlighter(self.contentEdit)

  @memoizedproperty
  def languageEdit(self):
    ret = QtWidgets.QComboBox()
    ret.setEditable(False)
    ret.addItems(map(i18n.language_name2, config.LANGUAGES))
    ret.setMaxVisibleItems(ret.count())
    ret.currentIndexChanged.connect(self._onLanguageChanged)
    return ret

  def _getLanguage(self):
    return config.language2htmllocale(config.LANGUAGES[self.languageEdit.currentIndex()])
  def _getContent(self):
    return self.contentEdit.toPlainText().strip()

  def _onContentChanged(self):
    self.saveButton.setEnabled(bool(self._getContent()))

  def _onLanguageChanged(self):
    self.spellHighlighter.setLanguage(self._getLanguage())

  def _save(self):
    post = {}
    post['content'] = self.postContent = self._getContent()
    post['lang'] = self.postLanguage = self._getLanguage()

    #import dataman
    #user = dataman.manager().user()
    #post['login'] = user.name
    #post['pasword'] = user.password

    if post['content']:
      self.q.postReceived.emit(json.dumps(post))
      #self.postContent = '' # clear content but leave language

      growl.msg(my.tr("Edit submitted"))

  def refresh(self):
    self.saveButton.setEnabled(False)

    self.contentEdit.setPlainText(self.postContent)

    try: langIndex = config.LANGUAGES.index(config.htmllocale2language(self.postLanguage))
    except ValueError: langIndex = 1 # 'en'
    self.languageEdit.setCurrentIndex(langIndex)

    self.spellHighlighter.setLanguage(self.postLanguage) # must after lang

class PostInput(QtWidgets.QDialog):

  postReceived = Signal(unicode) # json

  def __init__(self, parent=None):
    WINDOW_FLAGS = Qt.Dialog | Qt.WindowMinMaxButtonsHint
    super(PostInput, self).__init__(parent, WINDOW_FLAGS)
    skqss.class_(self, 'texture')
    self.setWindowTitle(mytr_("New Post"))
    self.setWindowIcon(rc.icon('window-textedit'))
    self.__d = _PostInput(self)
    #self.statusBar() # show status bar

    import netman
    netman.manager().onlineChanged.connect(lambda online: online or self.hide())
    import dataman
    dataman.manager().loginChanged.connect(lambda name, password: name or self.hide())

  #def clear(self): self.__d.clear()

  def setVisible(self, value):
    """@reimp @public"""
    if value and not self.isVisible():
      self.__d.refresh()
    super(PostInput, self).setVisible(value)

class _PostInputManager:
  def __init__(self):
    self.dialogs = []

  @staticmethod
  def _createDialog():
    import windows
    parent = windows.top()
    ret = PostInput(parent)
    ret.resize(300, 200)
    return ret

  def getDialog(self, q): # QObject -> QWidget
    for w in self.dialogs:
      if not w.isVisible():
        #w.clear() # use last input
        return w
    ret = self._createDialog()
    self.dialogs.append(ret)
    ret.postReceived.connect(q.postReceived)
    return ret

#@Q_Q
class PostInputManager(QObject):
  def __init__(self, parent=None):
    super(PostInputManager, self).__init__(parent)
    self.__d = _PostInputManager()

    from PySide.QtCore import QCoreApplication
    qApp = QCoreApplication.instance()
    qApp.aboutToQuit.connect(self.hide)

    import dataman, netman
    netman.manager().onlineChanged.connect(lambda t: t or self.hide())
    dataman.manager().loginChanged.connect(lambda t: t or self.hide())

  postReceived = Signal(unicode) # json

  #def clear(self): self.hide()

  def hide(self):
    if self.__d.dialogs:
      for w in self.__d.dialogs:
        if w.isVisible():
          w.hide()

  def newPost(self):
    w = self.__d.getDialog(self)
    w.show()

#@memoized
#def manager(): return PostInputManager()

#@QmlObject
class PostInputManagerBean(QObject):
  def __init__(self, parent=None):
    super(PostInputManagerBean, self).__init__(parent)
    self.__d = PostInputManager(self)
    self.__d.postReceived.connect(self.postReceived)

  postReceived = Signal(unicode) # json

  @Slot()
  def newPost(self): self.__d.newPost()

if __name__ == '__main__':
  a = debug.app()
  m = PostInputManager()
  m.newPost()
  a.exec_()

# EOF
