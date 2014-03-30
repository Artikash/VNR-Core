# coding: utf8
# webbrowser.py
# 12/13/2012 jichi

__all__ = ['WebBrowser']

import re, weakref
from functools import partial
from PySide.QtCore import Qt, Signal, QUrl
from PySide import QtGui
from Qt5 import QtWidgets
from sakurakit import skfileio, skqss, skos
from sakurakit.skclass import memoizedproperty, Q_Q
from sakurakit.skdebug import dprint
from sakurakit.skwidgets import SkTitlelessDockWidget, SkDraggableMainWindow, shortcut
from sakurakit.sktr import tr_
from addressui import *
from network import *
from tabui import *
from webkit import *
from i18n import i18n
import config, proxy, rc, settings, textutil, ui

START_HTML = rc.jinja_template('start').render({
  'tr': tr_,
  'rc': rc,
}) # unicode html

MAX_TITLE_LENGTH = 20

EMPTY_URL = "about:blank"

def _urltext(url): # unicode|QUrl -> unicode
  if isinstance(url, QUrl):
    url = proxy.fromproxyurl(url) or url
    url = url.toString()
  return textutil.simplifyurl(url) if url else ''

#class WebBrowser(QtWidgets.QMainWindow):
class WebBrowser(SkDraggableMainWindow):

  quitRequested = Signal()
  messageReceived = Signal(unicode)

  def __init__(self, parent=None):
    #WINDOW_FLAGS = (
    #  Qt.Window
    #  | Qt.CustomizeWindowHint
    #  | Qt.WindowTitleHint
    #  | Qt.WindowSystemMenuHint
    #  | Qt.WindowMinMaxButtonsHint
    #  | Qt.WindowCloseButtonHint
    #)
    super(WebBrowser, self).__init__(parent)
    self.__d = _WebBrowser(self)

  def showStatusMessage(self, t, type='message'):
    """
    @param  t  unicode
    @param  type  'message', 'warning', or 'error'
    """
    self.statusBar().showMessage(text)

  def openDefaultPage(self):
    self.__d.newTabAfterCurrentWithBlankPage()

  def openUrls(self, urls): # [unicode url]
    for url in urls:
      self.__d.openUnknownAfterCurrent(url)
      #self.__d.openUrl(url)

  def loadTabs(self): # -> bool
    return self.__d.loadTabs()

