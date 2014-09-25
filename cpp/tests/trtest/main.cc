// main.cc
// 9/20/2014 jichi
#include "trscript/trscript.h"
#include <QtCore>

int main()
{
  qDebug() << "enter";

  wchar_t ws[] = L"kawaii";
  QString text = QString::fromWCharArray(ws);

  QString path = "../cpp/libs/trscript/example.txt";
  //path = "Z:\\Users\\jichi\\opt\\stream/Caches/tmp/reader/dict/zht/ocr.txt";

  TranslationScriptManager m;
  m.loadFile(path);

  if (!m.isEmpty()) {
    qDebug() << text;
    text = m.translate(text);
    qDebug() << text;
  }

  qDebug() << "leave";
  return 0;
}

// EOF
