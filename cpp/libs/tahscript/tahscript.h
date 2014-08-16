#ifndef TAHSCRIPT_H
#define TAHSCRIPT_H

// tahscript.h
// 8/14/2014 jichi

#include "sakurakit/skglobal.h"
#include <QtCore/QString>

class TahScriptManagerPrivate;
class TahScriptManager
{
  SK_CLASS(TahScriptManager)
  SK_DISABLE_COPY(TahScriptManager)
  SK_DECLARE_PRIVATE(TahScriptManagerPrivate)

  // - Construction -
public:
  explicit TahScriptManager();
  ~TahScriptManager();

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

#endif // TAHSCRIPT_H
