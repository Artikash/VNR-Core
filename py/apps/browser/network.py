# coding: utf8
# network.py
# 12/13/2012 jichi

__all__ = ['WbNetworkAccessManager']

import re, os
from PySide.QtCore import QUrl
from PySide.QtNetwork import QNetworkAccessManager, QNetworkRequest, QNetworkDiskCache
from sakurakit import skfileio, sknetwork
import config, rc

## Proxy ##

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

_PROXY_IPS = {
  ip:host
  for host,ip in config.PROXY_DOMAINS.iteritems()
} # {string ip: string host}

def toproxyurl(url): # QUrl -> QUrl or None
 if url.scheme() == 'http':
   url = QUrl(url)
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
   return url

_re_proxy_key = re.compile(r'/proxy/([^/]+)(.*)')
def fromproxyurl(url): # QUrl -> QUrl or None
  if url.scheme() == 'http':
    host = url.host()
    if host == config.PROXY_HOST:
      path = url.path()
      m = _re_proxy_key.match(path)
      if m:
        key = m.group(1)
        if key:
          host = config.PROXY_SITES.get(key)
          if host:
            url = QUrl(url)
            url.setHost(host)
            path = m.group(2) or '/'
            if path[0] != '/':
              path = '/' + path
            url.setPath(path)
            return url
    else:
      host = _PROXY_IPS.get(host)
      if host:
        url = QUrl(url)
        url.setHost(host)
        return url

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
    url = fromproxyurl(url) or url
    return super(WbNetworkCookieJar, self).cookiesForUrl(url)

  def setCookiesFromUrl(self, cookies, url): # override
    url = fromproxyurl(url) or url
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
    newurl = toproxyurl(url)
    if newurl:
      req = QNetworkRequest(req) # since request tis constent
      req.setUrl(newurl)
      reply = super(WbNetworkAccessManager, self).createRequest(op, req, outgoingData)
      reply.setUrl(url) # restore the old url
      return reply
    return super(WbNetworkAccessManager, self).createRequest(op, req, outgoingData)

# EOF
