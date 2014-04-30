# coding: utf8
# rpcman.py
# 2/1/2013 jichi

__all__ = ['RpcServer', 'RpcClient']

if __name__ == '__main__':
  import sys
  sys.path.append('..')
  import debug
  debug.initenv()

from socketsvc import socketcli, socketpack, socketsrv

# Client

class _RpcClient:
  def __init__(self, parent):
    self.client = socketcli.SocketClient(parent)
    self.client.setPort(config.QT_METACALL_PORT)

  def invoke(self, *args): # [str] -> bool
    if not self.client.isActive():
      return False
    data = socketpack.packstrlist(args)
    return self.client.sendData(data)

class RpcClient:
  def __init__(self, parent=None):
    self.__d = _RpcClient(parent)

  def isActive(self): return self.__d.client.isActive() # -> bool
  def start(self): return self.__d.client.start() # -> bool
  def stop(self): self.__d.client.stop()
  def waitForReady(self): self.__d.client.waitForReady()

  # Actions
  # -> bool
  def activate(self): return self.__d.invoke('app.activate')

# Server

from ctypes import c_longlong
from functools import partial
import json
from PySide.QtCore import Signal, Qt, QObject
from sakurakit.skclass import Q_Q, memoized
from sakurakit.skdebug import dwarn
from mytr import my
import config, growl

@memoized
def manager(): return RpcServer()

class RpcServer(QObject):

  def __init__(self, parent=None):
    super(RpcServer, self).__init__(parent)
    self.__d =_RpcServer(self)

  activated = Signal()

  def stop(self):
    self.__d.server.stop()
  def start(self):
    """@return  bool"""
    return self.__d.server.start()
  def isActive(self):
    """@return  bool"""
    return self.__d.server.isActive()

  activated = Signal()
  connected = Signal()
  disconnected = Signal() # TODO: Use this signal with isActive to check if game process is running
  windowTextsReceived = Signal(dict) # {long hash:unicode text}
  engineTextReceived = Signal(unicode, c_longlong, int) # text, hash, role

  def sendTranslation(self, data):
    """
    @param  data  {hash:translation}
    """
    try:
      data = json.dumps(data) #, ensure_ascii=False) # the json parser in vnragent don't enforce ascii
      self.__d.callAgent('ui.text', data)
    except TypeError, e:
      dwarn("failed to encode json: %s" % e)

  def clearTranslation(self):
    self.__d.callAgent('ui.clear')

  def enableClient(self):
    self.__d.callAgent('ui.enable')

  def disableClient(self):
    self.__d.callAgent('ui.disable')

@Q_Q
class _RpcServer(object):
  def __init__(self, q):
    self.server = socketsrv.SocketServer(q)
    self.server.setPort(config.QT_METACALL_PORT)
    self.server.dataReceived.connect(self._onDataReceived)

  # Send

  def callAgent(self, *args):
    data = socketpack.packstrlist(args)
    # TODO: identify the agent socket
    # Don't forget to check if it is active before senddata
    self.server.broadcastData(data)

  # Receive

  @staticmethod
  def _unmarshalNumber(s): # str -> int, use hex
    try: return int(s)
    except ValueError:
      dwarn("failed to marshal number %s" % s)
      return 0

  @staticmethod
  def _marshalNumber(i): # int -> str, use hex
    return hex(i)

  def _onDataReceived(self, data, socket):
    args = socketpack.unpackstrlist(data)
    if not args:
      dwarn("unpack data failed")
      return
    self._onCall(*args)

  def _onCall(self, cmd, *params): # on serverMessageReceived
    """
    @param  cmd  str
    @param  params  [unicode]
    """
    if cmd == 'app.activate':
      self.q.activated.emit()

    elif cmd == 'growl.msg':
      if params:
        growl.msg(params[0])
    elif cmd == 'growl.warn':
      if params:
        growl.warn(params[0])
    elif cmd == 'growl.error':
      if params:
        growl.error(params[0])
    elif cmd == 'growl.notify':
      if params:
        growl.notify(params[0])

    elif cmd == 'agent.ping':
      if params:
        pid = self._unmarshalNumber(params[0])
        if pid:
          growl.msg(my.tr("Window text translator is loaded"))
          self.q.connected.emit()
    elif cmd == 'agent.ui.text':
      if params:
        self._onWindowTexts(params[0])
    elif cmd == 'agent.engine.text':
      if len(params) == 3:
        self._onEngineText(*params)
      else:
        dwarn("invalid parameter count:", params)

    else:
      dwarn("unknown command: %s" % cmd)

  def _onWindowTexts(self, data):
    """
    @param  data  json
    """
    try:
      d = json.loads(data)
      if d and type(d) == dict:
        d = {long(k):v for k,v in d.iteritems()}
        self.q.windowTextsReceived.emit(d)
      else:
        dwarn("error: json is not a map: %s" % data)
    except (ValueError, TypeError, AttributeError), e:
      dwarn(e)
      #dwarn("error: malformed json: %s" % data)

  def _onEngineText(self, text, hash, role):
    """
    @param  text  unicode
    @param  hash  qint64
    @param  role  int
    """
    try:
      hash = self._unmarshalNumber(hash)
      role = self._unmarshalNumber(role)
      self.q.engineTextReceived.emit(text, hash, role)
      text = u"なにこれ"
      self.callAgent('engine.text',
          text, self._marshalNumber(hash), self._marshalNumber(role))
    except ValueError:
      dwarn("failed to convert text hash or role to integer")

if __name__ == '__main__':
  a = debug.app()

  cli = RpcClient()
  cli.setPort(6003)
  if r.start():
    r.waitForReady()
    r.activate()
    a.processEvents()

  a.exec_()

# EOF
