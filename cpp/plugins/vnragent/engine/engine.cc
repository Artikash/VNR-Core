// engine.cc
// 4/20/2014 jichi

#include "engine/engine.h"
#include "engine/enginehash.h"
#include "engine/engineloader.h"
#include "embed/embedmanager.h"
#include "util/codepage.h"
#include <qt_windows.h>
#include <QtCore/QTextCodec>

/** Private class */

class AbstractEnginePrivate
{
public:
  const char *name,
             *encoding;

  const char *systemEncoding;
  QTextCodec *encoder,
             *decoder;

  EngineSettings *settings;

  AbstractEnginePrivate(const char *name, const char *encoding)
    :  name(name), encoding(encoding)
    , settings(new EngineSettings)
  {
    decoder = encoding ? QTextCodec::codecForName(encoding) : nullptr;

    systemEncoding = Util::encodingForCodePage(::GetACP());
    if (!systemEncoding)
      systemEncoding = ENC_SJIS;
    encoder = QTextCodec::codecForName(systemEncoding);
  }

  ~AbstractEnginePrivate() { delete settings; }

  // Utilities

  QByteArray encode(const QString &text) const
  { return encoder ? encoder->fromUnicode(text) : text.toLocal8Bit(); }

  QString decode(const QByteArray &data) const
  { return decoder ? decoder->toUnicode(data) : QString::fromLocal8Bit(data); }

  QByteArray transcode(const QByteArray &data) const
  { return !encoder || !decoder || encoder == decoder ? data : encoder->fromUnicode(decoder->toUnicode(data)); }

};

/** Public class */

// - Detection -

#define INVALID_INSTANCE ((AbstractEngine *)-1)

static AbstractEngine *instance_ = INVALID_INSTANCE;

AbstractEngine *AbstractEngine::instance()
{
  if (::instance_ == INVALID_INSTANCE)
    ::instance_ = Engine::getEngine();
  return ::instance_;
}

// - Construction -

AbstractEngine::AbstractEngine(const char *name, const char *encoding)
  : d_(new D(name, encoding)) {}

AbstractEngine::~AbstractEngine() { delete d_; }

EngineSettings *AbstractEngine::settings() const { return d_->settings; }
const char *AbstractEngine::name() const { return d_->name; }
const char *AbstractEngine::encoding() const { return d_->encoding; }

bool AbstractEngine::isTranscodingNeeded() const
{ return d_->encoder != d_->decoder; }

// - Dispatch -

QByteArray AbstractEngine::dispatchTextA(const QByteArray &data, int role, bool blocking) const
{
  if (data.isEmpty() || !d_->settings->textVisible[role])
    return QByteArray();
  if (!d_->settings->translationEnabled[role] && d_->settings->transcodingEnabled[role]) {
    return d_->transcode(data);
  }

  QString text = d_->decode(data);
  if (text.isEmpty())
    return data;

  qint64 hash = Engine::hashByteArray(data);
  auto p = EmbedManager::instance();
  QString repl = p->findTranslation(hash, role);
  p->addText(text, hash, role, repl.isEmpty());
  if (blocking && repl.isEmpty())
    repl = p->waitForTranslation(hash, role);

  return d_->encode(repl);
}

QString AbstractEngine::dispatchTextW(const QString &text, int role, bool blocking) const
{
  if (text.isEmpty() || !d_->settings->textVisible[role])
    return QString();

  qint64 hash = Engine::hashString(text);
  auto p = EmbedManager::instance();
  QString repl = p->findTranslation(hash, role);
  p->addText(text, hash, role, repl.isEmpty());
  if (blocking && repl.isEmpty())
    repl = p->waitForTranslation(hash, role);

  return repl;
}

// EOF
