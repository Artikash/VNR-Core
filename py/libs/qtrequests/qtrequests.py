# coding: utf8
# qtrequests
# 8/23/2014 jichi
# Python requests implemented using QtNetwork.

__all__ = ['Session']

import urllib, zlib
from PySide.QtCore import QUrl, QEventLoop, QCoreApplication
from PySide.QtNetwork import QNetworkRequest

#class EventLoop(QEventLoop):
#  def __init__(self, parent=None):
#    super(EventLoop, self).__init__(parent)
#  def __del__(self):
#    print "del: pass"

#MIMETYPE_JSON = 'application/json'
MIMETYPE_FORM = 'application/x-www-form-urlencoded'

# Is this thread-safe?
_zlib_decomp = zlib.decompressobj(16 + zlib.MAX_WBITS) # The magic number is only for http
def _gunzip(data):
  """
  @param  data  str
  @return  data  str

  GZIP, see: http://rationalpie.wordpress.com/2010/06/02/python-streaming-gzip-decompression/
  """
  return _zlib_decomp.decompress(data)

class Response:
  def __init__(self, ok=True, content=''):
    self.ok = ok # bool
    self.content = content # str

class _Session:
  encoding = 'utf8'

  def __init__(self, nam, abortSignal):
    self.nam = nam # QNetworkAccessManager
    self.abortSignal = abortSignal # Signal

  def _waitReply(self, reply):
    """
    @param  reply  QNetworkReply
    @param* abortSignal  Signal
    """
    loop = QEventLoop()
    if self.abortSignal:
      self.abortSignal.connect(loop.quit)

    qApp = QCoreApplication.instance()
    qApp.aboutToQuit.connect(loop.quit)

    reply.finished.connect(loop.quit)
    loop.exec_()

  def _createRequest(self, url, params=None, headers=None):
    """
    @param  url  unicode
    @param* headers  {unicode key:unicode value}
    @param* params  {unicode key:unicode value}
    """
    if params:
      url = self._createUrl(url, params)
    r = QNetworkRequest(url)
    if headers:
      for k,v in headers.iteritems():
        r.setRawHeader(self._tostr(k), self._tostr(v))
    return r

  _createGetRequest = _createRequest

  def _createPostRequest(self, *args, **kwargs):
    r = self._createRequest(*args, **kwargs)
    headers = kwargs.get('headers')
    if not headers or 'Content-Type' not in headers:
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

  def _tounicode(self, data):
    """
    @param  data  any
    @return  str not unicode not None
    """
    if isinstance(data, str):
      return data
      #return data.decode(self.encoding, errors='ignore')
    if isinstance(data, unicode):
      return data
    if isinstance(data, bool):
      return 'true' if data else 'false' # use lower case
    return "%s" % data # might throw

  def _createUrl(self, url, params=None):
    """
    @param  url  unicode
    @param* params  {unicode key:unicode value}
    @return  QUrl
    """
    url = QUrl(url)
    if params:
      for k,v in params.iteritems():
        url.addQueryItem(self._tounicode(k), self._tounicode(v))
    return url

  def _encodePostData(self, data):
    """
    @param  data  {unicode key:unicode value}
    @return  str
    """
    data = {self._tostr(k):self._tostr(v) for k,v in data.iteritems()}
    return urllib.urlencode(data) # application/x-www-form-urlencoded

  def _createPostData(self, data):
    """
    @param  data  None str or unicode or kw
    @return  str
    """
    if data is None:
      return ''
    if isinstance(data, str):
      return data
    if isinstance(data, unicode):
      return self._tostr(data)
    if isinstance(data, dict):
      return self._encodePostData(data) # application/x-www-form-urlencoded
      #return json.dumps(data) # application/json
    return "%s" % data # may throw for unknown data format

  def _readReply(self, reply):
    """
    @param  reply  QNetworkReply
    @return  str

    GZIP, see: http://rationalpie.wordpress.com/2010/06/02/python-streaming-gzip-decompression/
    """
    ret = reply.readAll().data()
    enc = reply.rawHeader('Content-Encoding').data() # QByteArray to str
    if enc and 'gzip' in enc:
      try: ret = _gunzip(ret)
      except: pass # ignore gunzip error
    return ret

  def get(self, *args, **kwargs):
    """
    @param  url  unicode
    @return  str or None
    """
    reply = self.nam.get(self._createGetRequest(*args, **kwargs))
    self._waitReply(reply)
    if reply.isRunning():
      reply.abort() # return None
    else:
      return self._readReply(reply)

  def post(self, url, data=None, **kwargs):
    """
    @param  url  unicode
    @param* data  any
    @return  str or None
    """
    reply = self.nam.post(
        self._createPostRequest(url, **kwargs),
        self._createPostData(data))
    self._waitReply(reply)
    if reply.isRunning():
      reply.abort()
    else:
      return self._readReply(reply)

class Session(object):

  def __init__(self, nam, abortSignal=None):
    """
    @param  nam  QNetworkAccessManager
    @param* abortSignal  Signal
    """
    self.__d = _Session(nam, abortSignal) # Network access manager

  # Properties

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

  def abortSignal(self):
    """
    @return  Signal
    """
    return self.__d.abortSignal

  def setAbortSignal(self, v):
    """
    @param  Signal
    """
    self.__d.abortSignal = v

  # Queries

  def get(self, *args, **kwargs):
    """Similar to requests.get.
    @param  url  unicode
    @param* headers  {unicode key:unicode value}
    @param* params  {unicode key:unicode value}
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
    @param* headers  {unicode key:unicode value}
    @param* params  {unicode key:unicode value}
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
