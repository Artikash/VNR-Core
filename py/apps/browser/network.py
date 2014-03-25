# coding: utf8
# network.py
# 12/13/2012 jichi

__all__ = ['WbNetworkAccessManager']

import os
from PySide.QtNetwork import QNetworkAccessManager, QNetworkRequest, QNetworkDiskCache
from sakurakit import skfileio, sknetwork
import proxy, rc

## Cookie ##

class WbNetworkCookieJar(sknetwork.SkNetworkCookieJar):
  def __init__(self, path, parent=None): # unicode
    super(WbNetworkCookieJar, self).__init__(parent)
    self.path = path
    self.load()

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

    # Enable offline cache
    cache = QNetworkDiskCache(self)
    cache.setCacheDirectory(rc.DIR_CACHE_NETMAN) # QNetworkDiskCache will create this directory if it does not exists.
    self.setCache(cache)

    # Load cookies
    jar = WbNetworkCookieJar(rc.COOKIE_LOCATION)
    self.setCookieJar(jar)

  # QNetworkReply *createRequest(Operation op, const QNetworkRequest &req, QIODevice *outgoingData = nullptr) override;
  def createRequest(self, op, req, outgoingData=None): # override
    url = req.url()
    newurl = proxy.toproxyurl(url)
    if newurl:
      req = QNetworkRequest(req) # since request tis constent
      req.setUrl(newurl)
      reply = super(WbNetworkAccessManager, self).createRequest(op, req, outgoingData)
      reply.setUrl(url) # restore the old url
      return reply
    return super(WbNetworkAccessManager, self).createRequest(op, req, outgoingData)

# EOF
