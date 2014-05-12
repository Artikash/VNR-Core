#pragma once

// engine.h
// 4/20/2014 jichi

#include "sakurakit/skglobal.h"
#include <QtCore/QString>

class EngineSettings;
class AbstractEnginePrivate;
class AbstractEngine
{
  //Q_OBJECT
  SK_CLASS(AbstractEngine)
  SK_DECLARE_PRIVATE(AbstractEnginePrivate)
  SK_DISABLE_COPY(AbstractEngine)

public:
  enum Encoding { UnknownEncoding = 0, Utf16Encoding, SjisEncoding };

  typedef ulong RequiredAttributes;
  enum RequiredAttribute {
    BlockingAttribute = 1           // non-blocking callback is not supported
    , ExchangeAttribute = 1 << 1    // qt is not supported and it requires exchanging data
  };

  static Self *instance(); // Needed to be explicitly deleted on exit

  AbstractEngine(const char *name, Encoding enc, RequiredAttributes flags = 0);
  virtual ~AbstractEngine();

  EngineSettings *settings() const;

  const char *name() const;
  const char *encoding() const;

  bool isTranscodingNeeded() const;

  virtual bool inject() = 0;
  virtual bool unload() = 0;

  //static bool isEnabled();
  //static void setEnabled(bool t);

public: // only needed by descendants
  QByteArray dispatchTextA(const QByteArray &data, int role) const;
  //QString dispatchTextW(const QString &text, int role, bool blocking = true) const;

  // This function is not thread-safe
  const char *exchangeTextA(const char *data, int size, int role);
};

// EOF

//signals:
//  // context is opaque, and the receiver of this signal is responsible to release the context
//  void textReceived(const QString &text, qint64 hash, int role, void *context);
//public:
//  virtual void drawText(const QString &text, const void *context) = 0;
//  virtual void releaseContext(void *context) = 0;
