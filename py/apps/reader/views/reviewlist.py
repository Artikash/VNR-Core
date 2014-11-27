# coding: utf8
# reviewlist.py
# 11/25/2014 jichi

if __name__ == '__main__':
  import sys
  sys.path.append('..')
  import debug
  debug.initenv()

import json
from functools import partial
from PySide.QtCore import Qt, Slot, QObject
from Qt5 import QtWidgets
from sakurakit import skevents, skfileio, skqss
from sakurakit.skclass import Q_Q, memoized, memoizedproperty
from sakurakit.skdebug import dprint, dwarn
from sakurakit.sktr import tr_
from sakurakit.skwebkit import SkWebView #, SkWebViewBean
from sakurakit.skwidgets import SkTitlelessDockWidget, SkStyleView, shortcut
#from sakurakit.skqml import QmlObject
from mytr import my, mytr_
import dataman, growl, netman, osutil, rc

@Q_Q
class _ReviewList(object):

  def __init__(self, q):
    self.gameId = 0 # long

    import comets
    self.comet = comets.createPostComet()
    qml = self.comet.q
    qml.topicDataReceived.connect(self._onTopicReceived)
    qml.topicDataUpdated.connect(self._onTopicUpdated)

    #self._viewBean = SkWebViewBean(self.webView)

    self._createUi(q)

    shortcut('ctrl+n', self._new, parent=q)

  def _createUi(self, q):
    q.setCentralWidget(self.webView)

    dock = SkTitlelessDockWidget(self.inspector)
    dock.setFeatures(QtWidgets.QDockWidget.NoDockWidgetFeatures)
    #dock.setAllowedAreas(Qt.BottomDockWidgetArea)
    q.addDockWidget(Qt.BottomDockWidgetArea, dock)

  def _createDialog(self):
    import windows
    parent = windows.normal()
    ret = ReviewList(parent=parent)
    ret.resize(550, 580)
    return ret

  # append ;null for better performance
  def _onTopicReceived(self, data): # unicode json ->
    js = 'if (window.READY) addTopic(%s); null' % data
    self.webView.evaljs(js)

  # append ;null for better performance
  def _onTopicUpdated(self, data): # unicode json ->
    js = 'if (window.READY) updateTopic(%s); null' % data
    self.webView.evaljs(js)

  def setGameId(self, gameId): # long ->
    if self.gameId != gameId:
      self.gameId = gameId

      path = '/game/%s' % gameId if gameId else ''
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
    import coffeebean, postedit, postinput
    m = coffeebean.manager()
    return (
      ('cacheBean', m.cacheBean),
      ('i18nBean', m.i18nBean),
      ('mainBean', m.mainBean),
      ('postEditBean', self.postEditBean),
      ('postInputBean', self.postInputBean),
    )

  @memoizedproperty
  def postEditBean(self):
    import postedit
    ret = postedit.PostEditorManagerBean(parent=self.q, manager=self.postEditorManager)
    return ret

  @memoizedproperty
  def postInputBean(self):
    import postinput
    ret = postinput.PostInputManagerBean(parent=self.q, manager=self.postInputManager)
    return ret

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
    #baseUrl = 'http://sakuradite.com'
    baseUrl = 'http://153.121.54.194' # must be the same as rest.coffee for the same origin policy
    #baseUrl = 'http://localhost:8080'

    dm = dataman.manager()
    user = dm.user()
    info = dm.queryGameInfo(itemId=self.gameId)
    title = info.title0 or tr_("Game")
    image = info.imageUrl0 if info.hasGoodImage0() else None

    w = self.webView
    w.setHtml(rc.haml_template('haml/reader/reviewlist').render({
      'title': title,
      'gameId': self.gameId,
      'userName': user.name,
      'userPassword': user.password,
      'image': image, # background image
      'rc': rc,
      'tr': tr_,
    }), baseUrl)
    self._injectBeans()

    if not self.comet.isActive():
      dprint("reactivate comet")
      self.comet.setActive(True)

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
        osutil.open_url("http://sakuradite.com/game/%s" % self.gameId))
    return ret

  @memoizedproperty
  def newButton(self):
    ret = QtWidgets.QPushButton(tr_("New"))
    skqss.class_(ret, 'btn btn-success')
    ret.setToolTip(tr_("New") + " (Ctrl+N)")
    #ret.setStatusTip(ret.toolTip())
    ret.clicked.connect(self._new)
    return ret

  @memoizedproperty
  def postEditorManager(self):
    import postedit
    ret = postedit.PostEditorManager(self.q)
    ret.postChanged.connect(self._update)
    return ret

  @memoizedproperty
  def postInputManager(self):
    import postinput
    ret = postinput.PostInputManager(self.q)
    ret.postReceived.connect(self._submit)
    return ret

  def _submit(self, postData, imageData):
    """
    @param  postData  unicode json
    @param  imageData  unicode
    """
    if self.gameId:
      user = dataman.manager().user()
      if user.name and user.password:
        post = json.loads(postData)
        post['topic'] = self.gameId
        post['login'] = user.name
        post['password'] = user.password
        if imageData:
          image = json.loads(imageData)
          image['login'] = user.name
          image['password'] = user.password
        else:
          image = None
        skevents.runlater(partial(self._submitPost, post, image))

  def _submitPost(self, post, image):
    """
    @param  post  kw
    @param  image  kw or None
    """
    dprint("enter")
    if image:
      data = skfileio.readdata(image['filename'])
      if data:
        post['image'] = netman.manager().submitImage(data, image)

    if image and not post.get('image') or not netman.manager().submitPost(post):
      growl.warn("<br/>".join((
        my.tr("Failed to submit post"),
        my.tr("Please try again"),
      )))
    dprint("leave")

  def _update(self, postData, imageData):
    """
    @param  postData  unicode json
    @param  imageData  unicode json
    """
    if self.gameId:
      user = dataman.manager().user()
      if user.name and user.password:
        post = json.loads(postData)
        #post['topic'] = self.gameId
        post['login'] = user.name
        post['password'] = user.password

        if imageData:
          image = json.loads(imageData)
          image['login'] = user.name
          image['password'] = user.password
        else:
          image = None
        skevents.runlater(partial(self._updatePost, post, image))

  def _updatePost(self, post, image):
    """
    @param  post  kw
    @param  image  kw or None
    """
    dprint("enter")
    if image:
      data = skfileio.readdata(image['filename'])
      if data:
        post['image'] = netman.manager().submitImage(data, image)

    if image and not post.get('image') or not netman.manager().updatePost(post):
      growl.warn("<br/>".join((
        my.tr("Failed to update post"),
        my.tr("Please try again"),
      )))
    dprint("leave")

  def _new(self): self.postInputManager.newPost()

