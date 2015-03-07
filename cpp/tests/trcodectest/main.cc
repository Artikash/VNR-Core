// main.cc
// 9/20/2014 jichi
#include "trcodec/trencode.h"
#include <QtCore>

int main()
{
  qDebug() << "enter";

  //wchar_t ws[] = L"ルナ様";
  wchar_t ws[] = L"「ごめんなさい。こう言う時どんな顔すればいいのか分からないの。」【綾波レイ】";
  std::wstring text = ws;

  std::wstring path;
  //path = L"../cpp/libs/trscript/example.txt";
  //path = L"/Users/jichi/stream/Caches/tmp/reader/dict/ja-zhs/trans_input.txt";
  path = L"/Users/jichi/opt/stream/Library/Frameworks/Sakura/cpp/libs/trcodec/example.txt";
  //path = L"/Users/jichi/stream/Caches/tmp/reader/dict/ja/game.txt";
  //path = L"../../../../Caches/tmp/reader/dict/zhs/test.txt";
  //path = L"/Users/jichi/tmp/escape_input.txt";

  TranslationEncoder m;
  m.loadScript(path);
  qDebug() << m.size();

  //if (!m.isEmpty()) {
  //  qDebug() << QString::fromStdWString(text);
  //  text = m.translate(text, 2);
  //  qDebug() << QString::fromStdWString(text);
  //}

  qDebug() << "leave";
  return 0;
}

// EOF
