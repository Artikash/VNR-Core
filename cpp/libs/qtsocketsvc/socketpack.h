#ifndef _QTSOCKETSVC_SOCKETPACK_H
#define _QTSOCKETSVC_SOCKETPACK_H

// socketpack.h
// 4/29/2014 jichi
//
// This class must be consistent with socketsvc/socketpack.py
#include "qtsocketsvc/qtsocketsvc.h"
#include <QtCore/QStringList>

QTSS_BEGIN_NAMESPACE

namespace SocketService {

inline QByteArray packInt32(qint32 value)
{
  quint8 bytes[4] = {
     value >> 24,
    (value >> 16) & 0xff,
    (value >> 8)  & 0xff,
     value        & 0xff
  };
  return bytes;
}

inline qint32 unpackInt32(const QByteArray &data, int offset = 0)
{
  return data.size() < 4 ? 0 :
        (data[offset]     << 24)
      | (data[offset + 1] << 16)
      | (data[offset + 2] <<  8)
      |  data[offset + 3];
}

QByteArray packStringList(const QStringList &l);
QStringList unpackStringList(const QByteArray &data);

} // SocketService

QTSS_END_NAMESPACE

#endif // _QTSOCKETSVC_SOCKETPACK_H
