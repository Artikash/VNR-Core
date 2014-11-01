# coding: utf8
# postedit.py
# 6/30/2014 jichi

__all__ = 'PostEditorManager', 'PostEditorManagerBean'

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
class _PostEditor(object):
  def __init__(self, q):
    self.postId = 0 # long
    self.userName = '' # unicode
    #self.userName = '' # unicode
    self.postLanguage = ''
    self.postContent = ''

    self._createUi(q)

    skwidgets.shortcut('ctrl+s', self._save, parent=q)

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
    ret = QtWidgets.QPushButton(tr_("Save"))
    skqss.class_(ret, 'btn btn-primary')
    ret.setToolTip(tr_("Save") + " (Ctrl+S)")
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
    #skqss.class_(ret, 'texture')
    skqss.class_(ret, 'edit edit-default')
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

  def _isChanged(self):
    t = self._getContent()
    return bool(t) and t != self.postContent or self.postLanguage != self._getLanguage()

  def _onContentChanged(self):
    self.saveButton.setEnabled(self._canSave())

  def _canSave(self): # -> bool
    t = self._getContent()
    return len(t) >= config.POST_CONTENT_MIN_LENGTH and len(t) <= config.POST_CONTENT_MAX_LENGTH and self._isChanged()

  def _onLanguageChanged(self):
    self.spellHighlighter.setLanguage(self._getLanguage())
    self.saveButton.setEnabled(self._canSave())

  def _save(self):
    v = self._getContent()
    post = {}
    if v and v != self.postContent:
      post['content'] = self.postContent = v
    v = self._getLanguage()
    if v != self.postLanguage:
      post['lang'] = self.postLanguage = v
    if post:
      post['id'] = self.postId
      post['userName'] = self.userName
      self.q.postChanged.emit(json.dumps(post))

      growl.msg(my.tr("Edit submitted"))

  def refresh(self):
    self.saveButton.setEnabled(False)

    self.contentEdit.setPlainText(self.postContent)

    try: langIndex = config.LANGUAGES.index(config.htmllocale2language(self.postLanguage))
    except ValueError: langIndex = 1 # 'en'
    self.languageEdit.setCurrentIndex(langIndex)

    self.spellHighlighter.setLanguage(self.postLanguage) # must after lang

class PostEditor(QtWidgets.QDialog):

  postChanged = Signal(unicode) # json

  def __init__(self, parent=None):
    WINDOW_FLAGS = Qt.Dialog|Qt.WindowMinMaxButtonsHint
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

  def setPost(self, id, userName='', language='', lang='', content='', **ignored):
    d = self.__d
    d.postId = id
    d.userName = userName
    d.postLanguage = language or lang
    d.postContent = content

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
    ret.resize(300, 200)
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

    from PySide.QtCore import QCoreApplication
    qApp = QCoreApplication.instance()
    qApp.aboutToQuit.connect(self.hide)

    import dataman, netman
    netman.manager().onlineChanged.connect(lambda t: t or self.hide())
    dataman.manager().loginChanged.connect(lambda t: t or self.hide())

  postChanged = Signal(unicode) # json

  #def clear(self): self.hide()

  def isVisible(self):
    if self.__d.dialogs:
      for w in self.__d.dialogs:
        if w.isVisible():
          return True
    return False

  def hide(self):
    if self.__d.dialogs:
      for w in self.__d.dialogs:
        if w.isVisible():
          w.hide()

  def editPost(self, **post):
    w = self.__d.getDialog(self)
    w.setPost(**post)
    w.show()

#@memoized
#def manager(): return PostEditorManager()

#@QmlObject
class PostEditorManagerBean(QObject):
  def __init__(self, parent=None, manager=None):
    super(PostEditorManagerBean, self).__init__(parent)
    self.manager = manager or PostEditorManager(self)
    self.manager.postChanged.connect(self.postChanged)

  postChanged = Signal(unicode) # json

  @Slot(unicode)
  def editPost(self, data): # json ->
    try:
      post = json.loads(data)
      post['id'] = long(post['id'])
      self.manager.editPost(**post)
    except Exception, e: dwarn(e)

if __name__ == '__main__':
  a = debug.app()
  m = PostEditorManager()
  m.editPost(id=123, content="123", lang='en')
  a.exec_()

# EOF
