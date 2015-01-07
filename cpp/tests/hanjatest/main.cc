// main.cc
// 1/6/2014 jichi
#include "hanjaconv/hanjaconv.h"
#include <QtCore>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>

void iterwords(const std::wstring &text, const std::function<void (size_t start, size_t length)> &fun)
{
  bool be = false; // is end
  size_t pos = 0; // beginning of a word
  for (size_t i =  0; i < text.size(); i++) {
    wchar_t ch = text[i];
    if (::isspace(ch) || ::ispunct(ch)) {
      be = true;
    } else if (be) {
      fun(pos, i - pos);
      pos = i;
      be = false;
    }
  }
  if (pos)
    fun(pos, text.size() - pos);
}

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
  //std::wstring s = L"容易이용이";

  HangulHanjaConverter conv;
  bool ok = conv.addFile(path);
  qDebug() << ok;

  std::wstring t = conv.convert(s);
  qDebug() << QString::fromStdWString(t);

  std::list <std::list<std::pair<std::wstring, std::wstring> > > ret;
  iterwords(s, [&ret, &s, &conv](size_t start, size_t stop) {

    std::wstring text = s.substr(start, stop);
    qDebug() << "----:" << start << ":"<<QString::fromStdWString(text);

    ret.push_back(std::list<std::pair<std::wstring, std::wstring> >());
    auto &l = ret.back();

    conv.collect(text, [&l, &text](size_t start, size_t length, const wchar_t *hanja) {
      l.push_back(std::make_pair(
        text.substr(start, length)
        , hanja ? std::wstring(hanja) : std::wstring()
      ));
    });
  });

  qDebug() << ret.size();
  for (auto it = ret.begin(); it != ret.end(); ++it)
    for (auto p = it->begin(); p != it->end(); ++p)
      qDebug() << QString::fromStdWString(p->first) << ":" << QString::fromStdWString(p->second);

  qDebug() << "leave";
  return 0;
}

// EOF
