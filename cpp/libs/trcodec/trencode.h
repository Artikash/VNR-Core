#ifndef TRENCODE_H
#define TRENCODE_H

// trencode.h
// 9/20/2014 jichi

#include "sakurakit/skglobal.h"
#include "trcodec/trdefine.h"
#include <string>

class TranslationEncoderPrivate;
class TranslationEncoder
{
  SK_CLASS(TranslationEncoder)
  SK_DISABLE_COPY(TranslationEncoder)
  SK_DECLARE_PRIVATE(TranslationEncoderPrivate)

  // - Construction -
public:
  explicit TranslationEncoder();
  ~TranslationEncoder();

  // Initialization

  ///  Return the number of loaded rules
  int size() const;

  ///  Return whether the script has been loaded, thread-safe
  bool isEmpty() const;

  ///  Clear the loaded script
  void clear();

  ///  Add script from file
  bool loadScript(const std::wstring &path);

  // Replacement
  std::wstring encode(const std::wstring &text, int category = 0) const;
};

#endif // TRENCODE_H
