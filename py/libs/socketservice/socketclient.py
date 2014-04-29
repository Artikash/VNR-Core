# coding: utf8
# socketclient.py
# jichi 4/28/2014

if __name__ == '__main__':
  import sys
  sys.path.append('..')

__all__ = ['SocketClient']

from PySide.QtCore import QObject, Signal

class SocketClient(QObject):

  def __init__(self, parent=None):
    super(SocketClient, self).__init__(parent)
    self.__d = _SocketClient(self)

  def address(self): return self.__d.address # -> str
  def setAddress(self, v): self.__d.address = v

  def port(self): return self.__d.port # -> int
  def setPort(self, v): self.__d.port = v

  def isActive(self): return True # -> bool

  def connect(self): return True # -> bool
  def disconnect(self): return True # -> bool

  connected = Signal()
  disconnected = Signal()

  received = Signal(str) # data
  def send(self, data):  # str
    pass

@Q_Q
class _SocketClient(object):
  def __init__(self, q):
    self.address = '127.0.0.1' # host name without http prefix
    self.port = 0 # int
    self.socket = None # # QTcpSocket

if __name__ == '__main__':
  import sys
  from PySide.QtCore import QCoreApplication
  app =  QCoreApplication(sys.argv)
  c = SocketClient()
  s.setPort(6002)

  sys.exit(app.exec_())

# EOF
