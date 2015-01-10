// main.cc
// 9/20/2014 jichi
#include "trscript/trscript.h"
#include <QtCore>

int main()
{
  qDebug() << "enter";

  wchar_t ws[] = L"ルナ様";
  //QString text = QString::fromWCharArray(ws);
  std::wstring text = ws;

  std::wstring path;
  //path = L"../cpp/libs/trscript/example.txt";
  path = L"/Users/jichi/stream/Caches/tmp/reader/dict/zhs/escape_input.txt";
  //path = L"../../../../Caches/tmp/reader/dict/zhs/test.txt";

  TranslationScriptManager m;
  m.setLinkEnabled(true);
  m.loadFile(path);
  qDebug() << m.size();

  if (!m.isEmpty()) {
    qDebug() << QString::fromStdWString(text);
    text = m.translate(text);
    qDebug() << QString::fromStdWString(text);
  }

  qDebug() << "leave";
  return 0;
}

// EOF
