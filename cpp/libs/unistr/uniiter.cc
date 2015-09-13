// uniiter.cc
// 9/12/2015 jichi
#include "unistr/uniiter.h"
#include <string>

void uniiter::iter_words(const wchar_t *text, size_t size,
                       const std::function<void (size_t start, size_t length)> &fun)
{
  size_t pos = 0; // beginning of a word
  for (size_t i = 0; i < size; i++) {
    wchar_t ch = text[i];
    if (::iswspace(ch) || ::iswpunct(ch)) {
      if (pos < i)
        fun(pos, i - pos);
      fun(i, 1);
      pos = i + 1;
    }
  }
  if (pos < size)
    fun(pos, size - pos);
}

// EOF
