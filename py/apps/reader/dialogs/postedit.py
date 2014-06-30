# coding: utf8
# posteditor.py
# 6/30/2014 jichi

__all__ = ['PostEditorManager', 'PostEditorManagerProxy']

if __name__ == '__main__':
  import sys
  sys.path.append('..')
  import debug
  debug.initenv()

import json
from PySide.QtCore import Qt, Signal, Slot, QObject
from Qt5 import QtWidgets
from sakurakit import skqss
from sakurakit.skclass import Q_Q, memoized, memoizedproperty
from sakurakit.skdebug import dwarn
from sakurakit.sktr import tr_
from mytr import mytr_
import config, i18n, rc

TEXTEDIT_MINIMUM_HEIGHT = 50

EDITABLE_FIELDS = 'lang', 'content'

@Q_Q
class _PostEditor(object):
  def __init__(self, q):
    self.postId = 0 # long
    self.userName = '' # unicode
    #self.userName = '' # unicode
    for k in EDITABLE_FIELDS:
      pty = 'post' + k.capitalize()
      setattr(self, pty, '') # unicode

    self._createUi(q)

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
    row.addWidget(self.cancelButton)
    row.addWidget(self.saveButton)
    layout.addLayout(row)

    q.setLayout(layout)

  @memoizedproperty
  def saveButton(self):
    ret = QtWidgets.QPushButton(tr_("Save"))
    skqss.class_(ret, 'btn btn-primary')
    ret.setToolTip(tr_("Save"))
    ret.setDefault(True)
    ret.clicked.connect(self._save)
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
    ret.setToolTip(tr_("Content"))
    ret.setAcceptRichText(False)
    ret.setMinimumHeight(TEXTEDIT_MINIMUM_HEIGHT)
    ret.textChanged.connect(self._onEdit)
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
    ret.currentIndexChanged.connect(self._onEdit)
    return ret

  def _getLang(self):
    return config.language2htmllocale(config.LANGUAGES[self.languageEdit.currentIndex()])
  def _getContent(self):
    return self.contentEdit.toPlainText().strip()

  def _isChanged(self):
    t = self._getContent()
    return bool(t) and t != self.postContent or self.postLang != self._getLang()

  def _onEdit(self):
    self.saveButton.setEnabled(self._isChanged())

  def _save(self):
    v = self._getContent()
    post = {}
    if v and v != self.postContent:
      post['content'] = self.postContent = v
    v = self._getLang()
    if v != self.postLang:
      post['lang'] = self.postLang = v
    if post:
      post['id'] = self.postId
      post['userName'] = self.userName
      self.q.postChanged.emit(json.dumps(post))

  def refresh(self):
    self.saveButton.setEnabled(False)

    self.contentEdit.setPlainText(self.postContent)
    self.spellHighlighter.setLanguage(self.postLang)

    try: langIndex = config.LANGUAGES.index(config.htmllocale2language(self.postLang))
    except ValueError: langIndex = 1 # 'en'
    self.languageEdit.setCurrentIndex(langIndex)

class PostEditor(QtWidgets.QDialog):

  postChanged = Signal(unicode) # json

  def __init__(self, parent=None):
    WINDOW_FLAGS = Qt.Dialog | Qt.WindowMinMaxButtonsHint
    super(PostEditor, self).__init__(parent, WINDOW_FLAGS)
    skqss.class_(self, 'texture')
    self.setWindowTitle(mytr_("Post Editor"))
    self.setWindowIcon(rc.icon('window-textedit'))
    self.__d = _PostEditor(self)
    #self.statusBar() # show status bar

    import netman
    netman.manager().onlineChanged.connect(lambda online: online or self.hide())
    import dataman
    dataman.manager().loginChanged.connect(lambda name, password: name or self.hide())

  def setPost(self, id, userName='', **kwargs):
    d = self.__d
    d.postId = id
    d.userName = userName

    for k in EDITABLE_FIELDS:
      pty = 'post' + k.capitalize()
      setattr(d, pty, kwargs.get(k))
    if self.isVisible():
      d.refresh()

  def setVisible(self, value):
    """@reimp @public"""
    if value and not self.isVisible():
      self.__d.refresh()
    super(PostEditor, self).setVisible(value)

class _PostEditorManager:
  def __init__(self):
    self.dialogs = []

  @staticmethod
  def _createDialog():
    import windows
    parent = windows.top()
    ret = PostEditor(parent)
    ret.resize(400, 200)
    return ret

  def getDialog(self, q): # QObject -> QWidget
    for w in self.dialogs:
      if not w.isVisible():
        return w
    ret = self._createDialog()
    self.dialogs.append(ret)
    ret.postChanged.connect(q.postChanged)
    return ret

#@Q_Q
class PostEditorManager(QObject):
  def __init__(self, parent=None):
    super(PostEditorManager, self).__init__(parent)
    self.__d = _PostEditorManager()

  postChanged = Signal(unicode) # json

  def clear(self): self.hide()

  def hide(self):
    if self.__d.dialogs:
      for w in self.__d.dialogs:
        if w.isVisible():
          w.hide()

  def editPost(self, **post):
    w = self.__d.getDialog(self)
    w.setPost(**post)
    w.show()

@memoized
def manager(): return PostEditorManager()

#@QmlObject
class PostEditorManagerProxy(QObject):
  def __init__(self, parent=None):
    super(PostEditorManagerProxy, self).__init__(parent)

    manager().postChanged.connect(self.postChanged)

  postChanged = Signal(unicode) # json

  @Slot(unicode)
  def editPost(self, data): # json ->
    try:
      post = json.loads(data)
      post['id'] = long(post['id'])
      manager().editPost(**post)
    except Exception, e: dwarn(e)

if __name__ == '__main__':
  a = debug.app()
  manager().editPost(id=123, content="123", lang='en')
  a.exec_()

# EOF
