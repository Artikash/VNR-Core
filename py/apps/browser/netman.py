# coding: utf8
# webbrowser.py
# 12/13/2012 jichi

__all__ = ['WbNetworkAccessManager']

from PySide.QtNetwork import QNetworkAccessManager, QNetworkRequest
import config

def _normalize_host(url): # str -> str
  url = url.lower()
  if not url.startswith('www.'):
    url = 'www.' + url
  return url

_PROXY_HOST = config.PROXY_HOST
_PROXY_SITES = {
  _normalize_host(host):key
  for key,host in config.PROXY_SITES.iteritems()
} # {string host: string key}

class WbNetworkAccessManager(QNetworkAccessManager):
  def __init__(self, parent=None):
    super(WbNetworkAccessManager, self).__init__(parent)

  # QNetworkReply *createRequest(Operation op, const QNetworkRequest &req, QIODevice *outgoingData = nullptr) override;
  def createRequest(self, op, req, outgoingData=None): # override
    url = req.url() # QUrl
    if url.scheme() == 'http':
      host = _normalize_host(url.host())
      key = _PROXY_SITES.get(host)
      if key:
        url.setHost(_PROXY_HOST)
        path = '/proxy/' + key + url.path()
        url.setPath(path)
      req = QNetworkRequest(req) # since request tis constent
      req.setUrl(url)
    return super(WbNetworkAccessManager, self).createRequest(op, req, outgoingData)

# EOF
