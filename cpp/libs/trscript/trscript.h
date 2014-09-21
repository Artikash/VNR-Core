#ifndef TRSCRIPT_H
#define TRSCRIPT_H

// trscript.h
// 9/20/2014 jichi

#include "sakurakit/skglobal.h"
#include <QtCore/QString>

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
  bool loadFile(const QString &path);

  // Replacement

  // Rewrite the text according to the script, thread-safe
  QString translate(const QString &text) const;
};

#endif // TRSCRIPT_H
