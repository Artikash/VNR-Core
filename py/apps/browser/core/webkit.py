# coding: utf8
# webkit.py
# 12/13/2012 jichi

__all__ = ['WbWebView', 'WbWebPage']

import re
from PySide.QtCore import Qt, Signal, QEvent, QUrl
from PySide.QtWebKit import QWebPage
from Qt5 import QtWidgets
from sakurakit import skwebkit
from sakurakit.skclass import Q_Q
from sakurakit.skdebug import dprint
from sakurakit.sktr import tr_
import beans, rc

## WbWebView ##

class WbWebView(skwebkit.SkWebView):
  messageReceived = Signal(unicode)

  def __init__(self, parent=None):
    super(WbWebView, self).__init__(parent, page=WbWebPage())
    self.__d = _WbWebView(self)
    self.enableHighlight()

    self.titleChanged.connect(self.setWindowTitle)
    self.onCreateWindow = None # -> QWebView

  def __del__(self):
    dprint("pass") # For debug usage

  # QWebView * QWebView::createWindow ( QWebPage::WebWindowType type ) [virtual protected]
  def createWindow(self, type): # override
    if self.onCreateWindow:
      return self.onCreateWindow(type)

  def _onLoadStarted(self):
    self.setCursor(Qt.BusyCursor)
  def _onLoadFinished(self, success): # bool ->
    self.setCursor(Qt.ArrowCursor)

  def zoomIn(self): # override
    super(WbWebView, self).zoomIn()
    self.__d.showZoomMessage()

  def zoomOut(self): # override
    super(WbWebView, self).zoomOut()
    self.__d.showZoomMessage()

  def zoomReset(self): # override
    super(WbWebView, self).zoomReset()
    self.__d.showZoomMessage()

  # Injection

  def inject(self):
    if not self.url().isEmpty():
      self.page().inject()

  def isInjectEnabled(self): return self.page().injectEnabled
  def setInjectEnabled(self, t):
    page = self.page()
    if page.isInjectEnabled() != t:
      page.setInjectEnabled(t)
      self.inject()

  def load(self, url): # QUrl ->
    t = url.toString() if isinstance(url, QUrl) else url
    if t.startswith('about:'):
      data = rc.html_data(t)
      if data:
        self.setHtml(data)
        return
    super(WbWebView, self).load(url)
@Q_Q
class _WbWebView(object):

  def __init__(self, q):
    page = q.page()
    page.loadStarted.connect(self._onLoadStarted)
    page.loadFinished.connect(self._onLoadFinished)

  def _showMessage(self, t): # unicode ->
    self.q.messageReceived.emit(t)

  def _showZoomMessage(self):
    z = self.q.zoomFactor()
    t = "%s %i%%" % (tr_("Zoom"), int(z * 100))
    self._showMessage(t)

  def _onLoadStarted(self):
    self.q.setCursor(Qt.BusyCursor)
  def _onLoadFinished(self, success): # bool ->
    self.q.setCursor(Qt.ArrowCursor)

## WbWebPage ##

