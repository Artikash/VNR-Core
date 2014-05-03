# coding: utf8
# rpcman.py
# 2/1/2013 jichi

__all__ = ['RpcServer', 'RpcClient']

if __name__ == '__main__':
  import sys
  sys.path.append('..')
  import debug
  debug.initenv()

RPC_WAIT_TIME = 3000 # wait time after sending data

from socketsvc import socketcli, socketpack, socketsrv
import config

# Client

class _RpcClient:
  def __init__(self, parent):
    self.client = socketcli.SocketClient(parent)
    self.client.setPort(config.QT_METACALL_PORT)

  def invoke(self, *args): # [str] -> bool
    if not self.client.isActive():
      return False
    data = socketpack.packstrlist(args)
    return self.client.sendData(data, waitTime=0) # no wait time

class RpcClient:
  def __init__(self, parent=None):
    self.__d = _RpcClient(parent)

  def isActive(self): return self.__d.client.isActive() # -> bool
  def start(self): self.__d.client.start()
  def stop(self): self.__d.client.stop()

  def waitForConnected(self, interval=RPC_WAIT_TIME): # -> bool
    return self.__d.client.waitForConnected(interval)

  # Actions
  # -> bool
  def activate(self): return self.__d.invoke('app.activate')

# Server

def _unmarshalInteger(s): # str -> int, use hex
  #try: return int(s, 16) #if s.startswith('0x') else int(s)
  try: return int(s)
  except ValueError:
    dwarn("failed to marshal number %s" % s)
    return 0

def _marshalInteger(v): # int -> str, use hex
  return str(v)
  #return hex(i).replace('0x', '').replace('L', '') # remove prefix '+-0x' and trailing 'L'

def _unmarshalBool(s): # str -> bool
  return s == '1'

def _marshalBool(v): # int -> str, use hex
  return '1' if v else '0'

from ctypes import c_longlong
from functools import partial
import json
from PySide.QtCore import Signal, Qt, QObject
from sakurakit.skclass import Q_Q, memoized
from sakurakit.skdebug import dwarn
from mytr import my
import growl

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

  # Agent

  agentConnected = Signal(long) # pid
  agentDisconnected = Signal(long) # pid
  windowTextsReceived = Signal(dict) # {long hash:unicode text}
  engineTextReceived = Signal(unicode, c_longlong, int) # text, hash, role

  def isAgentConnected(self): return bool(self.__d.agentSocket)
  def closeAgent(self): self.__d.closeAgentSocket()

  def enableAgent(self): self.__d.callAgent('ui.enable')
  def disableAgent(self): self.__d.callAgent('ui.disable')

  def detachAgent(self): self.__d.callAgent('detach')

  def agentProcessId(self): return self.__d.agentPid

  def clearWindowTranslation(self): self.__d.callAgent('ui.clear')

  def sendWindowTranslation(self, data):
    """
    @param  data  {hash:translation}
    """
    try:
      data = json.dumps(data) #, ensure_ascii=False) # the json parser in vnragent don't enforce ascii
      self.__d.callAgent('ui.text', data)
    except TypeError, e:
      dwarn("failed to encode json: %s" % e)

@Q_Q
class _RpcServer(object):
  def __init__(self, q):
    self.server = socketsrv.SocketServer(q)
    self.server.setPort(config.QT_METACALL_PORT)
    self.server.dataReceived.connect(self._onDataReceived)

    self.server.disconnected.connect(self._onDisconnected)

    self.agentSocket = None # QAbstractSocket
    self.agentPid = 0 # long

  # Send

  def callAgent(self, *args):
    if self.agentSocket:
      data = socketpack.packstrlist(args)
      self.server.sendData(data, self.agentSocket, waitTime=RPC_WAIT_TIME)

  # Receive

  def _onDisconnected(self, socket):
    if socket is self.agentSocket:
      self.agentSocket = None
      self.q.agentDisconnected.emit(self.agentPid)
      self.agentPid  = 0

  def _onDataReceived(self, data, socket):
    args = socketpack.unpackstrlist(data)
    if not args:
      dwarn("unpack data failed")
      return
    self._onCall(socket, *args)

  def _onCall(self, socket, cmd, *params): # on serverMessageReceived
    """
    @param  socket  QTcpSocket
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
        pid = _unmarshalInteger(params[0])
        if pid:
          self._onAgentPing(socket, pid)
    elif cmd == 'agent.ui.text':
      if params:
        self._onWindowTexts(params[0])
    elif cmd == 'agent.engine.text':
      if len(params) == 4:
        self._onEngineText(*params)
      else:
        dwarn("invalid parameter count:", params)

    else:
      dwarn("unknown command: %s" % cmd)

  def closeAgentSocket(self):
    self.agentPid = 0
    if self.agentSocket:
      self.server.closeSocket(self.agentSocket)
      self.agentSocket = None
      self.q.agentDisconnected.emit()

  def _onAgentPing(self, socket, pid):
    """
    @param  socket  QTcpSocket
    @param  pid  long
    """
    growl.msg(my.tr("Window text translator is loaded"))
    if self.agentSocket:
      self.server.closeSocket(self.agentSocket)
    self.agentPid = pid
    self.agentSocket = socket
    self.q.agentConnected.emit(pid) # SIGNAL TO BE CHANGED

    #reply = {
    #  'debug': config.APP_DEBUG,
    #  'userLanguage': 'en', # TODO
    #  'gameLanguage': 'en', # TODO
    #}
    #self.callAgent('config', json.dumps(reply))
    self.callAgent('config.debug', self._marshalBool(config.APP_DEBUG))

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

  def _onEngineText(self, text, hash, role, trans):
    """
    @param  text  unicode
    @param  hash  qint64
    @param  role  int
    @param  trans  bool   need translation
    """
    try:
      hash = _unmarshalInteger(hash)
      role = _unmarshalInteger(role)
      trans = _unmarshalBool(trans)
      self.q.engineTextReceived.emit(text, hash, role)
      if trans:
        print role, len(text)
        text = u'简体中文' + text
        #text = u'简体中文'
        self.callAgent('engine.text',
            text, _marshalInteger(hash), _marshalInteger(role))
    except ValueError:
      dwarn("failed to convert text hash or role to integer")

if __name__ == '__main__':
  a = debug.app()

  cli = RpcClient()
  cli.setPort(6003)
  r.start()
  if r.waitForConnected():
    r.activate()
    a.processEvents()

  a.exec_()

# EOF
