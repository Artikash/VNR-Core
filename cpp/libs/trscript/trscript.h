#ifndef TRSCRIPT_H
#define TRSCRIPT_H

// trscript.h
// 9/20/2014 jichi

#include "sakurakit/skglobal.h"
#include <string>

// All functions in this class are thread-safe to Qt threads
class TranslationScriptManagerPrivate;
class TranslationScriptManager
{
  SK_CLASS(TranslationScriptManager)
  SK_DISABLE_COPY(TranslationScriptManager)
  SK_DECLARE_PRIVATE(TranslationScriptManagerPrivate)

  // - Construction -
public:
  explicit TranslationScriptManager();
  ~TranslationScriptManager();

  // Initialization

  ///  Return the number of loaded rules
  int size() const;

  ///  Return whether the script has been loaded, thread-safe
  bool isEmpty() const;

  ///  Clear the loaded script
  void clear();

  ///  Clear script from file
  bool loadFile(const std::wstring &path);

  // Replacement

  // Rewrite the text according to the script, thread-safe
  std::wstring translate(const std::wstring &text) const;

  // Render option

  bool isUnderline() const;
  void setUnderline(bool value);
};

#endif // TRSCRIPT_H
