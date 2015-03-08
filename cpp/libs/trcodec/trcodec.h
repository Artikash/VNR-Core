#ifndef TRCODE_H
#define TRCODE_H

// trcode.h
// 9/20/2014 jichi

#include "sakurakit/skglobal.h"
#include <string>

class TranslationCodecPrivate;
class TranslationCodec
{
  SK_CLASS(TranslationCodec)
  SK_DISABLE_COPY(TranslationCodec)
  SK_DECLARE_PRIVATE(TranslationCodecPrivate)

  // - Construction -
public:
  explicit TranslationCodec();
  ~TranslationCodec();

  // Initialization

  ///  Return the number of loaded rules
  int size() const;

  ///  Return whether the script has been loaded, thread-safe
  bool isEmpty() const;

  ///  Clear the loaded script
  void clear();

  ///  Add script from file
  bool loadScript(const std::wstring &path);

  // Transformation

  /// Encode translation with selected category and limit maximum number of iterations
  std::wstring encode(const std::wstring &text, int selector = 0, int limit = 100) const;

  /// Decode translation with selected category and limit maximum number of iterations, and mark changes
  std::wstring decode(const std::wstring &text, int selector = 0, bool mark = true, int limit = 100) const;
};

#endif // TRCODE_H
