// main.cc
// 4/29/2014 jichi

#include "qtsocketsvc/socketclient.h"
#include "qtsocketsvc/socketpack.h"
#include <QtCore>


int main(int argc, char *argv[])
{
  QCoreApplication a(argc, argv);
  QByteArray data;

  int x = 100;
  data = SocketService::packInt32(x);
  x = SocketService::unpackInt32(data);
  qDebug() << x;

  QStringList l = QStringList() << "hello" << "world";
  data = SocketService::packStringList(l);
  l = SocketService::unpackStringList(data);
  qDebug() << l;

  //return a.exec();
  return 0;
}

// EOF