@Q_Q
class _WebBrowser(object):
  def __init__(self, q):
    self.loadProgress = 100 # int [0,100]

    self.visitedUrls = [] # [str url]
    self.closedUrls = [] # [str url]

    #layout = QtWidgets.QVBoxLayout()
    #layout.addWidget(self.addressWidget)
    #layout.addWidget(self.tabWidget)

    #w = QtWidgets.QWidget()
    #w.setLayout(layout)
    #layout.setContentsMargins(0, 0, 0, 0)
    #q.setCentralWidget(w)

    q.setCentralWidget(self.tabWidget)
    dock = SkTitlelessDockWidget(self.addressWidget)
    dock.setFeatures(QtWidgets.QDockWidget.NoDockWidgetFeatures)
    dock.setAllowedAreas(Qt.TopDockWidgetArea)
    q.addDockWidget(Qt.TopDockWidgetArea, dock)

    self._createShortcuts()

    from PySide.QtCore import QCoreApplication
    qApp = QCoreApplication.instance()
    qApp.aboutToQuit.connect(self._onQuit)

    #self.newTabAfterCurrentWithBlankPage()

    self.loadVisitedUrls()
    self.loadClosedUrls()

  def _onQuit(self):
    dprint("enter")
    self.saveTabs()
    self.saveVisitedUrls()
    self.saveClosedUrls()
    dprint("exit")

  def _createShortcuts(self):
    q = self.q
    shortcut(QtGui.QKeySequence.AddTab, self.newTabAfterCurrentWithBlankPage, parent=q)

    shortcut('ctrl+shift+t', self.undoCloseTab, parent=q)

    shortcut('ctrl+w', self.closeCurrentTab, parent=q)
    for k in 'ctrl+l', 'alt+d':
      shortcut(k, self.addressEdit.focus, parent=q)

    for i in range(1, 10):
      shortcut('ctrl+%i' % i, partial(self.activateTab, i-1), parent=q)
    #shortcut('ctrl+0', partial(self.activateTab, 10-9), parent=q) # ctrl+ 0 used by zoom reset

  ## Properties ##

  @memoizedproperty
  def networkAccessManager(self):
    ret = WbNetworkAccessManager(self.q)
    return ret

  @memoizedproperty
  def tabWidget(self):
    ret = WbTabWidget()
    skqss.class_(ret, 'webkit')
    ret.setTabBar(self.tabBar)
    if skos.MAC:
      ret.setDocumentMode(True) # only useful on mac, must comes after tabbar is set. This will disable double-click event on tabbar
    ret.setCornerWidget(self.newTabButton)
    ret.tabCloseRequested.connect(self.closeTab)
    ret.currentChanged.connect(self.loadAddress)
    ret.currentChanged.connect(self.refreshLoadProgress)
    ret.currentChanged.connect(self.refreshWindowTitle)
    ret.currentChanged.connect(self.refreshWindowIcon)
    ret.doubleClicked.connect(self.newTabAtLastWithBlankPage)
    return ret

  @memoizedproperty
  def tabBar(self):
    ret = WbTabBar()
    skqss.class_(ret, 'webkit')
    # FIXME: Enable glowing effect will cause issue for Flash
    #ret.setGraphicsEffect(ui.glowEffect(ret))
    #ret.doubleClickedAt.connect(self.newTabAfter)
    return ret

  @memoizedproperty
  def addressWidget(self):
    row = QtWidgets.QHBoxLayout()
    row.addWidget(self.navigationToolBar)
    row.addWidget(self.addressEdit, 1)
    row.addWidget(self.optionToolBar)
    row.setContentsMargins(2, 2, 2, 2)
    ret = QtWidgets.QWidget()
    ret.setLayout(row)
    return ret

  @memoizedproperty
  def addressEdit(self):
    ret = WbAddressEdit()
    ret.setGraphicsEffect(ui.glowEffect(ret))
    skqss.class_(ret, 'webkit address')
    # Not sure why that global shortcut does not work
    ret.textEntered.connect(self.openUnknown)
    ret.editTextChanged.connect(self.highlightText)
    return ret

  @memoizedproperty
  def newTabButton(self):
    ret = QtWidgets.QPushButton()
    ret.setGraphicsEffect(ui.glowEffect(ret))
    skqss.class_(ret, 'webkit btn-tab-corner')
    ret.setText("+")
    #ret.setToolTip(tr_("New Tab"))
    ret.setToolTip("%s (%s, %s)" % (i18n.tr("New Tab"), "Ctrl+T", tr_("Double-click")))
    ret.clicked.connect(self.newTabAtLastWithBlankPage)
    return ret

  @memoizedproperty
  def navigationToolBar(self):
    ret = QtWidgets.QToolBar()
    ret.setGraphicsEffect(ui.glowEffect(ret))
    skqss.class_(ret, 'webkit toolbar toolbar-nav')

    a = ret.addAction(u"\u25c0") # left triangle
    a.triggered.connect(self.back)
    a.setToolTip("%s (Ctrl+[, Alt+Left)" % tr_("Back"))

    a = ret.addAction(u"\u25B6") # right triangle
    a.triggered.connect(self.forward)
    a.setToolTip("%s (Ctrl+], Alt+Right)" % tr_("Forward"))

    #a = ret.addAction(u'\u27f3') # circle
    a = ret.addAction(u"◯") # まる
    a.triggered.connect(self.refresh)
    a.setToolTip("%s (Ctrl+R)" % tr_("Refresh"))

    #a = ret.addAction(u"\u238c")
    a = ret.addAction(u"←") # ひだり
    a.triggered.connect(self.undoCloseTab)
    a.setToolTip("%s (Ctrl+Shift+T)" % i18n.tr("Undo close tab"))
    return ret

  @memoizedproperty
  def optionToolBar(self):
    ret = QtWidgets.QToolBar()
    ret.setGraphicsEffect(ui.glowEffect(ret))
    skqss.class_(ret, 'webkit toolbar toolbar-opt')

    import settings
    ss = settings.global_()

    import jlpman
    JLP_ENABLED = jlpman.manager().isAvailable()

    a = ret.addAction(u"あ")
    a.setCheckable(True)
    a.setToolTip(i18n.tr("Toggle Japanese parser"))
    a.setEnabled(JLP_ENABLED)
    a.setChecked(ss.isMeCabEnabled())
    a.triggered[bool].connect(ss.setMeCabEnabled)
    a.triggered[bool].connect(self._onJlpToggled)

    a = ret.addAction(u"♪") # おんぷ
    a.setCheckable(True)
    a.setToolTip("%s (TTS)" % i18n.tr("Toggle text-to-speech") )
    a.setEnabled(JLP_ENABLED)
    a.setChecked(ss.isTtsEnabled())
    a.triggered[bool].connect(ss.setTtsEnabled)
    a.triggered[bool].connect(self._onTtsToggled)
    return ret

  ## JLP ##

  def _onJlpToggled(self, t):
    pass

  def _onTtsToggled(self, t):
    pass

  ## Load/save ##

  def loadTabs(self): # -> bool
    ret = False
    data = skfileio.readfile(rc.TABS_LOCATION)
    if data:
      urls = data.split('\n')
      for url in urls:
        self.openUnknownAfterCurrent(url)
      ret = True
    dprint("pass: ret = %s" % ret)
    return ret

  def saveTabs(self): # -> bool
    ret = False
    w = self.tabWidget
    urls = [] # [unicode url]
    for i in xrange(w.count()):
      v = w.widget(i)
      url = v.url().toString()
      if url != EMPTY_URL:
        urls.append(url)
    path = rc.TABS_LOCATION
    if urls:
      data = '\n'.join(urls)
      ret = skfileio.writefile(path, data)
    else:
      skfileio.removefile(path)
    dprint("pass: ret = %s" % ret)
    return ret

  def loadVisitedUrls(self):
    data = skfileio.readfile(rc.VISIT_HISTORY_LOCATION)
    if data:
      self.visitedUrls = data.split('\n')
      for url in reversed(self.visitedUrls):
        icon = rc.url_icon(url)
        self.addressEdit.addItem(icon, url)
    dprint("pass")

  def saveVisitedUrls(self):
    path = rc.VISIT_HISTORY_LOCATION
    if self.visitedUrls:
      from sakurakit import skcontainer
      l = skcontainer.uniquelist(reversed(self.visitedUrls))
      if len(l) > config.VISIT_HISTORY_SIZE:
        del l[config.VISIT_HISTORY_SIZE:]
      l.reverse()
      data = '\n'.join(l)
      skfileio.writefile(path, data)
    #else: # never not remove old history
    #  skfileio.removefile(path)
    dprint("pass")

  def loadClosedUrls(self):
    data = skfileio.readfile(rc.CLOSE_HISTORY_LOCATION)
    if data:
      self.closedUrls = data.split('\n')
    dprint("pass")

  def saveClosedUrls(self):
    path = rc.CLOSE_HISTORY_LOCATION
    if self.closedUrls:
      l = self.closedUrls
      if len(l) > config.CLOSE_HISTORY_SIZE:
        l = l[len(l) - config.CLOSE_HISTORY_SIZE:]
      data = '\n'.join(l)
      skfileio.writefile(path, data)
    else:
      skfileio.removefile(path)
    dprint("pass")

  ## Actions ##

  def highlightText(self, t):
    t = t.strip()
    if t:
      w = self.tabWidget.currentWidget()
      if w:
        w.rehighlight(t)

  def openUnknown(self, text): # string ->
    """
    @param  text  unicode
    """
    url = textutil.completeurl(text)
    self.openUrl(url)

  def openUnknownBeforeCurrent(self, text): # string ->
    """
    @param  text  unicode
    """
    url = textutil.completeurl(text)
    self.openUrlBeforeCurrent(url)

  def openUnknownAfterCurrent(self, text): # string ->
    """
    @param  text  unicode
    """
    url = textutil.completeurl(text)
    self.openUrlAfterCurrent(url)

  def openUrl(self, url, focus=True): # string ->
    """
    @param  url  unicode
    """
    self.addRecentUrl(url)
    if self.tabWidget.isEmpty():
      self.newTabAfterCurrent(focus=focus)
      v = self.tabWidget.currentWidget()
    else:
      v = self.tabWidget.currentWidget()
      v.stop()
    v.load(url)

  def openUrlBeforeCurrent(self, url, focus=True): # string ->
    """
    @param  url  unicode
    """
    self.addRecentUrl(url)
    v = self.newTabBeforeCurrent(focus=focus)
    v.load(url)

  def openUrlAfterCurrent(self, url, focus=False): # string ->
    """
    @param  url  unicode
    """
    self.addRecentUrl(url)
    v = self.newTabAfterCurrent(focus=focus)
    v.load(url)

  def addRecentUrl(self, url): # string|QUrl ->
    text = _urltext(url)
    if text:
      if text != EMPTY_URL:
        self.visitedUrls.append(text)
      self.addressEdit.addText(text)

      #urltext = url.toString() if isinstance(url, QUrl) else url
      #self.visitedUrls.append(urltext)

  def openBlankPage(self):
    if self.tabWidget.isEmpty():
      self.newTabAtLast()
    v = self.tabWidget.currentWidget()
    #assert v
    if v:
      v.setHtml(START_HTML)
      #self.tabWidget.setTabText(self.currentIndex(), tr("Start Page"));
      #int i = ui->addressEdit->findText(WB_BLANK_PAGE);
      #if (i >= 0)
      #  ui->addressEdit->setCurrentIndex(i);
      #ui->addressEdit->setIcon(WBRC_IMAGE_APP);

  def undoCloseTab(self):
    if self.closedUrls:
      url = self.closedUrls[-1]
      del self.closedUrls[-1]
      self.openUnknownBeforeCurrent(url)

  def activateTab(self, index): # int ->
    if index >=0 and index < self.tabWidget.count():
      self.tabWidget.setCurrentIndex(index)

  def newTabAfterCurrentWithBlankPage(self):
    self.newTabAfterCurrent()
    self.openBlankPage()

  def newTabAtLastWithBlankPage(self):
    self.newTabAtLast()
    self.openBlankPage()

  def newTabBeforeCurrent(self, focus=True): # -> webview
    return self.newTabBefore(index=self.tabWidget.currentIndex(), focus=focus)

  def newTabAfterCurrent(self, focus=True): # -> webview
    return self.newTabAfter(index=self.tabWidget.currentIndex(), focus=focus)

  def newTabAtLast(self, focus=True): # -> webview
    return self.newTabAfter(index=self.tabWidget.count() -1, focus=focus)

  def newTabAfter(self, index, focus=True): # -> webview
    index += 1
    index = min(max(0, index), self.tabWidget.count())
    w = self.createWebView()
    self.tabWidget.newTab(w, index=index, focus=focus)
    return w

  def newTabBefore(self, index, focus=True): # -> webview
    index = min(max(0, index), self.tabWidget.count())
    w = self.createWebView()
    self.tabWidget.newTab(w, index=index, focus=focus)
    return w

  def _createWindow(self, type): # QWebPage::WebWindowType -> QWebView
    return self.newTabAfterCurrent()

  def createWebView(self):
    ret = WbWebView()
    ret.onCreateWindow = self._createWindow

    page = ret.page()
    page.setNetworkAccessManager(self.networkAccessManager)
    page.linkHovered.connect(self.showLink)
    page.linkClickedWithModifiers.connect(self.openUrlAfterCurrent)

    ref = weakref.ref(ret)

    #ret.titleChanged.connect(partial(self.setTabTitle, ret))
    ret.titleChanged.connect(partial(lambda ref, v:
        self.setTabTitle(ref(), v),
        ref))

    ret.urlChanged.connect(self.refreshAddress)
    ret.messageReceived.connect(self.q.messageReceived)
    ret.linkClicked.connect(self.addRecentUrl)
    ret.linkClicked.connect(lambda url:
        url.isEmpty() or self.setDisplayAddress(url))

    ret.iconChanged.connect(partial(lambda ref:
        ref() == self.tabWidget.currentWidget() and self.refreshWindowIcon(),
        ref))

    ret.titleChanged.connect(partial(lambda ref, value:
        ref() == self.tabWidget.currentWidget() and self.refreshWindowTitle(),
        ref))
    page.loadProgress.connect(partial(lambda ref, value:
        ref() == self.tabWidget.currentWidget() and self.refreshLoadProgress(),
        ref))

    return ret

  def showMessage(self, t): # unicode ->
      self.q.messageReceived.emit(t)

  def showLink(self, url, content): # unicode, unicode
    text = _urltext(url)
    if text:
      self.showMessage(text)

  def forward(self):
    w = self.tabWidget.currentWidget()
    if w:
      w.forward()
  def back(self):
    w = self.tabWidget.currentWidget()
    if w:
      w.back()
  def refresh(self):
    w = self.tabWidget.currentWidget()
    if w:
      w.reload()

  def currentTabTitle(self): # -> unicode
    w = self.tabWidget.currentWidget()
    return w.title() if w else ''

  def currentTabIcon(self): # int -> unicode
    w = self.tabWidget.currentWidget()
    if w:
      icon = w.icon()
      if not icon:
        icon = rc.url_icon(w.url())
      return icon

  def refreshWindowTitle(self):
    t = self.currentTabTitle()
    if not t:
      t = u"Kagami (α)"
    self.q.setWindowTitle(t)

  def refreshWindowIcon(self):
    icon = self.currentTabIcon()
    self.addressEdit.setItemIcon(0, icon)
    self.q.setWindowIcon(icon)

  def refreshLoadProgress(self):
    v = self.loadProgress
    w = self.tabWidget.currentWidget()
    if w:
      v = w.page().progress()
    if self.loadProgress != v:
      self.loadProgress = v
      self.showProgressMessage()
      self.addressEdit.setProgress(v)

  def showProgressMessage(self):
    if self.loadProgress == 0:
      t = "%s ..." % i18n.tr("Loading")
    elif self.loadProgress == 100:
      t = i18n.tr("Loading complete")
    else:
      t = "%s ... %i/100" % (i18n.tr("Loading"), self.loadProgress)
    self.showMessage(t)

  def setDisplayAddress(self, url):
    text = _urltext(url)
    #if text:
    #self.addressEdit.setEditText(text)
    self.addressEdit.setText(text)

  def refreshAddress(self):
    v = self.tabWidget.currentWidget()
    if v:
      self.setDisplayAddress(v.url())

  def loadAddress(self):
    w = self.tabWidget.currentWidget()
    url = w.url() if w else ''
    self.setDisplayAddress(url)

  def closeTab(self, index):
    if self.tabWidget.count() <= 1:
      self.q.quitRequested.emit()
    else:
      if index >= 0 and index < self.tabWidget.count():
        w = self.tabWidget.widget(index)
        url = w.url()
        self.tabWidget.removeTab(index)

        w.clear() # enforce clear flash
        w.setParent(None) # only needed in PySide, otherwise the parent is QStackWidget

        #url = url.toString()
        url = _urltext(url)
        if url != EMPTY_URL:
          self.closedUrls.append(url)

  def closeCurrentTab(self):
    self.closeTab(self.tabWidget.currentIndex())

  def setTabTitle(self, tab, title):
    """
    @param  tab  index or QWidget
    @param  title  unicode
    """
    tw = self.tabWidget
    if not isinstance(tab, int) and not isinstance(tab, long):
      tab = tw.indexOf(tab)
    if not title:
      title = tr_("Empty")
    if tab >= 0 and tab < self.tabWidget.count():
      tw.setTabToolTip(tab, title)
      tw.setTabText(tab, self.shortenTitle(title))

  @staticmethod
  def shortenTitle(t):
    if len(t) < MAX_TITLE_LENGTH:
      return t
    else:
      return t[:MAX_TITLE_LENGTH - 2] + ' ...'

# EOF