class ReviewList(QtWidgets.QMainWindow):
  def __init__(self, parent=None):
    WINDOW_FLAGS = Qt.Dialog|Qt.WindowMinMaxButtonsHint
    super(ReviewList, self).__init__(parent, WINDOW_FLAGS)
    self.setWindowIcon(rc.icon('window-review'))
    self.setWindowTitle(tr_("Review"))
    self.__d = _ReviewList(self)

  def refresh(self): self.__d.refresh()
  def clear(self): self.__d.clear()

  def gameId(self): return self.__d.gameId
  def setGameId(self, gameId): self.__d.setGameId(gameId)

  def setVisible(self, value):
    """@reimp @public"""
    if value and not self.isVisible():
      self.__d.refresh()
    super(ReviewList, self).setVisible(value)
    if not value:
      self.clear()

  #def addPost(self, data): # unicode json ->
  #  self.__d.addPost(data)

  #def updatePost(self, data): # unicode json ->
  #  self.__d.updatePost(data)

class _ReviewListManager:
  def __init__(self):
    self.dialogs = []

    #import comets
    #comet = comets.globalComet()
    ##assert comet
    #if comet: # for debug purpose when comet is empty
    #  comet.postDataReceived.connect(self._onPostReceived)
    #  comet.postDataUpdated.connect(self._onPostUpdated)

  def _createDialog(self):
    import windows
    parent = windows.normal()
    ret = ReviewList(parent=parent)
    ret.resize(550, 580)
    return ret

  #def _onPostReceived(self, data):
  #  try:
  #    obj = json.loads(data)
  #    topicId = obj['topicId']
  #    for w in self.dialogs:
  #      if w.isVisible() and w.topicId() == topicId:
  #        w.addPost(data)
  #  except Exception, e:
  #    dwarn(e)

  #def _onPostUpdated(self, data):
  #  try:
  #    obj = json.loads(data)
  #    topicId = obj['topicId']
  #    for w in self.dialogs:
  #      if w.isVisible() and w.topicId() == topicId:
  #        w.updatePost(data)
  #  except Exception, e:
  #    dwarn(e)

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

class ReviewListManager:
  def __init__(self):
    self.__d = _ReviewListManager()

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
  return ReviewListManager()

#@QmlObject
class ReviewListManagerProxy(QObject):
  def __init__(self, parent=None):
    super(ReviewListManagerProxy, self).__init__(parent)

  @Slot(int)
  def showGame(self, id):
    manager().showGame(id)

if __name__ == '__main__':
  import config
  a = debug.app()
  manager().showGame(101)
  a.exec_()

# EOF
