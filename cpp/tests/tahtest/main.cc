// main.cc
// 8/14/2014 jichi
#include "tahscript/tahscript.h"
#include <QtCore/QDebug>

int main()
{
  qDebug() << "enter";

  //wchar_t ws[] = L"裕輔なら、問題なく対処してくれるだろう。";
  wchar_t ws[] = L"「いなさそうですっ」";
  QString text = QString::fromWCharArray(ws);

  QString path = "../../../../Caches/Dictionaries/TAHScripts/ja-ja.wiki";

  TahScriptManager m;
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
