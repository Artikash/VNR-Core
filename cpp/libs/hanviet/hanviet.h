#ifndef HANVIET_H
#define HANVIET_H

// hanviet.h
// 2/2/2015 jichi

#include "sakurakit/skglobal.h"
#include <string>

class HanVietWordDictionary;
class HanVietPhraseDictionary;
class HanVietTranslatorPrivate;
class HanVietTranslator
{
  SK_CLASS(HanVietTranslator)
  SK_DISABLE_COPY(HanVietTranslator)
  SK_DECLARE_PRIVATE(HanVietTranslatorPrivate)

  // - Construction -
public:
  explicit HanVietTranslator();
  ~HanVietTranslator();

  HanVietWordDictionary *wordDicionary() const;
  HanVietPhraseDictionary *phraseDicionary() const;

  // Initialization

  ///  Clear the loaded script
  void clear();

  bool addWordFile(const std::wstring &path);
  bool addPhraseFile(const std::wstring &path);

  // Query

  std::wstring lookupWord(int ch) const;
  std::wstring lookupPhrase(const std::wstring &text) const;

  // Replacement

  std::wstring translate(const std::wstring &text, bool mark = false) const;
};

#endif // HANVIET_H
