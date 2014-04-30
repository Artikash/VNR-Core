// main.cc
// 4/29/2014 jichi

#include "qtsocketsvc/socketclient.h"
#include "qtsocketsvc/socketpack.h"
#include <QtCore>


int main(int argc, char *argv[])
{
  QByteArray data;

  int x = 100;
  data = SocketService::packUInt32(x);
  x = SocketService::unpackUInt32(data);
  qDebug() << x;

  QStringList l = QStringList() << "hello" << "world";
  data = SocketService::packStringList(l);
  l = SocketService::unpackStringList(data);
  qDebug() << l;

  QCoreApplication a(argc, argv);
  SocketClient cli;
  cli.setPort(6002);
  cli.start();
  cli.waitForReady();
  qDebug() << cli.isActive();
  qDebug() << cli.isReady();
  const char *text = "hello";
  bool ok = cli.sendData(text);
  qDebug() << ok;

  cli.waitForBytesWritten();
  ok = cli.sendData(text);
  qDebug() << ok;

  //cli.dumpSocketInfo();

  QObject::connect(&cli, SIGNAL(disconnected()), qApp, SLOT(quit()));
  return a.exec();
}

// EOF
