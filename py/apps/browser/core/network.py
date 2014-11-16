# coding: utf8
# network.py
# 12/13/2012 jichi

__all__ = 'WbNetworkAccessManager',

import os
from PySide.QtNetwork import QNetworkAccessManager, QNetworkRequest, QNetworkDiskCache
from sakurakit import skfileio, sknetwork
from sakurakit.skdebug import dprint
import proxy, rc

## Cookie ##

class WbNetworkCookieJar(sknetwork.SkNetworkCookieJar):
  def __init__(self, path, parent=None): # unicode
    super(WbNetworkCookieJar, self).__init__(parent)
    self.path = path
    self.load()
    self._injectCookies()

    # Automatically save cookies using timer
    from PySide.QtCore import QCoreApplication
    qApp = QCoreApplication.instance()
    qApp.aboutToQuit.connect(self.save)

  # Persistent storage

  def load(self): # unicode ->
    path = self.path
    if path and os.path.exists(path):
      self.unmarshal(skfileio.readdata(path))

  def save(self): # unicode -> bool
    return bool(self.path) and skfileio.writedata(self.path, self.marshal())

  def _injectCookies(self):
    from PySide.QtCore import QUrl
    from PySide.QtNetwork import QNetworkCookie
    import cookies

    for cookies,urls in cookies.itercookies():
      l = [QNetworkCookie(k,v) for k,v in cookies.iteritems()]
      for url in urls:
        if url.startswith("http://www."):
          domain = url.replace("http://www", '') # such as .dmm.co.jp
          for c in l:
            c.setDomain(domain)
        self.setCookiesFromUrl(l, QUrl(url))

  # Proxy

  def cookiesForUrl(self, url): # override
    url = proxy.fromproxyurl(url) or url
    return super(WbNetworkCookieJar, self).cookiesForUrl(url)

  def setCookiesFromUrl(self, cookies, url): # override
    url = proxy.fromproxyurl(url) or url
    return super(WbNetworkCookieJar, self).setCookiesFromUrl(cookies, url)

## Network ##

class WbNetworkAccessManager(QNetworkAccessManager):
  def __init__(self, parent=None):
    super(WbNetworkAccessManager, self).__init__(parent)
    self.sslErrors.connect(_WbNetworkAccessManager.onSslErrors)

    # Enable offline cache
    cache = QNetworkDiskCache(self)
    cache.setCacheDirectory(rc.DIR_CACHE_NETMAN) # QNetworkDiskCache will create this directory if it does not exists.
    self.setCache(cache)

    # Load cookies
    jar = WbNetworkCookieJar(rc.COOKIES_LOCATION)
    self.setCookieJar(jar)

  # QNetworkReply *createRequest(Operation op, const QNetworkRequest &req, QIODevice *outgoingData = nullptr) override;
  def createRequest(self, op, req, outgoingData=None): # override
    url = req.url()
    newurl = self._getBlockedUrl(url)
    if newurl:
      req = QNetworkRequest(newurl)
    else:
      newurl = proxy.toproxyurl(url)
      if newurl:
        req = QNetworkRequest(req) # since request tis constent
        req.setUrl(newurl)
        reply = super(WbNetworkAccessManager, self).createRequest(op, req, outgoingData)
        #if url.host().lower().endswith('dmm.co.jp'):
        reply.setUrl(url) # restore the old url
        return reply
    return super(WbNetworkAccessManager, self).createRequest(op, req, outgoingData)

  @staticmethod
  def _getBlockedUrl(url):
    """
    @param  url  QUrl
    @return  unicode or QUrl or None
    """
    if url.path() == '/js/localize_welcome.js': # for DMM
      dprint("block dmm localize welcome")
      return rc.DMM_LOCALIZED_WELCOME_URL

class _WbNetworkAccessManager:

  # http://stackoverflow.com/questions/8362506/qwebview-qt-webkit-wont-open-some-ssl-pages-redirects-not-allowed
  @staticmethod
  def onSslErrors(reply, errors): # QNetworkReply, [QSslError] ->
    reply.ignoreSslErrors()
    #dprint("ignore ssl error")

# EOF
