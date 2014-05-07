// engine.cc
// 4/20/2014 jichi

#include "engine/engine.h"
#include "engine/enginehash.h"
#include "engine/engineloader.h"
#include "embed/embedmanager.h"
#include "util/codepage.h"
#include <QtCore/QTextCodec>

/** Private class */

class AbstractEnginePrivate
{
  QTextCodec *encoder,
             *decoder;
  const char *systemEncoding;
public:
  const char *name,
             *encoding;

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
  { return !encoder || !decoder ? data : decoder->fromUnicode(encoder->toUnicode(data)); }

};

/** Public class */

// - Detection -

static AbstractEngine *instance_;

AbstractEngine *AbstractEngine::instance()
{
  if (!::instance_)
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

// - Dispatch -

QByteArray AbstractEngine::dispatchTextA(const QByteArray &data, int role, bool blocking) const
{
  if (!d_->settings->textVisible[role])
    return QByteArray();
  if (!d_->settings->translationEnabled[role] && d_->settings->transcodingEnabled[role])
    return d_->transcode(data);

  QString text = d_->decode(data);
  if (text.isEmpty())
    return data;

  qint64 hash = Engine::hashByteArray(data);
  auto p = EmbedManager::instance();
  QString ret = p->findTranslation(hash, role);
  p->addText(text, hash, role, ret.isEmpty());
  if (blocking && ret.isEmpty())
    ret = p->waitForTranslation(hash, role);

  return d_->encode(ret);
}

// EOF
