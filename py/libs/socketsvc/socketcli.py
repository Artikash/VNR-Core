# coding: utf8
# socketcli.py
# jichi 4/28/2014

if __name__ == '__main__':
  import sys
  sys.path.append('..')

__all__ = ['SocketClient']

from PySide.QtCore import QObject, Signal
#from PySide.QtNetwork import QAbstractSocket
from sakurakit.skclass import Q_Q
from sakurakit.skdebug import dprint
import socketio

class SocketClient(QObject):

  def __init__(self, parent=None):
    super(SocketClient, self).__init__(parent)
    self.__d = _SocketClient(self)

  connected = Signal()
  disconnected = Signal()
  socketError = Signal()

  dataReceived = Signal(bytearray) # data

  def sendData(self, data):  # str -> bool
    return self.__d.writeSocket(data)

  def address(self): return self.__d.address # -> str
  def setAddress(self, v): self.__d.address = v

  def port(self): return self.__d.port # -> int
  def setPort(self, v): self.__d.port = v

  def isActive(self): # -> bool, is connected
    s = self.__d.socket
    return bool(s) and s.state() == s.ConnectedState

  def isReady(self): # -> bool, is connected or disconnected instead of connecting or uninitialized
    s = self.__d.socket
    return bool(s) and s.state() in (s.ConnectedState, s.UnconnectedState)

  def start(self): return self.__d.start() # -> bool, always return true
  def stop(self): self.__d.stop()

  def waitForReady(self):
    s = self.__d.socket
    if s and s.state() not in (s.ConnectedState, s.UnconnectedState):
      from PySide.QtCore import QEventLoop
      loop = QEventLoop()
      s.stateChanged.connect(loop.quit)
      s.error.connect(loop.quit)
      loop.exec_();
      while s.state() in (s.HostLookupState, s.ConnectingState):
        loop.exec_()

@Q_Q
class _SocketClient(object):
  def __init__(self, q):
    self.encoding = 'utf8'
    self.address = '127.0.0.1' # host name without http prefix
    self.port = 0 # int
    self.socket = None # # QTcpSocket

  def _createSocket(self):
    from PySide.QtNetwork import QTcpSocket
    q = self.q
    ret = QTcpSocket(q)
    socketio.initsocket(ret)
    ret.error.connect(q.socketError)
    ret.connected.connect(q.connected)
    ret.disconnected.connect(q.disconnected)
    ret.readyRead.connect(self.readSocket)
    return ret

  def start(self):
    from PySide.QtNetwork import QHostAddress
    if not self.socket:
      self.socket = self._createSocket()
    self.socket.connectToHost(QHostAddress(self.address), self.port)
    dprint("pass")
    return True

  def stop(self):
    if self.socket and self.socket.isOpen():
      self.socket.close()
      dprint("pass")

  def readSocket(self):
    if self.socket:
      data = socketio.readsocket(self.socket)
      if data != None:
        self.q.dataReceived.emit(data)

  def writeSocket(self, data):
    if self.socket:
      if isinstance(data, unicode):
        data = data.encode(self.encoding, errors='ignore')
      socketio.writesocket(data, self.socket)

if __name__ == '__main__':
  t = "hello"
  t = u"こんにちは"
  import sys
  from PySide.QtCore import QCoreApplication
  app =  QCoreApplication(sys.argv)
  c = SocketClient()
  c.setPort(6002)
  def f(data):
    print data, type(data), len(data)
  c.dataReceived.connect(f)
  c.start()
  c.waitForReady()
  c.sendData(t)
  c.disconnected.connect(app.quit)


  sys.exit(app.exec_())

# EOF
