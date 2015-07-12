#ifndef HANGUL2HANJA_H
#define HANGUL2HANJA_H

// hangul2hanja.h
// 1/6/2015 jichi

#include "sakurakit/skglobal.h"
#include <functional>
#include <string>

class HangulHanjaConverterPrivate;
class HangulHanjaConverter
{
  SK_CLASS(HangulHanjaConverter)
  SK_DISABLE_COPY(HangulHanjaConverter)
  SK_DECLARE_PRIVATE(HangulHanjaConverterPrivate)

  // - Construction -
public:
  typedef std::function<void (size_t start, size_t len, const wchar_t *hanja)> collect_fun_t;

  HangulHanjaConverter();
  ~HangulHanjaConverter();

  // Initialization

  ///  Return the number of loaded rules
  int size() const;

  ///  Return whether the script has been loaded, thread-safe
  bool isEmpty() const;

  ///  Clear the loaded script
  void clear();

  ///  Add script from file, reverse to determine the direction
  bool loadFile(const wchar_t *path);

  // Replacement

  // Replace the characters according to the script, thread-safe
  std::wstring convert(const std::wstring &text) const;

  // Collect list of replaced words.
  void collect(const std::wstring &text, const collect_fun_t &fun) const;
};

#endif // HANGUL2HANJA_H
