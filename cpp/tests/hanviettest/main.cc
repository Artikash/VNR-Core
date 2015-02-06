// main.cc
// 2/2/2015 jichi
#include "hanviet/hanviet.h"
#include "hanviet/phrasedic.h"
#include "hanviet/worddic.h"
#include <QtCore>

int main()
{
  qDebug() << "enter";

  std::wstring dicdir = L"/Users/jichi/opt/stream/Library/Dictionaries/hanviet/",
               worddic = dicdir + L"ChinesePhienAmWords.txt",
               phrasedic = dicdir + L"VietPhrase.txt";

  std::wstring t;
  std::wstring s = L"我说你在说什么？顶。我说";

  HanVietTranslator ht;
  ht.addPhraseFile(phrasedic);
  ht.addWordFile(worddic);

  enum : bool { mark = false };

  t = ht.translate(s, mark);
  qDebug() << QString::fromStdWString(t);

  t = ht.phraseDicionary()->analyze(s, mark, [](const std::wstring &s, const std::wstring &t) {
  });
  qDebug() << QString::fromStdWString(t);

  qDebug() << "leave";
  return 0;
}

// EOF
