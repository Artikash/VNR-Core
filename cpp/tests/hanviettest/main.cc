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
  std::wstring s = L"你在说什么？A";

  HanVietWordDictionary wd;
  wd.addFile(worddic);
  qDebug() << wd.size();
  t = wd.translate(s);
  qDebug() << QString::fromStdWString(t);

  HanVietPhraseDictionary pd;
  pd.addFile(phrasedic);
  qDebug() << pd.size();
  t = pd.translate(s);
  qDebug() << QString::fromStdWString(t);

  HanVietTranslator ht;
  ht.addPhraseFile(phrasedic);
  ht.addWordFile(worddic);

  t = ht.translate(s);
  qDebug() << QString::fromStdWString(t);

  qDebug() << "leave";
  return 0;
}

// EOF
