// majiro.cc
// 4/20/2014 jichi

#include "model/engine.h"
#include "model/manifest.h"
#include "engine/enginemanager.h"
#include "engine/enginehash.h"
#include <QtCore/QTextCodec>

/** Private class */

class AbstractEnginePrivate
{
  QTextCodec *codec;
public:
  const char *name,
             *encoding;

  AbstractEnginePrivate(const char *name, const char *encoding)
    : codec(nullptr),
      name(name), encoding(encoding)
  {
    if (encoding)
      codec = QTextCodec::codecForName(encoding);
  }

  QByteArray encode(const QString &text) const
  { return encoder ? codec->fromUnicode(text) : QByteArray(); }

  QString decode(const QByteArray &data) const
  { return decoder ? codec->toUnicode(data) : QString(); }
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

// - Dispatch -

QString AbstractEngine::dispatchText(const QByteArray &data, int role, bool blocking) const
{
  QString text = d_->decode(data);
  if (!text.isEmpty()) {
    qint64 hash = Engine::hashByteArray(data);
    auto p = EngineManager::instance();
    p->addText(text, hash, role);
    return blocking ? p->waitForTranslation(hash, role)
                    : p->findTranslation(hash, role);
  }
  return QString();
}

// EOF
