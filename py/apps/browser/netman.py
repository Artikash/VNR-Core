# coding: utf8
# webbrowser.py
# 12/13/2012 jichi

__all__ = ['WbNetworkAccessManager']

import os
from PySide.QtNetwork import QNetworkAccessManager, QNetworkRequest, QNetworkDiskCache
from sakurakit import skfileio, sknetwork
import config, rc

def _normalize_host(url): # str -> str
  url = url.lower()
  if not url.startswith('www.'):
    url = 'www.' + url
  return url

_PROXY_SITES = {
  _normalize_host(host):key
  for key,host in config.PROXY_SITES.iteritems()
} # {string host: string key}

_PROXY_DOMAINS = {
  _normalize_host(host):ip
  for host,ip in config.PROXY_DOMAINS.iteritems()
} # {string host: string ip}

class WbNetworkCookieJar(sknetwork.SkNetworkCookieJar):
  def __init__(self, path, parent=None): # unicode
    super(WbNetworkCookieJar, self).__init__(parent)
    self.path = path
    self.load()

    # Automatically save cookies using timer
    from PySide.QtCore import QCoreApplication
    qApp = QCoreApplication.instance()
    qApp.aboutToQuit.connect(self.save)

  def load(self): # unicode ->
    path = self.path
    if path and os.path.exists(path):
      self.unmarshal(skfileio.readdata(path))

  def save(self): # unicode -> bool
    return bool(self.path) and skfileio.writedata(self.path, self.marshal())

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
    url = req.url() # QUrl
    if url.scheme() == 'http':
      host = _normalize_host(url.host())
      ip = _PROXY_DOMAINS.get(host)
      if ip:
        url.setHost(ip)
      else:
        key = _PROXY_SITES.get(host)
        if key:
          url.setHost(config.PROXY_HOST)
          path = '/proxy/' + key + url.path()
          url.setPath(path)
      req = QNetworkRequest(req) # since request tis constent
      req.setUrl(url)
    return super(WbNetworkAccessManager, self).createRequest(op, req, outgoingData)

# EOF
