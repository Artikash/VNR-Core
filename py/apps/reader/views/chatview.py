# coding: utf8
# chatview.py
# 8/27/2014 jichi

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
import main, osutil, py, rc

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
      ('trBean', m.trBean),
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
        self.updateAndRefresh, Qt.QueuedConnection)

    ret.page().setLinkDelegationPolicy(QWebPage.DelegateAllLinks) # Since there are local images
    #ret.page().setLinkDelegationPolicy(QWebPage.DelegateExternalLinks)
    ret.linkClicked.connect(osutil.open_url)
    return ret

  def refresh(self):
    """@reimp"""
    q = self.q
    info = dm.queryGameInfo(itemId=self.itemId, id=self.gameId, cache=False)
    self._gameBean.info = info
    if info:
      if not self.gameId:
        self.gameId = info.gameId
      if not self.itemId:
        self.itemId = info.itemId

    title = info.title if info else ""
    if not title and self.gameId:
      title = dm.queryGameName(id=self.gameId)

    t = title or tr_("Game")
    if info and info.itemId:
      t += " #%s" % info.itemId
    if info:
      if info.upcoming:
        t += u" (未発売)"
      elif info.recent:
        t += u" (新作)"
    q.setWindowTitle(t)

    icon = info.icon if info else None
    q.setWindowIcon(icon or rc.icon('window-ChatView'))

    self.editButton.setEnabled(bool(self.gameId))
    self.launchButton.setEnabled(bool(info and info.local))
    self.discussButton.setEnabled(bool(info and info.itemId))

    # Fake base addr that twitter javascript can access document.cookie
    #baseUrl = ''
    #baseUrl = "http://localhost:6100"
    #baseUrl = "http://www.amazon.co.jp" # cross domain so that amazon iframe works
    #baseUrl = 'qrc://'     # would crash QByteArray when refresh
    #baseUrl = 'qrc://any'  # would crash QByteArray when refresh
    baseUrl = 'qrc:///_'    # any place is fine
    #baseUrl = 'file:///'    # would crash QByteArray when refresh
    #baseUrl = 'file:///any' # any place that is local

    w = self.webView
    w.setHtml(rc.haml_template('haml/game').render({
      'title': title,
      'game': info,
      #'cache': cacheman.CacheJinjaUtil,
      'rc': rc,
      'py': py,
      'tr': tr_,
      #'i18n': i18n,
      'jlp': mecabman.manager(),
      #'settings', settings.global_(),
      'online': netman.manager().isOnline(),
      #'proxy': proxy.manager(),
    }), baseUrl)

    self._injectBeans()

  @memoizedproperty
  def inspector(self):
    ret = SkStyleView()
    skqss.class_(ret, 'texture')
    layout = QtWidgets.QHBoxLayout()
    layout.addStretch()
    layout.addWidget(self.newButton)
    #layout.addWidget(self.refreshButton) # disabled
    layout.addWidget(self.browseButton)
    ret.setLayout(layout)
    layout.setContentsMargins(4, 4, 4, 4)
    return ret

  @memoizedproperty
  def browseButton(self):
    ret = QtWidgets.QPushButton(tr_("Browse"))
    skqss.class_(ret, 'btn btn-info')
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
      d.webView.clear()

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

# EOF
