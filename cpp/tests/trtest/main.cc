// main.cc
// 9/20/2014 jichi
#include "trscript/trscript.h"
#include <QtCore>

int main()
{
  qDebug() << "enter";

  wchar_t ws[] = L"ABCABC";
  //QString text = QString::fromWCharArray(ws);
  std::wstring text = ws;

  std::wstring path = L"../cpp/libs/trscript/example.txt";
  //path = "Z:\\Users\\jichi\\opt\\stream/Caches/tmp/reader/dict/zht/ocr.txt";
  path = L"../../../../Caches/tmp/reader/dict/zhs/test.txt";

  TranslationScriptManager m;
  m.setUnderline(true);
  m.loadFile(path);

  if (!m.isEmpty()) {
    qDebug() << QString::fromStdWString(text);
    text = m.translate(text);
    qDebug() << QString::fromStdWString(text);
  }

  qDebug() << "leave";
  return 0;
}

// EOF
