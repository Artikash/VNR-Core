# coding: utf8
# webbrowser.py
# 12/13/2012 jichi

__all__ = ['WebBrowser']

import re
from functools import partial
from PySide.QtCore import Qt, Signal
from PySide import QtGui
from Qt5 import QtWidgets
from sakurakit.skclass import memoizedproperty, Q_Q
from sakurakit.skwidgets import SkTitlelessDockWidget, shortcut
from sakurakit.sktr import tr_
from netman import *
from webkit import *
from widgets import *
import textutil

class WebBrowser(QtWidgets.QMainWindow):

  quitRequested = Signal()

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

@Q_Q
class _WebBrowser(object):
  def __init__(self, q):
    q.setCentralWidget(self.tabWidget)

    dock = SkTitlelessDockWidget(self.header)
    dock.setFeatures(QtWidgets.QDockWidget.NoDockWidgetFeatures)
    dock.setAllowedAreas(Qt.TopDockWidgetArea)
    q.addDockWidget(Qt.TopDockWidgetArea, dock)

    self._createShortcuts()

    self.newTabAfterCurrentWithBlankPage()

  def _createShortcuts(self):
    q = self.q
    shortcut(QtGui.QKeySequence.AddTab, self.newTabAfterCurrentWithBlankPage, parent=q)

    shortcut('ctrl+w', self.closeCurrentTab, parent=q)
    for k in 'ctrl+l', 'alt+d':
      shortcut(k, self.addressEdit.focus, parent=q)

  ## Properties ##

  @memoizedproperty
  def networkAccessManager(self):
    ret = WbNetworkAccessManager(self.q)
    return ret

  @memoizedproperty
  def tabWidget(self):
    ret = WbTabWidget()
    ret.setTabBar(self.tabBar)
    #ret.setCornerWidget(self.closeTabButton)
    ret.currentChanged.connect(self.loadAddress)
    ret.tabCloseRequested.connect(self.closeTab)
    ret.doubleClicked.connect(self.newTabAtLastWithBlankPage, Qt.QueuedConnection)
    return ret

  @memoizedproperty
  def tabBar(self):
    ret = WbTabBar()
    #ret.doubleClickedAt.connect(self.newTabAfter, Qt.QueuedConnection)
    return ret

  # FIXME: This button is too ugly
  @memoizedproperty
  def closeTabButton(self):
    ret = QtWidgets.QPushButton()
    ret.setText("x")
    ret.setToolTip(tr_("Close"))
    ret.clicked.connect(self.closeCurrentTab)
    return ret

  @memoizedproperty
  def header(self):
    layout = QtWidgets.QHBoxLayout()
    layout.addWidget(self.addressEdit)
    ret = QtWidgets.QWidget()
    ret.setLayout(layout)
    return ret

  @memoizedproperty
  def addressEdit(self):
    ret = WbAddressEdit()
    ret.textEntered.connect(self.openUnknown)
    ret.editTextChanged.connect(self.highlightText)
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

  def openUrl(self, url): # string ->
    """
    @param  url  unicode
    """
    #self.addRecentUrl(url)
    if self.tabWidget.isEmpty():
      self.newTabAfterCurrent()
    v = self.tabWidget.currentWidget()
    v.load(url)

  def openBlankPage(self):
    if self.tabWidget.isEmpty():
      self.newTabAtLast()
    v = self.tabWidget.currentWidget()
    #assert v
    if v:
      v.setUrl("https://google.com")
      #v->setContent(::rc_html_start_(), "text/html");
      #ui->tabWidget->setTabText(tabIndex(), tr("Start Page"));
      #int i = ui->addressEdit->findText(WB_BLANK_PAGE);
      #if (i >= 0)
      #  ui->addressEdit->setCurrentIndex(i);
      #ui->addressEdit->setIcon(WBRC_IMAGE_APP);

  def newTabAfterCurrentWithBlankPage(self):
    self.newTabAfterCurrent()
    self.openBlankPage()

  def newTabAtLastWithBlankPage(self):
    self.newTabAfterCurrent()
    self.openBlankPage()

  def newTabAfterCurrent(self, focus=True):
    self.newTabAfter(index=self.tabWidget.currentIndex(), focus=focus)

  def newTabAtLast(self, focus=True):
    self.newTabAfter(index=self.tabWidget.count() -1, focus=focus)

  def newTabAfter(self, index, focus=True):
    self.tabWidget.newTab(self.createWebView(), index=index+1, focus=focus)

  def createWebView(self):
    ret = WbWebView()
    ret.onCreateWindow = self._createWindow
    ret.page().setNetworkAccessManager(self.networkAccessManager)
    ret.titleChanged.connect(partial(self.setTabTitle, ret))
    ret.urlChanged.connect(self.updateAddress)
    return ret

  def _createWindow(self, type): # QWebPage::WebWindowType -> QWebView
    ret = self.createWebView()
    self.tabWidget.newTab(ret,
        index=self.tabWidget.currentIndex() + 1,
        focus=True)
    return ret

  def updateAddress(self):
    v = self.tabWidget.currentWidget()
    if v:
      url = v.url()
      url = '' if url.isEmpty() else url.toString()
      url = textutil.simplifyurl(url)
      self.addressEdit.setUrl(url)

  def loadAddress(self):
    w = self.tabWidget.currentWidget()
    if w:
      url = w.url()
      url = '' if url.isEmpty() else url.toString()
      url = textutil.simplifyurl(url)
    else:
      url = ''
    self.addressEdit.setEditText(url)

  def closeTab(self, index):
    if self.tabWidget.count() <= 1:
      self.q.quitRequested.emit()
    else:
      if index >= 0 and index < self.tabWidget.count():
        self.tabWidget.removeTab(index)

  def closeCurrentTab(self):
    self.closeTab(self.tabWidget.currentIndex())

