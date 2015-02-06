#ifndef PHRASEDIC_H
#define PHRASEDIC_H

// phrasedic.h
// 1/7/2015 jichi

#include "sakurakit/skglobal.h"
#include <string>
#include <functional>

class HanVietPhraseDictionaryPrivate;
class HanVietPhraseDictionary
{
  SK_CLASS(HanVietPhraseDictionary)
  SK_DISABLE_COPY(HanVietPhraseDictionary)
  SK_DECLARE_PRIVATE(HanVietPhraseDictionaryPrivate)

  // - Construction -
public:
  explicit HanVietPhraseDictionary();
  ~HanVietPhraseDictionary();

  // Initialization

  ///  Return the number of loaded rules
  int size() const;

  ///  Return whether the script has been loaded, thread-safe
  bool isEmpty() const;

  ///  Clear the loaded script
  void clear();

  ///  Add script from file, reverse to determine the direction
  bool addFile(const std::wstring &path);

  // Replacement

  // Return the text for the character
  std::wstring lookup(const std::wstring &text) const;

  // Replace the characters according to the script
  std::wstring translate(const std::wstring &text, bool mark = false) const;

  // Translate with alignment
  typedef std::function<void (const std::wstring &, const std::wstring &)> align_fun_t;
  std::wstring analyze(const std::wstring &text, bool mark, align_fun_t align) const;
};

#endif // PHRASEDIC_H
