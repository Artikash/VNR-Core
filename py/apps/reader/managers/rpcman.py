# coding: utf8
# rpcman.py
# 2/1/2013 jichi

__all__ = ['RpcServer', 'RpcClient']

#from ctypes import c_longlong
from functools import partial
import json
from PySide.QtCore import Signal, Qt, QObject
from sakurakit.skclass import Q_Q, memoized
from sakurakit.skdebug import dwarn
import pyreader
from mytr import my
import config, growl

# Server

class RpcSignals(pyreader.ReaderMetaCallPropagator):

  activationRequested = Signal()
  activationReceived = Signal()

  def __init__(self, parent=None):
    super(RpcSignals, self).__init__(parent)
    self.activationRequested.connect(self.activationReceived, Qt.QueuedConnection)

class RpcClient(QObject):

  LOCALHOST = '127.0.0.1'

  def __init__(self, parent=None):
    super(RpcClient, self).__init__(parent)
    self.__d = d = RpcSignals(self)

  def activate(self): self.__d.activationRequested.emit()

  def stop(self): self.__d.stop()
  def isActive(self): return self.__d.isActive()
  def start(self): return self.__d.startClient(self.LOCALHOST, config.QT_METACALL_PORT)
  def waitForReady(self): self.__d.waitForReady()

@Q_Q
class _RpcServer(object):
  def __init__(self, q):
    self.r = r = RpcSignals(q)
    r.activationReceived.connect(q.activated)
    r.serverMessageReceived.connect(self._onMessage)

    self.s = s = pyreader.MetaCallSocketObserver(r)
    r.setSocketObserver(s)
    s.disconnected.connect(q.disconnected)

    #r.q_pingClient.connect(r.pingClient, Qt.QueuedConnection)
    #r.q_callClient.connect(r.callClient, Qt.QueuedConnection)
    #r.q_updateClientData.connect(r.updateClientData, Qt.QueuedConnection)

    #r.growlServerMessage.connect(growl.msg)
    #r.growlServerWarning.connect(growl.warn)
    #r.growlServerError.connect(growl.error)

    #r.pingServer.connect(partial(growl.msg,
    #  my.tr("Window text translator is loaded")
    #))
    #r.pingServer.connect(q.connected)

    #r.updateServerData.connect(self._onDataReceived)

  def _onMessage(self, cmd, param): # on serverMessageReceived
    """
    @param  cmd  str
    @param  param  unicode
    """
    if cmd == 'growl.msg':
      if param:
        growl.msg(param)
    elif cmd == 'growl.warn':
      if param:
        growl.warn(param)
    elif cmd == 'growl.error':
      if param:
        growl.error(param)
    elif cmd == 'growl.notify':
      if param:
        growl.notify(param)

    elif cmd == 'agent.ping':
      growl.msg(my.tr("Window text translator is loaded"))
      self.q.connected.emit()
    elif cmd == 'agent.ui.text':
      self._onWindowTexts(param)
    elif cmd == 'agent.engine.text':
      self._onEngineText(param)

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

  def _onEngineText(self, data):
    """
    @param  data  json
    """
    print 1111111111111
    try:
      d = json.loads(data)
      if d and type(d) == list:
        self.q.engineTextsReceived.emit(d)

# CHECKPOINT
        reply = []
        for item in d:
          reply.append({
            'hash': item['hash'],
            'role': item['role'],
            'text': "//"+item['text'],
            #'text': u'何これ、神马玩意',
          })
          print item['text']
        reply = json.dumps(reply) #, ensure_ascii=False) # the json parser in vnragent don't enforce ascii
        self.callAgent('engine.text', reply)
        print 2222222
      else:
        dwarn("error: json is not a map: %s" % data)
    except (ValueError, TypeError, AttributeError), e:
      dwarn(e)
      #dwarn("error: malformed json: %s" % data)

  def callAgent(self, cmd, param=''):
    self.r.agentMessageRequested.emit(cmd, param)

class RpcServer(QObject):

  def __init__(self, parent=None):
    super(RpcServer, self).__init__(parent)
    self.__d =_RpcServer(self)

  activated = Signal()

  def stop(self):
    self.__d.r.stop()
  def start(self):
    """@return  bool"""
    return self.__d.r.startServer('127.0.0.1', config.QT_METACALL_PORT)
  def isActive(self):
    """@return  bool"""
    return self.__d.r.isActive()

  connected = Signal()
  disconnected = Signal() # TODO: Use this signal with isActive to check if game process is running
  windowTextsReceived = Signal(dict) # {long hash:unicode text}
  engineTextsReceived = Signal(list) # [{role:int,hash:long,text:unicode}]

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

@memoized
def manager(): return RpcServer()

# EOF