class WbWebPage(skwebkit.SkWebPage):
  def __init__(self, parent=None):
    super(WbWebPage, self).__init__(parent)
    self.__d = _WbWebPage(self)

    # 3/22/2014: FIXME
    # If I use DelegateNoLinks, linkClicked will not emit
    # Otherwise when disabled, createWindow will not be called
    #self.setLinkDelegationPolicy(QWebPage.DelegateAllLinks) # handle all links
    self.setLinkDelegationPolicy(QWebPage.DelegateAllLinks)

    self.linkClicked.connect(self.openUrl)

    self.linkHovered.connect(self.setHoveredLink)

  linkClickedWithModifiers = Signal(unicode)

  def hoveredLink(self): return self.__d.hoveredLink
  def setHoveredLink(self, v): self.__d.hoveredLink = v

  def progress(self): return self.__d.progress # -> int [0,100]
  def isLoading(self): return self.__d.progress < 100
  def isFinished(self): return self.__d.progress == 100

  def inject(self): self.__d.injectJavaScript() # Force inject

  def isInjectEnabled(self): return self.__d.injectEnabled
  def setInjectEnabled(self, t): self.__d.injectEnabled = t
    #d = self.__d
    #if d.injectEnabled != t:
    #  d.injectEnabled = t
    #  d.injectJavaScript()

  def event(self, ev): # override
    if (ev.type() == QEvent.MouseButtonRelease and
        ev.button() == Qt.LeftButton and ev.modifiers() == Qt.ControlModifier and
        self.__d.hoveredLink):
      self.linkClickedWithModifiers.emit(self.__d.hoveredLink)
      ev.accept()
      return True
    return super(WbWebPage, self).event(ev)

  def openUrl(self, url): # QUrl
    self.mainFrame().load(url)

  ## Extensions

  # bool supportsExtension(Extension extension) const
  def supportsExtension(self, extension): # override
    if extension == QWebPage.ErrorPageExtension:
      return True
    return super(WbWebPage, self).supportsExtension(extension)

  #bool extension(Extension extension, const ExtensionOption *option, ExtensionReturn *output)
  def extension(self, extension, option, output): # override
    if extension == QWebPage.ErrorPageExtension and self.errorPageExtension(option, output):
      return True
    return super(WbWebPage, self).extension(extension, option, output)

  #bool errorPageExtension(const ErrorPageExtensionOption *option, ErrorPageExtensionReturn *output) # override
  def errorPageExtension(self, option, output): # override
    if not option or not output:
      return False
    dprint("enter: error = %s, message = %s" % (option.error, option.errorString))
    output.encoding = "UTF-8" # force UTF-8
    #output.baseUrl = option.url # local url
    #output.contentType = "text/html" # already automaticly detected
    output.content = rc.jinja_template('error').render({
      'code': self.extensionErrorCode(option.error),
      'message': option.errorString,
      'url': option.url.toString(),
      'tr': tr_,
      'rc': rc,
    }).encode('utf8', 'ignore')
    return True

  @staticmethod
  def extensionErrorCode(error): # int -> int
    if error == 3:
      return 404
    else:
      return error

  # See: WebKit/qt/Api/qwebpage.cpp
  # Example: Mozilla/5.0 (Windows NT 6.1) AppleWebKit/534.34 (KHTML, like Gecko) MYAPP/MYVERSION Safari/534.34
  # QString userAgentForUrl(const QUrl &url) const
  #def userAgentForUrl(self, url): # override
  #  # Get rid of app name from user agent
  #  ret = super(WbWebPage, self).userAgentForUrl(url)
  #  return re.sub(r" \\S+ Safari/", " Safari/", ret)

@Q_Q
class _WbWebPage(object):

  def __init__(self, q):
    self.hoveredLink = ''

    self.progress = 100 # int [0,100]

    self.injectEnabled = False # bool
    self._beansInjected = False # bool

    q.loadProgress.connect(self._onLoadProgress)
    q.loadStarted.connect(self._onLoadStarted)
    q.loadFinished.connect(self._onLoadFinished)

    f = q.mainFrame()
    f.javaScriptWindowObjectCleared.connect(self._onJavaScriptCleared)

  ## Progress

  def _onLoadProgress(self, value):
    self.progress = value # int ->
  def _onLoadStarted(self):
    self.progress = 0
  def _onLoadFinished(self, success): # bool ->
    self.progress = 100
    if success and self.injectEnabled:
      self.injectJavaScript()

  ## JavaScript

  def _onJavaScriptCleared(self):
    self._beansInjected = False
    if self.injectEnabled:
      self.injectBeans()

  def injectJavaScript(self):
    #if not self.q.parent().url().isEmpty():
    self.injectBeans()
    f = self.q.mainFrame()
    f.evaluateJavaScript(rc.cdn_data('inject'))

  def injectBeans(self):
    if not self._beansInjected: # and not self.q.parent().url().isEmpty():
      self._beansInjected = True

      f = self.q.mainFrame()
      #f.addToJavaScriptWindowObject('bean', self._webBean)
      for name,obj in self._iterbeans():
        f.addToJavaScriptWindowObject(name, obj)

  @staticmethod
  def _iterbeans():
    """
    return  [(unicode name, QObject bean)]
    """
    import beans
    m = beans.manager()
    return (
      ('cdnBean', m.cdnBean),
      ('clipBean', m.clipBean),
      ('settingsBean', m.settingsBean),
      ('jlpBean', m.jlpBean),
      ('ttsBean', m.ttsBean),
    )

# EOF
