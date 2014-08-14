// main.cc
// 8/14/2014 jichi
#include "tahscript/tahscript.h"
#include <QtCore/QDebug>

int main()
{
  qDebug() << "enter";

  QString path = "../cpp/libs/tahscript/example.txt";

  TahScriptManager m;
  m.loadFile(path);

  qDebug() << "leave";
  return 0;
}

// EOF
