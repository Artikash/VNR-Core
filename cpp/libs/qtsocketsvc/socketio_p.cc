// socketio_p.cc
// 4/29/2014 jichi
#include "qtsocketsvc/socketio_p.h"
#include "qtsocketsvc/socketpack.h"

#define DEBUG "socketio_p"
#include "sakurakit/skdebug.h"

bool SocketService::writeSocket(QIODevice *socket, const QByteArray &data, bool pack)
{
  Q_ASSERT(socket);
  if (pack) {
    QByteArray packet = packData(data);
    return socket->write(packet) == packet.size();
  } else
    return socket->write(data) == data.size();
}

QByteArray SocketService::readSocket(QIODevice *socket, quint32 &dataSize)
{
  Q_ASSERT(socket);
  enum { HeadSize = UInt32Size };
  qint64 bytesAvailable = socket->bytesAvailable();
  if (!dataSize) {
    if (bytesAvailable < HeadSize) {
      DOUT("insufficient head size");
      return QByteArray();
    }
    QByteArray head = socket->read(HeadSize);
    qint32 size = unpackUInt32(head);
    if (!size) {
      DOUT("empty message size");
      return QByteArray();
    }
    dataSize = size;
    bytesAvailable -= HeadSize;
  }

  if (dataSize == 0) {
    DOUT("zero data size");
    return QByteArray();
  }
  if (bytesAvailable < dataSize) {
    DOUT("insufficient message size:" << bytesAvailable << "<" << dataSize);
    return QByteArray();
  }
  DOUT("message size =" << dataSize);

  QByteArray data = socket->read(dataSize);
  dataSize = 0;
  return data;
}

// EOF
