// msghandler.cc
// 4/27/2014 jichi

#include "util/msghandler.h"
#include "config.h"
#include <QtCore/QDateTime>
#include <QtCore/QFile>
#include <QtCore/QTextStream>

#define DEBUG_FILE      VNRAGENT_DEBUG_FILE
#define DEBUG_TIMESTAMP QDateTime::currentDateTime().toString("MM:dd: hh:mm:ss")

// See: http://www.cppblog.com/lauer3912/archive/2011/04/10/143870.html
void Util::debugMsgHandler(QtMsgType type, const char *msg)
{
  QString output;
  switch (type) {
  case QtDebugMsg:    output = QString("%1: %2\n").arg(DEBUG_TIMESTAMP).arg(msg); break;
  case QtWarningMsg:  output = QString("%1: warning: %2\n").arg(DEBUG_TIMESTAMP).arg(msg); break;
  case QtCriticalMsg: output = QString("%1: critical: %2\n").arg(DEBUG_TIMESTAMP).arg(msg); break;
  case QtFatalMsg:    output = QString("%1: fatal: %2\n").arg(DEBUG_TIMESTAMP).arg(msg); break;
  default: return;
  }

  QFile file(DEBUG_FILE);
  if (file.open(QIODevice::Text|QIODevice::WriteOnly|QIODevice::Append))
    QTextStream(&file) << output;
}

void Util::installDebugMsgHandler()
{
  QFile file(DEBUG_FILE);
  if (file.open(QIODevice::Text|QIODevice::WriteOnly)) {
    QTextStream(&file) << "\n--------\n\n";
    qInstallMsgHandler(debugMsgHandler);
  }
}

// EOF
