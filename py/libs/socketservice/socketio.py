# coding: utf8
# socketio.py
# jichi 4/28/2014

from PySide.QtCore import QByteArray
from sakurakit.skdebug import dprint, dwarn
import socketmarshal

MESSAGE_HEADER_SIZE = 4 # 4 bytes

def writesocket(data, socket):
  """
  @param  data  str or QBytArray not unicode
  @param  socket  QAbstractSocket
  @return  bool

  Passing unicode will crash Python
  """
  ok = False
  # Explicitly use QByteArray to preserve message size
  #if isinstance(data, unicode):
  #  data = data.encode(encoding, errors=encodingErrors)
  if not isinstance(data, QByteArray):
    data = QByteArray(data)
  size = len(data)
  header = socketmarshal.int32bytes(size)
  data.prepend(header)
  #assert len(data) == size
  ok = len(data) == socket.write(data)
  dprint("pass: ok = %s" % ok)
  return ok

def readsocket(socket):
  """
  @param  QAbstractSocket
  @return  QByteArray or None

  The socket used in this function must have messageSize property initialized to 0
  """
  headerSize = MESSAGE_HEADER_SIZE
  bytesAvailable = socket.bytesAvailable()
  if not socket.messageSize and bytesAvailable < headerSize:
    dprint("insufficient header size")
    return
  if not socket.messageSize:
    ba = socket.read(headerSize)
    size = socketmarshal.bytes2int(ba)
    if not size:
      dwarn("empty message size")
      return
    socket.messageSize = size
    bytesAvailable -= headerSize

  bodySize = socket.messageSize
  if bodySize < 0:
    dwarn("negative data size = %s" % bodySize)
    return
  if bodySize == 0:
    dwarn("zero data size")
    return ''

  if bytesAvailable < bodySize:
    dprint("insufficient message size: %s < %s" % (bytesAvailable, bodySize))
    return

  dprint("message size = %s" % socket.messageSize)

  data = socket.read(bodySize)
  socket.messageSize = 0
  return data

# EOF
