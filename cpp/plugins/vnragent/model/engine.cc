// majiro.cc
// 4/20/2014 jichi

#include "model/engine.h"
#include "model/manifest.h"
#include "engine/enginedriver.h"
#include "engine/enginehash.h"
#include <QtCore/QTextCodec>
#include <QtCore/QTextDecoder>
#include <QtCore/QTextEncoder>

/** Private class */

class AbstractEnginePrivate
{
  QTextCodec *codec;
  QTextEncoder *encoder;
  QTextDecoder *decoder;
public:
  const char *name,
             *encoding;

  AbstractEnginePrivate(const char *name, const char *encoding)
    : codec(nullptr), encoder(nullptr), decoder(nullptr),
      name(name), encoding(encoding)
  {
    if (encoding) {
      codec = QTextCodec::codecForName(encoding);
      encoder = codec->makeEncoder();
      decoder = codec->makeDecoder();
    }
  }

  QByteArray encode(const QString &text) const
  { return encoder ? encoder->fromUnicode(text) : QByteArray(); }

  QString decode(const QByteArray &data) const
  { return decoder ? decoder->toUnicode(data) : QString(); }
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

const char *AbstractEngine::name() const { return d_->name; }
const char *AbstractEngine::encoding() const { return d_->encoding; }

// - Translate -

QString AbstractEngine::translate(const QByteArray &data) const
{
  if (auto p = EngineDriver::instance())
    if (p->isEnabled()) {
      QString text = d_->decode(data);
      if (!text.isEmpty()) {
        qint64 hash = Engine::hashByteArray(data);
        return p->translate(text, hash);
      }
    }
  return QString();
}

// EOF
