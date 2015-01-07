// main.cc
// 1/6/2014 jichi
#include "hanjaconv/hanjaconv.h"
#include <QtCore>

int main()
{
  qDebug() << "enter";

  const wchar_t *path = L"/Users/jichi/opt/stream/Library/Dictionaries/hanja/dic6.txt";
  //std::wstring s = L"공주";
  //std::wstring s = L"상냥한 곳은 바뀌지 않았어. 옛날은 좀 더 형분적인 표현(이었)였지만.";
  //std::wstring s = L"지금 무엇을 하고 있을까. 자신의 판단으로 대답해도 좋은 것인가 헤매었을 것이다.";

  // Sentence from http://ko.wikipedia.org/wiki/자유_콘텐츠
  // Ambiguity: 이용이
  // Supposed to be 利用이 instead of 이容易
  std::wstring s = L"자유 콘텐츠는 저작권이 소멸된 퍼블릭 도메인은 물론, 저작권이 있지만 위 기준에 따라 자유롭게 이용이 허락된 콘텐츠도 포함한다.";
  //std::wstring s = L"이용이";

  HangulHanjaConverter conv;
  bool ok = conv.addFile(path);
  qDebug() << ok;

  std::wstring t = conv.convert(s);
  qDebug() << QString::fromStdWString(t);


  qDebug() << "leave";
  return 0;
}

// EOF
