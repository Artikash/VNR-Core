// msghandler.cc
// 4/27/2014 jichi

#include "config.h"
#include "util/location.h"
#include "util/msghandler.h"
#include <QtCore/QDir>
#include <QtCore/QDateTime>
#include <QtCore/QFile>
#include <QtCore/QTextStream>

#define DEBUG_FILE      VNRAGENT_DEBUG_FILE
#define DEBUG_TIMESTAMP QDateTime::currentDateTime().toString("MM:dd: hh:mm:ss")

QString Util::debugFileLocation()
{
  static QString ret;
  if (ret.isEmpty()) {
    ret = vnrLocation();
    if (ret.isEmpty())
      ret = desktopLocation();
    if (!ret.isEmpty())
      ret.append(QDir::separator());
    ret.append(DEBUG_FILE);
  }
  return ret;
}

// See: http://www.cppblog.com/lauer3912/archive/2011/04/10/143870.html
void Util::debugMsgHandler(QtMsgType type, const char *msg)
{

  QString output;
  switch (type) {
  case QtDebugMsg:    output = QString("%1: %2\n").arg(DEBUG_TIMESTAMP, msg); break;
  case QtWarningMsg:  output = QString("%1: warning: %2\n").arg(DEBUG_TIMESTAMP, msg); break;
  case QtCriticalMsg: output = QString("%1: critical: %2\n").arg(DEBUG_TIMESTAMP, msg); break;
  case QtFatalMsg:    output = QString("%1: fatal: %2\n").arg(DEBUG_TIMESTAMP, msg); break;
  default: return;
  }

  QFile file(debugFileLocation());
  if (file.open(QIODevice::Text|QIODevice::WriteOnly|QIODevice::Append))
    QTextStream(&file) << output;
}

void Util::installDebugMsgHandler()
{
  static bool installed = false; // only install once
  if (!installed) {
    installed = true;
    QFile file(debugFileLocation());
    if (file.open(QIODevice::Text|QIODevice::WriteOnly)) {
      QTextStream(&file) << "This log file is created by VNR hook for each game. For bug report, please also attach this file.\n";
      qInstallMsgHandler(debugMsgHandler);
    }
  }
}

void Util::uninstallDebugMsgHandler()
{ qInstallMsgHandler(nullptr); }

// EOF
