// socketio_p.cc
// 4/29/2014 jichi
#include "qtsocketsvc/socketio_p.h"
#include "qtsocketsvc/socketpack.h"
#include <QtNetwork/QAbstractSocket>

#define DEBUG "socketio_p"
#include "sakurakit/skdebug.h"

QByteArray SocketService::readSocket(QAbstractSocket *socket, quint32 &dataSize)
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

bool SocketService::writeSocket(QAbstractSocket *socket, const QByteArray &data)
{
  Q_ASSERT(socket);
  int size = data.size();
  QByteArray packet = packUInt32(size) + data;
  return socket->write(packet) == packet.size();
}

// EOF
