# coding: utf8
# socketserver.py
# jichi 4/28/2014

if __name__ == '__main__':
  import sys
  sys.path.append('..')

__all__ = ['SocketServer']

import weakref
from functools import partial
from PySide.QtCore import QObject, Signal
from sakurakit.skclass import Q_Q
from sakurakit.skdebug import dprint

# http://stackoverflow.com/questions/444591/convert-a-string-of-bytes-into-an-int-python
def _bytes2int(ba): # QByteArray => int
  return sum(ord(c) << (i * 8) for i, c in enumerate(ba[::-1]))

class SocketServer(QObject):
  """
  Message protocol:
  The first 4b is int32 message size.
  """

  def __init__(self, parent=None):
    super(SocketServer, self).__init__(parent)
    self.__d = _SocketServer(self)

  connected = Signal(QObject) # client socket
  disconnected = Signal(QObject) # client socket
  socketError = Signal(QObject) # client socket

  dataReceived = Signal(str, QObject) # data, client socket

  def sendData(self, data, client):  # str, client socket
    pass

  def broadcastData(self, data):
    pass

  def address(self): return self.__d.address # -> str
  def setAddress(self, v): self.__d.address = v

  def port(self): return self.__d.port # -> int
  def setPort(self, v): self.__d.port = v

  def start(self): pass
  def stop(self): pass

  def isActive(self): # -> bool
    return bool(self.__d.server) and self.__d.server.isListening()

  def removeSocket(self, socket): # QTcpSocket
    if socket.isOpen():
      socket.close()
    self.__d.deleteSocket(socket)

MESSAGE_HEADER_SIZE = 4 # 4 bytes

@Q_Q
class _SocketServer(object):
  def __init__(self, q):
    self.address = '127.0.0.1' # host name without http prefix
    self.port = 0 # int
    self.server = None # QTcpServer
    self.sockets = [] # [QTcpSocket]

  def createServer(self):
    from PySide.QtNetwork import QTcpServer
    ret = QTcpServer(self.q)
    ret.newConnection.connect(self._onNewConnection)
    return ret

  def start(self): # -> bool
    from PySide.QtNetwork import QHostAddress
    if not self.server:
      self.server = self.createServer()
    return self.server.listen(QHostAddress(self.address), self.port)

  def _onNewConnection(self):
    socket = self._server.nextPendingConnection();
    if socket:
      socket.messageSize = 0 # int
      self.sockets.append(socket)
      ref = weakref.ref(socket)
      socket.error.connect(partial(lambda ref, error:
          self.q.socketError.emit(ref()),
          ref))
      socket.connected.connect(partial(lambda ref:
          self.q.connected.emit(ref()),
          ref))
      socket.disconnected.connect(partial(lambda ref:
          self.q.disconnected.emit(ref()),
          ref))

      socket.disconnected.connect(partial(lambda ref:
          self.deleteSocket(ref()),
          ref))

      socket.readyRead.connect(partial(lambda ref:
          self.readSocket(ref()),
          ref))

  def deleteSocket(self, socket):
    socket.deleteLater()
    try: self.sockets.remove(socket)
    except ValueError: pass

  def readSocket(self, socket):
    bytesAvailable = socket.bytesAvailable()
    if not socket.messageSize and bytesAvailable < MESSAGE_HEADER_SIZE:
      dprint("insufficient header size")
      return
    if not socket.messageSize:
      ba = socket.read(MESSAGE_HEADER_SIZE)
      size = _bytes2int(ba)
      if not size:
        dwarn("empty message size")
        return
      socket.messageSize = size

    if bytesAvailable < socket.messageSize - message:
      dprint("insufficient message size")
      return

    data = socket.readAll()
    socket.messageSize = 0

    self.q.dataReceived.emit(data, socket)

if __name__ == '__main__':
  import sys
  from PySide.QtCore import QCoreApplication
  app =  QCoreApplication(sys.argv)
  s = SocketServer()
  s.setPort(6002)
  s.start()

  sys.exit(app.exec_())

# EOF
