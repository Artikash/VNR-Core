#pragma once

// engine.h
// 4/20/2014 jichi

#include "sakurakit/skglobal.h"
#include <QtCore/QString>

class AbstractEnginePrivate;
class AbstractEngine
{
  SK_CLASS(AbstractEngine)
  SK_DISABLE_COPY(AbstractEngine)
  SK_DECLARE_PRIVATE(AbstractEnginePrivate)

public:
  static Self *instance();

  AbstractEngine(const char *name, const char *encoding);
  virtual ~AbstractEngine();

  const char *name() const;
  const char *encoding() const;

  virtual bool inject() = 0;

  //static bool isEnabled();
  //static void setEnabled(bool t);

  // Helper functions only used by descendant classes
public:
  QString translate(const QByteArray &data) const;
};

// EOF
