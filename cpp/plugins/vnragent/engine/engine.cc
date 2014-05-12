// engine.cc
// 4/20/2014 jichi

#include "engine/engine.h"
#include "engine/enginehash.h"
#include "engine/engineloader.h"
#include "engine/enginesettings.h"
#include "embed/embedmanager.h"
#include "util/codepage.h"
#include "util/textutil.h"
#include "winkey/winkey.h"
#include <qt_windows.h>
#include <QtCore/QTextCodec>

/** Private class */

class AbstractEnginePrivate
{
  typedef AbstractEngine Q;

  static const char *encodingName(Q::Encoding v)
  {
    switch (v) {
    case Utf16Encoding: return ENC_UTF16;
    case SjisEncoding: return ENC_SJIS;
    default: return nullptr;
    }
  }

public:
  const char *name;
  Q::Encoding encoding;
  Q::RequiredAttributes attributes;

  QTextCodec *encoder,
             *decoder;

  EngineSettings *settings;

  AbstractEnginePrivate(const char *name, Q::Encoding encoding, Q::RequiredAttributes attributes)
    :  name(name), encoding(encoding), attributes(attributes)
    , settings(new EngineSettings)
  {
    const char *engineEncoding = encodingName(encoding);
    decoder = engineEncoding ? QTextCodec::codecForName(engineEncoding) : nullptr;

    const char *systemEncoding = Util::encodingForCodePage(::GetACP());
    encoder = QTextCodec::codecForName(systemEncoding ? systemEncoding : ENC_SJIS);
  }

  ~AbstractEnginePrivate() { delete settings; }

  // Utilities

  QByteArray encode(const QString &text) const
  { return encoder ? encoder->fromUnicode(text) : text.toLocal8Bit(); }

  QString decode(const QByteArray &data) const
  { return decoder ? decoder->toUnicode(data) : QString::fromLocal8Bit(data); }

  //QByteArray transcode(const QByteArray &data) const
  //{ return !encoder || !decoder || encoder == decoder ? data : encoder->fromUnicode(decoder->toUnicode(data)); }

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

AbstractEngine::AbstractEngine(const char *name, Encoding encoding, RequiredAttributes attributes)
  : d_(new D(name, encoding, attributes)) {}

AbstractEngine::~AbstractEngine() { delete d_; }

EngineSettings *AbstractEngine::settings() const { return d_->settings; }
const char *AbstractEngine::name() const { return d_->name; }
const char *AbstractEngine::encoding() const { return d_->encoding; }

bool AbstractEngine::isTranscodingNeeded() const
{ return d_->encoder != d_->decoder; }

// - Dispatch -

QByteArray AbstractEngine::dispatchTextA(const QByteArray &data, int role) const
{
  QString text = d_->decode(data);
  if (text.isEmpty())
    return data;

  auto p = EmbedManager::instance();

  bool canceled = !d_->settings->enabled ||
      d_->settings->detectsControl && WinKey::isKeyControlPressed();

  qint64 hash = canceled ? 0 : Engine::hashByteArray(data);
  if (!canceled && d_->settings->extractionEnabled[role] && !d_->settings->translationEnabled[role]) {
    enum { NeedsTranslation = false };
    p->sendText(text, hash, role, NeedsTranslation);
  }

  if (!d_->settings->textVisible[role])
    return QByteArray();

  if (!d_->settings->translationEnabled[role])
    return d_->settings->transcodingEnabled[role] ? d_->encode(text) : data;
  if (canceled ||
      role == Engine::OtherRole && !Util::needsTranslation(text))
    return d_->encode(text);

  QString repl = p->findTranslation(hash, role);
  bool needsTranslation = repl.isEmpty();
  p->sendText(text, hash, role, needsTranslation);
  if (needsTranslation && d_->attributes & BlockingRequired)
    repl = p->waitForTranslation(hash, role);

  if (repl.isEmpty())
    repl = text;
  else if (role == Engine::NameRole && d_->settings->nameTextVisible && repl != text)
    repl = QString("%1(%2)").arg(text, repl);

  return d_->encode(repl);
}

//QString AbstractEngine::dispatchTextW(const QString &text, int role, bool blocking) const
//{
//  if (text.isEmpty() || !d_->settings->textVisible[role])
//    return QString();
//
//  qint64 hash = Engine::hashString(text);
//  auto p = EmbedManager::instance();
//  QString repl = p->findTranslation(hash, role);
//  p->addText(text, hash, role, repl.isEmpty());
//  if (blocking && repl.isEmpty())
//    repl = p->waitForTranslation(hash, role);
//  if (repl.isEmpty())
//    repl = text;
//
//  return repl;
//}

// - Exchange -

const char *AbstractEngine::exchangeTextA(const char *data, int size, int role)
{
  if (!data || size <= 0)
    return data;
  return data;
}

// EOF
