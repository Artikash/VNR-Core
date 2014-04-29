// socketpack.cc
// 4/29/2014 jichi
#include "qtsocketsvc/socketpack.h"

QTSS_BEGIN_NAMESPACE

QByteArray SocketService::packStringList(const QStringList &l);
QStringList SocketService::unpackStringList(const QByteArray &data);

QTSS_END_NAMESPACE

#endif // _QTSOCKETSVC_SOCKETPACK_H
