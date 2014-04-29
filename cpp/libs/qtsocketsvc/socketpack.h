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

QByteArray packStringList(const QStringList &l);
QStringList unpackStringList(const QByteArray &data);

} // SocketService

QTSS_END_NAMESPACE

#endif // _QTSOCKETSVC_SOCKETPACK_H
