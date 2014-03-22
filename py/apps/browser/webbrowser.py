# coding: utf8
# webbrowser.py
# 12/13/2012 jichi

__all__ = ['WebBrowser']

import re
from functools import partial
from PySide.QtCore import Qt, Signal, QUrl
from PySide import QtGui
from Qt5 import QtWidgets
from sakurakit import skqss
from sakurakit.skclass import memoizedproperty, Q_Q
from sakurakit.skwidgets import SkTitlelessDockWidget, SkDraggableMainWindow, shortcut
from sakurakit.sktr import tr_
from addressui import *
from netman import *
from tabui import *
from webkit import *
from i18n import i18n
import rc, textutil, ui

START_HTML = rc.jinja_template('start').render({
  'tr': tr_,
  'rc': rc,
}) # unicode html

MAX_TITLE_LENGTH = 20

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
      self.__d.openUnknown(url)
      #self.__d.openUrl(url)

@Q_Q
class _WebBrowser(object):
  def __init__(self, q):
    self.loadProgress = 100 # int [0,100]

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

    #self.newTabAfterCurrentWithBlankPage()

  def _createShortcuts(self):
    q = self.q
    shortcut(QtGui.QKeySequence.AddTab, self.newTabAfterCurrentWithBlankPage, parent=q)

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
    ret.setTabBar(self.tabBar)
    ret.setCornerWidget(self.newTabButton)
    ret.tabCloseRequested.connect(self.closeTab)
    ret.currentChanged.connect(self.loadAddress)
    ret.currentChanged.connect(self.refreshLoadProgress)
    ret.currentChanged.connect(self.refreshWindowTitle)
    ret.doubleClicked.connect(self.newTabAtLastWithBlankPage, Qt.QueuedConnection)
    return ret

  @memoizedproperty
  def tabBar(self):
    ret = WbTabBar()
    ret.setGraphicsEffect(ui.glowEffect(ret))
    #ret.doubleClickedAt.connect(self.newTabAfter, Qt.QueuedConnection)
    return ret

  @memoizedproperty
  def addressWidget(self):
    row = QtWidgets.QHBoxLayout()
    row.addWidget(self.addressToolBar)
    row.addWidget(self.addressEdit, 1)
    row.setContentsMargins(2, 2, 2, 2)
    ret = QtWidgets.QWidget()
    ret.setLayout(row)
    return ret

  @memoizedproperty
  def addressEdit(self):
    ret = WbAddressEdit()
    ret.setGraphicsEffect(ui.glowEffect(ret))
    skqss.class_(ret, "address-edit")
    # Not sure why that global shortcut does not work
    ret.textEntered.connect(self.openUnknown)
    ret.editTextChanged.connect(self.highlightText)
    return ret

  @memoizedproperty
  def newTabButton(self):
    ret = QtWidgets.QPushButton()
    ret.setGraphicsEffect(ui.glowEffect(ret))
    skqss.class_(ret, 'btn-tab-corner')
    ret.setText("+")
    #ret.setToolTip(tr_("New Tab"))
    ret.setToolTip("%s (%s, %s)" % (i18n.tr("New Tab"), "cmd+T", tr_("Double-click")))
    ret.clicked.connect(self.newTabAtLastWithBlankPage)
    return ret

  @memoizedproperty
  def addressToolBar(self):
    ret = QtWidgets.QToolBar()
    ret.setGraphicsEffect(ui.glowEffect(ret))
    skqss.class_(ret, 'toolbar-address')

    a = ret.addAction(u"\u25c0") # left triangle
    a.triggered.connect(self.back)
    a.setToolTip("%s (cmd+[)" % tr_("Back"))

    a = ret.addAction(u"\u25B6") # right triangle
    a.triggered.connect(self.forward)
    a.setToolTip("%s (cmd+])" % tr_("Forward"))

    #a = ret.addAction(u'\u27f3') # circle
    a = ret.addAction(u"◯") # まる
    a.triggered.connect(self.refresh)
    a.setToolTip("%s (cmd+R)" % tr_("Refresh"))
    return ret

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

  def openUrl(self, url, focus=True): # string ->
    """
    @param  url  unicode
    """
    self.addRecentUrl(url)
    if self.tabWidget.isEmpty():
      self.newTabAfterCurrent(focus=focus)
    v = self.tabWidget.currentWidget()
    v.load(url)

  def openUrlAfterCurrent(self, url, focus=False): # string ->
    """
    @param  url  unicode
    """
    self.addRecentUrl(url)
    v = self.newTabAfterCurrent(focus=focus)
    v.load(url)

  def addRecentUrl(self, url): # string|QUrl ->
    if isinstance(url, QUrl):
      url = url.toString()
    if url:
      url = textutil.simplifyurl(url)
      self.addressEdit.addUrl(url)

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

  def activateTab(self, index): # int ->
    if index >=0 and index < self.tabWidget.count():
      self.tabWidget.setCurrentIndex(index)

  def newTabAfterCurrentWithBlankPage(self):
    self.newTabAfterCurrent()
    self.openBlankPage()

  def newTabAtLastWithBlankPage(self):
    self.newTabAfterCurrent()
    self.openBlankPage()

  def newTabAfterCurrent(self, focus=True): # -> webview
    return self.newTabAfter(index=self.tabWidget.currentIndex(), focus=focus)

  def newTabAtLast(self, focus=True): # -> webview
    return self.newTabAfter(index=self.tabWidget.count() -1, focus=focus)

  def newTabAfter(self, index, focus=True): # -> webview
    w = self.createWebView()
    self.tabWidget.newTab(w, index=index+1, focus=focus)
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

    ret.titleChanged.connect(partial(self.setTabTitle, ret))
    ret.urlChanged.connect(self.refreshAddress)
    ret.messageReceived.connect(self.q.messageReceived)
    ret.linkClicked.connect(self.addRecentUrl)
    ret.linkClicked.connect(lambda url:
        url.isEmpty() or self.setDisplayAddress(url))

    ret.titleChanged.connect(partial(lambda view, value:
        view == self.tabWidget.currentWidget() and self.refreshWindowTitle(),
        ret))
    page.loadProgress.connect(partial(lambda view, value:
        view == self.tabWidget.currentWidget() and self.refreshLoadProgress(),
        ret))

    return ret

  def showMessage(self, t): # unicode ->
      self.q.messageReceived.emit(t)

  def showLink(self, url, content): # unicode, unicode
    if url:
      self.showMessage(textutil.simplifyurl(url))

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

  def tabTitle(self, index=-1): # int -> unicode
    w = self.tabWidget
    if index == -1:
      index = w.currentIndex()
    return w.tabToolTip(index) if index >=0 and index < w.count() else ''

  def refreshWindowTitle(self):
    t = self.tabTitle()
    if not t:
      t = u"Kagami (α)"
    self.q.setWindowTitle(t)

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
    if isinstance(url, QUrl):
      url = '' if url.isEmpty() else url.toString()
    url = textutil.simplifyurl(url)
    self.addressEdit.setUrl(url)

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
        self.tabWidget.removeTab(index)
        #w.stop() # does not work!
        w.clear()

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
