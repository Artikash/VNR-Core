# coding: utf8
# topicsview.py
# 11/25/2014 jichi

if __name__ == '__main__':
  import sys
  sys.path.append('..')
  import debug
  debug.initenv()

import json
from functools import partial
from PySide.QtCore import Qt, QObject
from Qt5 import QtWidgets
from sakurakit import skevents, skfileio, skqss
from sakurakit.skclass import Q_Q, memoized, memoizedproperty
from sakurakit.skdebug import dprint, dwarn
from sakurakit.sktr import tr_
from sakurakit.skwebkit import SkWebView #, SkWebViewBean
from sakurakit.skwidgets import SkTitlelessDockWidget, SkStyleView, shortcut
#from sakurakit.skqml import QmlObject
from mytr import my, mytr_
import config, dataman, growl, netman, osutil, rc

@Q_Q
class _TopicsView(object):

  def __init__(self, q):
    self.gameId = 0 # long

    import comets
    self.comet = comets.createPostComet()
    qml = self.comet.q
    qml.topicDataReceived.connect(self._onTopicReceived)
    qml.topicDataUpdated.connect(self._onTopicUpdated)

    #self._viewBean = SkWebViewBean(self.webView)

    self._createUi(q)

    #shortcut('ctrl+n', self._new, parent=q)

  def _createUi(self, q):
    q.setCentralWidget(self.webView)

    dock = SkTitlelessDockWidget(self.inspector)
    dock.setFeatures(QtWidgets.QDockWidget.NoDockWidgetFeatures)
    #dock.setAllowedAreas(Qt.BottomDockWidgetArea)
    q.addDockWidget(Qt.BottomDockWidgetArea, dock)

  def _createDialog(self):
    import windows
    parent = windows.normal()
    ret = TopicsView(parent=parent)
    ret.resize(550, 580)
    return ret

  # append ;null for better performance
  def _onTopicReceived(self, data): # unicode json ->
    js = 'if (window.READY) addTopic(%s); null' % data
    self.webView.evaljs(js)
    dprint("pass")

  # append ;null for better performance
  def _onTopicUpdated(self, data): # unicode json ->
    js = 'if (window.READY) updateTopic(%s); null' % data
    self.webView.evaljs(js)
    dprint("pass")

  def setGameId(self, gameId): # long ->
    if self.gameId != gameId:
      self.gameId = gameId

      path = 'game/%s' % gameId if gameId else ''
      if path:
        self.comet.setPath(path)
        self.comet.setActive(True)
      else:
        self.comet.setActive(False)
        self.comet.setPath('')

  def clear(self):
    self.setGameId(0)
    self.webView.clear()

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
      ('i18nBean', m.i18nBean),
      ('mainBean', m.mainBean),
      ('topicEditBean', self.topicEditBean),
      ('topicInputBean', self.topicInputBean),
    )

  @memoizedproperty
  def topicEditBean(self):
    import topicedit
    return topicedit.TopicEditorManagerBean(parent=self.q, manager=self.topicEditorManager)

  @memoizedproperty
  def topicInputBean(self):
    import topicinput
    return topicinput.TopicInputManagerBean(parent=self.q, manager=self.topicInputManager)

  @memoizedproperty
  def webView(self):
    from PySide.QtWebKit import QWebPage
    ret = SkWebView()
    ret.titleChanged.connect(self.q.setWindowTitle)
    ret.enableHighlight() # highlight selected text
    ret.ignoreSslErrors() # needed to access Twitter

    ret.pageAction(QWebPage.Reload).triggered.connect(
        self.refresh, Qt.QueuedConnection)

    ret.page().setLinkDelegationPolicy(QWebPage.DelegateAllLinks) # Since there are local images
    ret.page().mainFrame().setScrollBarPolicy(Qt.Horizontal, Qt.ScrollBarAlwaysOff) # disable horizontal scroll

    #ret.page().setLinkDelegationPolicy(QWebPage.DelegateExternalLinks)
    ret.linkClicked.connect(osutil.open_url)
    return ret

  def refresh(self):
    """@reimp"""
    host = config.API_HOST # must be the same as rest.coffee for the same origin policy

    dm = dataman.manager()
    user = dm.user()
    info = dm.queryGameInfo(itemId=self.gameId)
    if info:
      title = info.title0 or tr_("Game")
      image = info.imageUrl0 if info.hasGoodImage0() else None
      icon = info.icon
    else:
      title = image = icon = None
    self.q.setWindowIcon(icon or rc.icon('window-review'))

    w = self.webView
    w.setHtml(rc.haml_template('haml/reader/topicsview').render({
      'host': host,
      'title': title,
      'gameId': self.gameId,
      'userName': user.name,
      'userPassword': user.password,
      'image': image, # background image
      'rc': rc,
      'tr': tr_,
    }), host)
    self._injectBeans()

    if not self.comet.isActive():
      dprint("reactivate comet")
      self.comet.setActive(True)

  @memoizedproperty
  def inspector(self):
    ret = SkStyleView()
    skqss.class_(ret, 'texture')
    layout = QtWidgets.QHBoxLayout()
    #layout.addWidget(self.newButton) # not enabled
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
        osutil.open_url("http://sakuradite.com/game/%s" % self.gameId))
    return ret

  #@memoizedproperty
  #def newButton(self):
  #  ret = QtWidgets.QPushButton(tr_("New"))
  #  skqss.class_(ret, 'btn btn-success')
  #  ret.setToolTip(tr_("New") + " (Ctrl+N)")
  #  #ret.setStatusTip(ret.toolTip())
  #  ret.clicked.connect(self._new)
  #  return ret

  @memoizedproperty
  def topicEditorManager(self):
    import topicedit
    ret = topicedit.TopicEditorManager(self.q)
    ret.topicChanged.connect(self._update)
    return ret

  @memoizedproperty
  def topicInputManager(self):
    import topicinput
    ret = topicinput.TopicInputManager(self.q)
    ret.topicReceived.connect(self._submit)
    return ret

  def _submit(self, topicData, imageData, ticketData):
    """
    @param  topicData  unicode json
    @param  imageData  unicode json
    @param  ticketData  unicode json
    """
    if self.gameId:
      user = dataman.manager().user()
      if user.name and user.password:
        topic = json.loads(topicData)
        topic['subjectId'] = self.gameId
        topic['subjectType'] = 'game'
        topic['login'] = user.name
        topic['password'] = user.password
        if imageData:
          image = json.loads(imageData)
          image['login'] = user.name
          image['password'] = user.password
        else:
          image = None
        if ticketData:
          tickets = []
          a = json.loads(ticketData)
          for k,v in a.iteritems():
            tickets.append({
              'type': k,
              'value': v,
              'targetId': self.gameId,
              'targetType': 'game',
              'login': user.name,
              'password': user.password,
            })
        else:
          tickets = None
        skevents.runlater(partial(self._submitTopic, topic, image, tickets))

  def _submitTopic(self, topic, image, tickets):
    """
    @param  topic  kw
    @param  image  kw or None
    @param  tickets [kw]
    """
    dprint("enter")
    nm = netman.manager()
    if tickets:
      for data in tickets:
        nm.updateTicket(data) # error not checked
    if image:
      data = skfileio.readdata(image['filename'])
      if data:
        topic['image'] = nm.submitImage(data, image)

    if image and not topic.get('image') or not nm.submitTopic(topic):
      growl.warn("<br/>".join((
        my.tr("Failed to submit topic"),
        my.tr("Please try again"),
      )))
    dprint("leave")

  def _update(self, topicData, imageData, ticketData):
    """
    @param  topicData  unicode  json
    @param  imageData  unicode  json
    @param  ticketData  unicode  json
    """
    if self.gameId:
      user = dataman.manager().user()
      if user.name and user.password:
        topic = json.loads(topicData)
        #topic['subjectId'] = self.gameId
        topic['login'] = user.name
        topic['password'] = user.password

        if imageData:
          image = json.loads(imageData)
          image['login'] = user.name
          image['password'] = user.password
        else:
          image = None

        tickets = None
        skevents.runlater(partial(self._updateTopic, topic, image, tickets))

  def _updateTopic(self, topic, image, tickets):
    """
    @param  topic  kw
    @param  image  kw or None
    @param  tickets  [kw] or None
    """
    dprint("enter")
    if image:
      data = skfileio.readdata(image['filename'])
      if data:
        topic['image'] = netman.manager().submitImage(data, image)

    if image and not topic.get('image') or not netman.manager().updateTopic(topic):
      growl.warn("<br/>".join((
        my.tr("Failed to update topic"),
        my.tr("Please try again"),
      )))
    dprint("leave")

  #def _new(self): self.topicInputManager.newTopic()

