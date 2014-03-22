# coding: utf8
# webbrowser.py
# 12/13/2012 jichi

__all__ = ['WbWebView', 'WbWebPage']

import re
from PySide.QtCore import Qt, Signal
from PySide.QtWebKit import QWebPage
from Qt5 import QtWidgets
from sakurakit import skwebkit
from sakurakit.skdebug import dprint
from sakurakit.sktr import tr_
import rc

## WbWebView ##

class WbWebView(skwebkit.SkWebView):
  messageReceived = Signal(unicode)

  def __init__(self, parent=None):
    super(WbWebView, self).__init__(parent, page=WbWebPage())
    self.enableHighlight()

    self.titleChanged.connect(self.setWindowTitle)
    self.onCreateWindow = None # -> QWebView

    page = self.page()
    page.loadStarted.connect(self._onLoadStarted)
    page.loadFinished.connect(self._onLoadFinished)

  # QWebView * QWebView::createWindow ( QWebPage::WebWindowType type ) [virtual protected]
  def createWindow(self, type): # override
    if self.onCreateWindow:
      return self.onCreateWindow(type)


  def _showMessage(self, t): # unicode ->
    self.messageReceived.emit(t)

  def _onLoadStarted(self):
    self.setCursor(Qt.BusyCursor)
  def _onLoadFinished(self, success): # bool ->
    self.setCursor(Qt.ArrowCursor)

  def zoomIn(self): # override
    super(WbWebView, self).zoomIn()
    self._showZoomMessage()

  def zoomOut(self): # override
    super(WbWebView, self).zoomOut()
    self._showZoomMessage()

  def zoomReset(self): # override
    super(WbWebView, self).zoomReset()
    self._showZoomMessage()

  def _showZoomMessage(self):
    z = self.zoomFactor()
    t = "%s %i%%" % (tr_("Zoom"), int(z * 100))
    self._showMessage(t)

## WbWebPage ##

class WbWebPage(skwebkit.SkWebPage):
  def __init__(self, parent=None):
    super(WbWebPage, self).__init__(parent)
    self.setLinkDelegationPolicy(QWebPage.DelegateAllLinks) # handle all links
    self.linkClicked.connect(self.openUrl)

    self._progress = 100 # int [0,100]

    self.loadProgress.connect(self._onLoadProgress)
    self.loadStarted.connect(self._onLoadStarted)
    self.loadFinished.connect(self._onLoadFinished)

  def progress(self): return self._progress # -> int [0,100]
  def isLoading(self): return self._progress < 100
  def isFinished(self): return self._progress == 100

  def _onLoadProgress(self, value): self._progress = value # int ->
  def _onLoadStarted(self): self._progress = 0
  def _onLoadFinished(self, success): self._progress = 100

  def openUrl(self, url): # QUrl
    self.mainFrame().load(url)

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
      'tr': tr_,
      'code': self.extensionErrorCode(option.error),
      'message': option.errorString,
      'url': option.url.toString(),
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

# EOF
