// hangulparse.cc
// 1/6/2015 jichi
#include "pyhangul/hangulparse.h"
#include <QtCore/QChar>

//namespace {
//std::locale UTF8_LOCALE("ja_JP.UTF-8");
//} // unamed

void hangul_iter_parse(const std::wstring &text, const std::function<void (size_t start, size_t length)> &fun)
{
  size_t pos = 0; // beginning of a word
  for (size_t i =  0; i < text.size(); i++) {
    //wchar_t ch = text[i];
    //if (::isspace(ch) || ::ispunct(ch)) // this need std::locale(ja_JP.UTF-8)
    QChar ch = text[i];
    if (ch.isSpace() || ch.isPunct()) {
      if (pos < i)
        fun(pos, i - pos);
      fun(i, 1);
      pos = i + 1;
    }
  }
  if (pos < text.size())
    fun(pos, text.size() - pos);
}

// EOF

/*

void hangul_iter_parse(const std::wstring &text, const std::function<void (size_t start, size_t length)> &fun)
{
  bool be = false; // is end
  size_t pos = 0; // beginning of a word
  for (size_t i =  0; i < text.size(); i++) {
    wchar_t ch = text[i];
    if (::isspace(ch) || ::ispunct(ch)) // this need std::locale(ja_JP.UTF-8)
    else if (be) {
      fun(pos, i - pos);
      pos = i;
      be = false;
    }
  }
  if (pos)
    fun(pos, text.size() - pos);
}

void hangul_iter_parse(const QString &text, const std::function<void (size_t start, size_t length)> &fun)
{
  bool be = false; // is end
  size_t pos = 0; // beginning of a word
  for (size_t i =  0; i < text.size(); i++) {
    const QChar &ch = text[i];
    if (ch.isSpace() || ch.isPunct())
      be = true;
    else if (be) {
      fun(pos, i - pos);
      pos = i;
      be = false;
    }
  }
  if (pos)
    fun(pos, text.size() - pos);
}

*/