class TopicsView(QtWidgets.QMainWindow):
  def __init__(self, parent=None):
    WINDOW_FLAGS = Qt.Dialog|Qt.WindowMinMaxButtonsHint
    super(TopicsView, self).__init__(parent, WINDOW_FLAGS)
    self.setWindowIcon(rc.icon('window-review'))
    self.setWindowTitle(tr_("Review"))
    self.__d = _TopicsView(self)

  def refresh(self): self.__d.refresh()
  def clear(self): self.__d.clear()

  def gameId(self): return self.__d.gameId
  def setGameId(self, gameId): self.__d.setGameId(gameId)

  def setVisible(self, value):
    """@reimp @public"""
    if value and not self.isVisible():
      self.__d.refresh()
    super(TopicsView, self).setVisible(value)
    if not value:
      self.clear()

class _TopicsViewManager:
  def __init__(self):
    self.dialogs = []

  def _createDialog(self):
    import windows
    parent = windows.normal()
    ret = TopicsView(parent=parent)
    ret.resize(550, 580)
    return ret

  def getDialog(self, gameId=0):
    """
    @param  gameId  long
    """
    if gameId:
      for w in self.dialogs:
        if w.isVisible() and gameId == w.gameId():
          return w
    for w in self.dialogs:
      if not w.isVisible():
        return w
    ret = self._createDialog()
    self.dialogs.append(ret)
    return ret

class TopicsViewManager:
  def __init__(self):
    self.__d = _TopicsViewManager()

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

  def showGame(self, gameId):
    """
    @param  gameId  long
    """
    w = self.__d.getDialog(gameId)
    if w.gameId() == gameId:
      w.refresh()
    else:
      w.clear()
      w.setGameId(gameId)
    w.show()
    w.raise_()

@memoized
def manager():
  import webrc
  webrc.init()
  return TopicsViewManager()

#@QmlObject
#class TopicsViewManagerProxy(QObject):
#  def __init__(self, parent=None):
#    super(TopicsViewManagerProxy, self).__init__(parent)
#
#  @Slot(int)
#  def showGame(self, id):
#    manager().showGame(id)

if __name__ == '__main__':
  a = debug.app()
  manager().showGame(101)
  a.exec_()

# EOF
