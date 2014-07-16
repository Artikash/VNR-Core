# coding: utf8
# topicview.py
# 7/16/2014 jichi

__all__ = ['TopicViewManager']

import json, os
from functools import partial
from PySide.QtCore import Qt, Slot, QObject
from Qt5 import QtWidgets
from sakurakit import skdatetime, skevents, skthreads, skqss
from sakurakit.skclass import Q_Q, memoized, memoizedproperty
from sakurakit.skdebug import dprint, dwarn
from sakurakit.sktr import tr_
from sakurakit.skwebkit import SkWebView, SkWebViewBean
from sakurakit.skwidgets import SkTitlelessDockWidget, SkStyleView, shortcut
#from sakurakit.skqml import QmlObject
from mytr import my, mytr_
import cacheman, dataman, features, growl, i18n, jsonutil, main, mecabman, netman, osutil, prompt, proxy, py, rc

class TopicCoffeeBean(QObject):
  def __init__(self, parent=None):
    super(TopicCoffeeBean, self).__init__(parent)
    self.info = info # GameItemInfo or None

@Q_Q
class _TopicView(object):

  def __init__(self, q):
    self.clear()
    self._locked = False

    self._topicBean = TopicCoffeeBean(parent=q)
    self._viewBean = SkWebViewBean(self.webView)

    self._createUi(q)

    shortcut('alt+n', self._newPost, parent=q)

  def _createUi(self, q):
    q.setCentralWidget(self.webView)

    dock = SkTitlelessDockWidget(self.inspector)
    dock.setFeatures(QtWidgets.QDockWidget.NoDockWidgetFeatures)
    #dock.setAllowedAreas(Qt.BottomDockWidgetArea)
    q.addDockWidget(Qt.BottomDockWidgetArea, dock)

  def clear(self):
    self.gameId = 0 # long
    self.itemId = 0 # long

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
      ('viewBean', self._viewBean),
      ('topicBean', self._topicBean), # do it at last
    )

  @memoizedproperty
  def webView(self):
    from PySide.QtWebKit import QWebPage
    ret = SkWebView()
    #ret.titleChanged.connect(self.q.setWindowTitle)
    ret.enableHighlight() # highlight selected text
    #ret.ignoreSslErrors() # needed to access Twitter

    ret.pageAction(QWebPage.Reload).triggered.connect(
        self.updateAndRefresh, Qt.QueuedConnection)

    #ret.page().setLinkDelegationPolicy(QWebPage.DelegateAllLinks) # Since there are local images
    ret.page().setLinkDelegationPolicy(QWebPage.DelegateExternalLinks)
    ret.linkClicked.connect(osutil.open_url)
    return ret

  def updateAndRefresh(self):
    if self._locked:
      dwarn("locked")
      return
    gameId = self.gameId
    if not gameId:
      #growl.notify(my.tr("Unknown game. Please try updating the database."))
      return
    self._locked = True

    if netman.manager().isOnline():
      #growl.msg(my.tr("Updating game information"))
      ok = dataman.manager().updateReferences(gameId)
      #dm.touchGames()
      if not ok:
        growl.notify(my.tr("The game title it not specified. You can click the Edit button to add one."))
      #else:
        #growl.msg(my.tr("Found game information"))

    self._locked = False
    self.refresh()
    #skevents.runlater(partial(
    #    dm.updateReferences, g.id),
    #    200)

  def refresh(self):
    """@reimp"""
    q = self.q
    dm = dataman.manager()
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
    q.setWindowIcon(icon or rc.icon('window-topicview'))

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
      'cache': cacheman.CacheJinjaUtil,
      'rc': rc,
      'py': py,
      'tr': tr_,
      'i18n': i18n,
      'jlp': mecabman.manager(),
      #'settings', settings.global_(),
      'online': netman.manager().isOnline(),
      'proxy': proxy.manager(),
      'mainland': features.MAINLAND_CHINA,
    }), baseUrl)

    self._injectBeans()

  @memoizedproperty
  def inspector(self):
    ret = SkStyleView()
    skqss.class_(ret, 'texture')
    layout = QtWidgets.QHBoxLayout()
    layout.addWidget(self.replyButton)
    layout.addStretch()
    #layout.addWidget(self.helpButton)
    layout.addWidget(self.refreshButton)
    ret.setLayout(layout)
    layout.setContentsMargins(4, 4, 4, 4)
    return ret

  @memoizedproperty
  def replyButton(self):
    ret = QtWidgets.QPushButton(tr_("Reply"))
    skqss.class_(ret, 'btn btn-primary')
    ret.setToolTip(tr_("Reply") + " (Alt+N)")
    ret.setEnabled(False)
    #ret.setStatusTip(ret.toolTip())
    ret.clicked.connect(self._newPost)
    return ret

  def _newPost(self): main.manager().showReferenceView(gameId=self.gameId)

  @memoizedproperty
  def refreshButton(self):
    ret = QtWidgets.QPushButton(tr_("Refresh"))
    skqss.class_(ret, 'btn btn-default')
    ret.setToolTip(tr_("Refresh") + " (Ctrl+R)")
    #ret.setStatusTip(ret.toolTip())
    ret.clicked.connect(self.updateAndRefresh)

    nm = netman.manager()
    ret.setEnabled(nm.isOnline())
    nm.onlineChanged.connect(ret.setEnabled)
    return ret

  #@memoizedproperty
  #def helpButton(self):
  #  ret = QtWidgets.QPushButton(tr_("Help"))
  #  skqss.class_(ret, 'btn btn-default')
  #  ret.setToolTip(tr_("Help"))
  #  #ret.setStatusTip(ret.toolTip())
  #  ret.clicked.connect(main.manager().showReferenceHelp)
  #  return ret

class TopicView(QtWidgets.QMainWindow):
  def __init__(self, parent=None):
    WINDOW_FLAGS = Qt.Dialog | Qt.WindowMinMaxButtonsHint
    super(TopicView, self).__init__(parent, WINDOW_FLAGS)
    self.setWindowIcon(rc.icon('window-topicview'))
    self.setWindowTitle(tr_("Topic"))
    self.__d = _TopicView(self)

  def refresh(self): self.__d.refresh()
  def clear(self): self.__d.clear()

  def topicId(self): return self.__d.topicId
  def setTopicId(self, v): self.__d.topicId = v

  def setVisible(self, value):
    """@reimp @public"""
    d = self.__d
    if value and not self.isVisible():
      d.refresh()
      # TODO: Should I fetch initial JSON topic data here?
      #gameId = d.gameId
      #if gameId and netman.manager().isOnline():
      #  g = dataman.manager().queryGame(id=gameId)
      #  #if not g:
      #  #  skevents.runlater(d.updateAndRefresh, 2000) # 2 seconds
      #  #else:
      #  if g:
      #    t = g.refsUpdateTime
      #    now = skdatetime.current_unixtime()
      #    import config
      #    if t + config.APP_UPDATE_REFS_INTERVAL < now:
      #      g.refsUpdateTime = now
      #      skevents.runlater(d.updateAndRefresh, 2000) # 2 seconds
    super(TopicView, self).setVisible(value)
    if not value:
      d.webView.clear()

class _TopicViewManager:
  def __init__(self):
    self.dialogs = []

  def _createDialog(self):
    import windows
    parent = windows.normal()
    ret = TopicView(parent=parent)
    ret.resize(550, 580)
    return ret

  def getDialog(self, topicId=0):
    """
    @param* topicId  long
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

class TopicViewManager:
  def __init__(self):
    self.__d = _TopicViewManager()

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
  return TopicViewManager()

# EOF
