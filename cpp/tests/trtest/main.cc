// main.cc
// 9/20/2014 jichi
#include "trscript/trscript.h"
#include <QtCore>

int main()
{
  qDebug() << "enter";

  //wchar_t ws[] = L"kawaii";
  wchar_t ws[] = L"aBbA";
  QString text = QString::fromWCharArray(ws);

  QString path = "../cpp/libs/trscript/example.txt";
  //path = "Z:\\Users\\jichi\\opt\\stream/Caches/tmp/reader/dict/zht/ocr.txt";
  path = "Z:\\Users\\jichi\\opt\\stream\\Caches\\tmp\\reader\\dict\\ja\\origin.txt";

  TranslationScriptManager m;
  m.setUnderline(true);
  m.loadFile(path);

  if (!m.isEmpty()) {
    qDebug() << text;
    text = m.translate(text);
    qDebug() << text;
  }

  QRegExp rx("(a.*?b)(.*)", Qt::CaseSensitive, QRegExp::RegExp2);
  qDebug() << text.replace(rx, "\\2\\1");

  qDebug() << "leave";
  return 0;
}

// EOF