#    if (textSizeMultiplier_ > 0)
#      view->setTextSizeMultiplier(textSizeMultiplier_);
#
#    wbview->setSearchEngines(qxSubList(searchEngines_, SearchEngineFactory::VisibleEngineCount));
#    wbview->setSearchEngine(searchEngine_);
#
#    connect(wbview, SIGNAL(message(QString)), SLOT(showMessage(QString)));
#    connect(wbview, SIGNAL(errorMessage(QString)), SLOT(showError(QString)));
#    connect(wbview, SIGNAL(warning(QString)), SLOT(warn(QString)));
#    connect(wbview, SIGNAL(notification(QString)), SLOT(notify(QString)));
#
#    connect(wbview, SIGNAL(selectedTextChanged(QString)), SIGNAL(selectedTextChanged(QString)));
#    connect(wbview, SIGNAL(windowCreated(QWebView*)), SLOT(newTab(QWebView*)));
#    connect(wbview, SIGNAL(openLinkRequested(QString)), SLOT(newTabInBackground(QString)));
#
#    connect(wbview, SIGNAL(openUrlWithAcPlayerRequested(QString)), SIGNAL(openUrlWithAcPlayerRequested(QString)));
#    connect(wbview, SIGNAL(importUrlToAcPlayerRequested(QString)), SIGNAL(importUrlToAcPlayerRequested(QString)));
#    connect(wbview, SIGNAL(openUrlWithAcDownloaderRequested(QString)), SIGNAL(openUrlWithAcDownloaderRequested(QString)));
#    connect(wbview, SIGNAL(downloadAnnotationUrlRequested(QString)), SIGNAL(downloadAnnotationUrlRequested(QString)));
#    connect(wbview, SIGNAL(undoClosedTabRequested()), SLOT(undoCloseTab()));
#    connect(wbview, SIGNAL(newWindowRequested()), SIGNAL(newWindowRequested()));
#    connect(wbview, SIGNAL(fullScreenRequested()), SIGNAL(fullScreenRequested()));
#
#    connect(wbview, SIGNAL(searchWithEngineRequested(QString,int)), SLOT(searchInNewTab(QString,int)));
##ifdef Q_OS_WIN
#    connect(wbview, SIGNAL(menuBarVisibilityChangeRequested(bool)), menuBar(), SLOT(setVisible(bool)));
#    connect(wbview, SIGNAL(toggleMenuBarVisibleRequested()), SLOT(toggleMenuBarVisible()));
##endif // Q_OS_WIN
#    connect(this, SIGNAL(searchEngineChanged(int)), wbview, SLOT(setSearchEngine(int)));
#  }
#
#  connect(view, SIGNAL(loadProgress(int)), SLOT(updateLoadProgress()));
#  connect(view, SIGNAL(statusBarMessage(QString)), SLOT(showMessage(QString)));
#  connect(view, SIGNAL(urlChanged(QUrl)), SLOT(updateAddressbar()));
#  connect(view, SIGNAL(loadStarted()), SLOT(handleLoadStarted()));
#  connect(view, SIGNAL(loadFinished(bool)), SLOT(handleLoadFinished()));
#  connect(view, SIGNAL(linkClicked(QUrl)), SLOT(addRecentUrl(QUrl)));
#
#  view->installEventFilter(mouseGestureFilter_);
#
#  QString t; //= tr("New Tab");
#  if (index < 0 || index >= tabCount())
#    index = ui->tabWidget->addTab(view, t);
#  else
#    ui->tabWidget->insertTab(index, view, t);
#
#  auto textDaemon = new detail::SetTabText(ui->tabWidget, view);
#  connect(view, SIGNAL(titleChanged(QString)), textDaemon, SLOT(trigger(QString)));
#
#  auto iconDaemon = new detail::SetTabIcon(ui->tabWidget, view);
#  connect(view, SIGNAL(loadStarted()), iconDaemon, SLOT(trigger()));
#  connect(view, SIGNAL(loadFinished(bool)), iconDaemon, SLOT(trigger(bool)));
#
#  auto searchDaemon = new detail::SearchTab(this, ui->tabWidget, view);
#  connect(view, SIGNAL(loadFinished(bool)), searchDaemon, SLOT(trigger(bool)));
#
#  connect(view, SIGNAL(titleChanged(QString)), SLOT(updateWindowTitle()));

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
    if len(t) < 15:
      return t
    else:
      return t[:13] + ' ...'

# EOF
