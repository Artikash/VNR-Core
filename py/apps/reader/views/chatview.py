# coding: utf8
# chatview.py
# 8/27/2014 jichi

if __name__ == '__main__':
  import sys
  sys.path.append('..')
  import debug
  debug.initenv()

import json
#from functools import partial
from PySide.QtCore import Qt, Slot, QObject
from Qt5 import QtWidgets
from sakurakit import skqss
from sakurakit.skclass import Q_Q, memoized, memoizedproperty
from sakurakit.skdebug import dprint, dwarn
from sakurakit.sktr import tr_
from sakurakit.skwebkit import SkWebView #, SkWebViewBean
from sakurakit.skwidgets import SkTitlelessDockWidget, SkStyleView, shortcut
#from sakurakit.skqml import QmlObject
from mytr import my, mytr_
import osutil, rc

@Q_Q
class _ChatView(object):

  def __init__(self, q):
    self.clear()

    #self._viewBean = SkWebViewBean(self.webView)

    self._createUi(q)

    shortcut('ctrl+n', self._new, parent=q)

  def _createUi(self, q):
    q.setCentralWidget(self.webView)

    dock = SkTitlelessDockWidget(self.inspector)
    dock.setFeatures(QtWidgets.QDockWidget.NoDockWidgetFeatures)
    #dock.setAllowedAreas(Qt.BottomDockWidgetArea)
    q.addDockWidget(Qt.BottomDockWidgetArea, dock)

  def clear(self):
    self.topicId = 0 # long

  def _injectBeans(self):
    h = self.webView.page().mainFrame()
    #h.addToJavaScriptWindowObject('bean', self._webBean)
    for name,obj in self._beans:
      h.addToJavaScriptWindowObject(name, obj)

  @memoizedproperty
  def _beans(self):
    """
    return  [(unicode name, QObject bean)]
    """
    import coffeebean
    m = coffeebean.manager()
    return (
      ('cacheBean', m.cacheBean),
      #('trBean', m.trBean),
      #('viewBean', self._viewBean),
    )

  @memoizedproperty
  def webView(self):
    from PySide.QtWebKit import QWebPage
    ret = SkWebView()
    #ret.titleChanged.connect(self.q.setWindowTitle)
    ret.enableHighlight() # highlight selected text
    ret.ignoreSslErrors() # needed to access Twitter

    ret.pageAction(QWebPage.Reload).triggered.connect(
        self.refresh, Qt.QueuedConnection)

    ret.page().setLinkDelegationPolicy(QWebPage.DelegateAllLinks) # Since there are local images
    #ret.page().setLinkDelegationPolicy(QWebPage.DelegateExternalLinks)
    ret.linkClicked.connect(osutil.open_url)
    return ret

  def refresh(self):
    """@reimp"""
    #baseUrl = 'http://sakuradite.com'
    baseUrl = 'http://153.121.54.194' # must be the same as rest.coffee for the same origin policy

    w = self.webView
    w.setHtml(rc.haml_template('haml/reader/chat').render({
      'topicId': self.topicId,
      'title': tr_("Chat"),
      'rc': rc,
    }), baseUrl)
    self._injectBeans()

  @memoizedproperty
  def inspector(self):
    ret = SkStyleView()
    skqss.class_(ret, 'texture')
    layout = QtWidgets.QHBoxLayout()
    layout.addWidget(self.newButton)
    layout.addStretch()
    layout.addWidget(self.browseButton)
    layout.addWidget(self.refreshButton)
    ret.setLayout(layout)
    layout.setContentsMargins(4, 4, 4, 4)
    return ret

  @memoizedproperty
  def refreshButton(self):
    ret = QtWidgets.QPushButton(tr_("Refresh"))
    skqss.class_(ret, 'btn btn-info')
    ret.setToolTip(tr_("Refresh") + " (Ctrl+R)")
    #ret.setStatusTip(ret.toolTip())
    ret.clicked.connect(self.refresh)
    #nm = netman.manager()
    #ret.setEnabled(nm.isOnline())
    #nm.onlineChanged.connect(ret.setEnabled)
    return ret

  @memoizedproperty
  def browseButton(self):
    ret = QtWidgets.QPushButton(tr_("Browse"))
    skqss.class_(ret, 'btn btn-default')
    ret.setToolTip(tr_("Launch"))
    #ret.setStatusTip(ret.toolTip())
    ret.clicked.connect(lambda:
        osutil.open_url("http://sakuradite.com/topic/%s" % self.topicId))
    return ret

  @memoizedproperty
  def newButton(self):
    ret = QtWidgets.QPushButton(tr_("New"))
    skqss.class_(ret, 'btn btn-success')
    ret.setToolTip(tr_("Edit") + " (Ctrl+N)")
    #ret.setStatusTip(ret.toolTip())
    ret.clicked.connect(self._new)
    return ret

  def _new(self): main.manager().showReferenceView(gameId=self.gameId)

class ChatView(QtWidgets.QMainWindow):
  def __init__(self, parent=None):
    WINDOW_FLAGS = Qt.Dialog | Qt.WindowMinMaxButtonsHint
    super(ChatView, self).__init__(parent, WINDOW_FLAGS)
    #self.setWindowIcon(rc.icon('window-chat'))
    self.__d = _ChatView(self)

  def refresh(self): self.__d.refresh()
  def clear(self): self.__d.clear()

  def topicId(self): return self.__d.topicId
  def setTopicId(self, topicId): self.__d.topicId = topicId

  def setVisible(self, value):
    """@reimp @public"""
    if value and not self.isVisible():
      self.__d.refresh()
    super(ChatView, self).setVisible(value)
    if not value:
      self.__d.webView.clear()

class _ChatViewManager:
  def __init__(self):
    self.dialogs = []

  def _createDialog(self):
    import windows
    parent = windows.normal()
    ret = ChatView(parent=parent)
    ret.resize(550, 580)
    return ret

  def getDialog(self, topicId=0):
    """
    @param  topicId  long
    """
    if topicId:
      for w in self.dialogs:
        if w.isVisible() and topicId == w.topicId():
          return w
    for w in self.dialogs:
      if not w.isVisible():
        return w
    ret = self._createDialog()
    self.dialogs.append(ret)
    return ret

class ChatViewManager:
  def __init__(self):
    self.__d = _ChatViewManager()

  def clear(self): self.hide()

  def hide(self):
    if self.__d.dialogs:
      for w in self.__d.dialogs:
        if w.isVisible():
          w.hide()

  def showTopic(self, topicId):
    """
    @param  topicId  long
    """
    w = self.__d.getDialog(topicId)
    if w.topicId() == topicId:
      w.refresh()
    else:
      w.clear()
      w.setTopicId(topicId)
    w.show()
    w.raise_()

@memoized
def manager():
  import webrc
  webrc.init()
  return ChatViewManager()

#@QmlObject
class ChatViewManagerProxy(QObject):
  def __init__(self, parent=None):
    super(ChatViewManagerProxy, self).__init__(parent)

  @Slot(int)
  def showTopic(self, id):
    manager().showTopic(id)

if __name__ == '__main__':
  a = debug.app()
  manager().showTopic(409)
  a.exec_()

# EOF
