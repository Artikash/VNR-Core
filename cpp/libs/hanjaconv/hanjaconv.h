#ifndef HANJACONV_H
#define HANJACONV_H

// hanjaconv.h
// 1/6/2015 jichi

#include "sakurakit/skglobal.h"
#include <functional>
#include <string>

class HanjaConverterPrivate;
class HanjaConverter
{
  SK_CLASS(HanjaConverter)
  SK_DISABLE_COPY(HanjaConverter)
  SK_DECLARE_PRIVATE(HanjaConverterPrivate)

  // - Construction -
public:
  typedef std::function<void (size_t start, size_t len, const wchar_t *hanja)> collect_fun_t;

  HanjaConverter();
  ~HanjaConverter();

  // Initialization

  ///  Return the number of loaded rules
  int size() const;

  ///  Return whether the script has been loaded, thread-safe
  bool isEmpty() const;

  ///  Clear the loaded script
  void clear();

  ///  Add script from file, reverse to determine the direction
  bool loadFile(const std::wstring &path);

  // Replacement

  // Replace the characters according to the script, thread-safe
  std::wstring convert(const std::wstring &text) const;

  // Collect list of replaced words.
  void collect(const std::wstring &text, const collect_fun_t &fun) const;
};

#endif // HANJACONV_H
