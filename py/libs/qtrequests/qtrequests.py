# coding: utf8
# qtrequests
# 8/23/2014 jichi
# Python requests implemented using QtNetwork.

__all__ = ['Session']

import json
import urllib
from PySide.QtCore import QEventLoop, QCoreApplication
from PySide.QtNetwork import QNetworkRequest

#class EventLoop(QEventLoop):
#  def __init__(self, parent=None):
#    super(EventLoop, self).__init__(parent)
#  def __del__(self):
#    print "del: pass"

MIMETYPE_JSON = 'application/json'
MIMETYPE_FORM = 'application/x-www-form-urlencoded'

class Response:
  def __init__(self, ok=True, content=''):
    self.ok = ok # bool
    self.content = content # str

class _Session:
  encoding = 'utf8'

  def __init__(self, nam):
    self.nam = nam

  @staticmethod
  def _waitReply(reply, abortSignal=None):
    """
    @param  reply  QNetworkReply
    @param* abortSignal  Signal
    """
    loop = QEventLoop()
    if abortSignal:
      abortSignal.connect(loop.quit)

    qApp = QCoreApplication.instance()
    qApp.aboutToQuit.connect(loop.quit)

    reply.finished.connect(loop.quit)
    loop.exec_()

  @staticmethod
  def _createRequest(url):
    """
    @param  url  unicode
    """
    return QNetworkRequest(url)

  _createGetRequest = _createRequest

  def _createPostRequest(self, *args, **kwargs):
    r = self._createRequest(*args, **kwargs)
    r.setHeader(QNetworkRequest.ContentTypeHeader, MIMETYPE_FORM)
    return r

  def _tostr(self, data):
    """
    @param  data  any
    @return  str not unicode not None
    """
    if isinstance(data, str):
      return data
    if isinstance(data, unicode):
      return data.encode(self.encoding, errors='ignore')
    return "%s" % data # might throw

  def _createPostData(self, data):
    """
    @param  data  None str or unicode or kw
    @return  ''
    """
    if data is None:
      return ''
    if isinstance(data, str):
      return data
    if isinstance(data, unicode):
      return self._tostr(data)
    if isinstance(data, dict):
      data = {self._tostr(k):self._tostr(v) for k,v in data.iteritems()}
      return urllib.urlencode(data) # application/x-www-form-urlencoded
      #return json.dumps(data) # application/json
    return "%s" % data # may throw for unknown data format

  def get(self, url, abortSignal=None):
    """
    @param  url  unicode
    @param* abortSignal  Signal
    @return  str or None
    """
    reply = self.nam.get(self._createGetRequest(url))
    self._waitReply(reply, abortSignal=abortSignal)
    if reply.isRunning():
      reply.abort()
    return reply.readAll().data()

  def post(self, url, data=None, abortSignal=None):
    """
    @param  url  unicode
    @param* abortSignal  Signal
    @return  str or None
    """
    reply = self.nam.post(
        self._createPostRequest(url),
        self._createPostData(data))
    self._waitReply(reply, abortSignal=abortSignal)
    if reply.isRunning():
      reply.abort()
    return reply.readAll().data()

class Session(object):

  def __init__(self, nam):
    self.__d = _Session(nam) # Network access manager

  def networkAccessManager(self):
    """
    @return  QNetworkAccessManager
    """
    return self.__d.nam

  def setNetworkAccessManager(self, v):
    """
    @param  QNetworkAccessManager
    """
    self.__d.nam = v

  def get(self, *args, **kwargs):
    """Similar to requests.get.
    @param  url  unicode
    @param* abortSignal  Signal
    @return  Response not None
    """
    data = self.__d.get(*args, **kwargs)
    ok = data is not None
    content = data or ''
    return Response(ok=ok, content=content)

  def post(self, *args, **kwargs):
    """Similar to requests.post.
    @param  url  unicode
    @param  data  None or str or unicode or kw
    @param* abortSignal  Signal
    @return  Response not None
    """
    data = self.__d.post(*args, **kwargs)
    ok = data is not None
    content = data or ''
    return Response(ok=ok, content=content)

if __name__ == '__main__':
  import sys
  sys.path.append('..')

  def test():
    print "start"
    from PySide.QtNetwork import QNetworkAccessManager
    nam = QNetworkAccessManager()

    #import requests
    #s = requests
    #s = requests.Session()

    s = Session(nam)

    from sakurakit.skprofiler import SkProfiler
    url = "http://www.google.com"
    with SkProfiler():
      r = s.post(url)
      print r.ok
      print type(r.content)

    print "quit"
    app.quit()

  from PySide.QtCore import QTimer
  app = QCoreApplication(sys.argv)
  QTimer.singleShot(0, test)
  app.exec_()

# EOF
