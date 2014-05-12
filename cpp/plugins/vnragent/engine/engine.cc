// engine.cc
// 4/20/2014 jichi

#include "engine/engine.h"
#include "engine/engine_p.h"
#include "engine/enginehash.h"
#include "engine/engineloader.h"
#include "engine/enginememory.h"
#include "engine/enginesettings.h"
#include "embed/embedmanager.h"
#include "util/codepage.h"
#include "util/textutil.h"
#include "winkey/winkey.h"
#include <qt_windows.h>
#include <QtCore/QTimer>
#include <QtCore/QTextCodec>

/** Private class */

AbstractEnginePrivate::AbstractEnginePrivate(Q *q, const char *name, Q::Encoding encoding, Q::RequiredAttributes attributes)
  :  q_(q)
  , name(name), encoding(encoding), attributes(attributes)
  , encoder(nullptr), decoder(nullptr)
  , settings(new EngineSettings)
  , exchangeMemory(nullptr), exchangeTimer(nullptr)
{}

AbstractEnginePrivate::~AbstractEnginePrivate()
{
  delete settings;
  if (exchangeMemory)
    delete exchangeMemory;
}

void AbstractEnginePrivate::finalize()
{
  finalizeCodecs();
  if (testAttribute(Q::ExchangeAttribute))
    startExchange();
}

void AbstractEnginePrivate::startExchange()
{
  exchangeMemory = new EngineSharedMemory;

  exchangeTimer = new QTimer(this);
  exchangeTimer->setSingleShot(false);
  exchangeTimer->setInterval(ExchangeInterval);
  connect(exchangeTimer, SIGNAL(timeout()), SLOT(exchange()));

  exchangeTimer->start();
}

void AbstractEnginePrivate::finalizeCodecs()
{
  const char *engineEncoding = Q::encodingName(encoding);
  decoder = engineEncoding ? QTextCodec::codecForName(engineEncoding) : nullptr;

  const char *systemEncoding = Util::encodingForCodePage(::GetACP());
  encoder = QTextCodec::codecForName(systemEncoding ? systemEncoding : ENC_SJIS);
}

 // Encoding

QByteArray AbstractEnginePrivate::encode(const QString &text) const
{ return encoder ? encoder->fromUnicode(text) : text.toLocal8Bit(); }

QString AbstractEnginePrivate::decode(const QByteArray &data) const
{ return decoder ? decoder->toUnicode(data) : QString::fromLocal8Bit(data); }

// Exchange

void AbstractEnginePrivate::exchange()
{
  if (!exchangeMemory)
    return;
  if (exchangeMemory->requestStatus() == EngineSharedMemory::ReadyStatus) {
    exchangeMemory->setRequestStatus(EngineSharedMemory::EmptyStatus);
    if (auto req = exchangeMemory->requestText()) {
      auto key = exchangeMemory->requestKey();
      auto role = exchangeMemory->requestRole();
      auto sig = exchangeMemory->requestSignature();
      QByteArray resp = q_->dispatchTextA(req, sig, role);
      exchangeMemory->setResponseStatus(EngineSharedMemory::BusyStatus);
      exchangeMemory->setResponseText(resp);
      exchangeMemory->setResponseRole(role);
      exchangeMemory->setResponseKey(key);
      exchangeMemory->setResponseStatus(EngineSharedMemory::ReadyStatus);
    }
  }
}

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

const char *AbstractEngine::encodingName(Encoding v)
{
  switch (v) {
  case Utf16Encoding: return ENC_UTF16;
  case SjisEncoding: return ENC_SJIS;
  default: return nullptr;
  }
}

AbstractEngine::AbstractEngine(const char *name, Encoding encoding, RequiredAttributes attributes)
  : d_(new D(this, name, encoding, attributes)) {}

AbstractEngine::~AbstractEngine() { delete d_; }

EngineSettings *AbstractEngine::settings() const { return d_->settings; }
const char *AbstractEngine::name() const { return d_->name; }
AbstractEngine::Encoding AbstractEngine::encoding() const { return d_->encoding; }

bool AbstractEngine::isTranscodingNeeded() const
{ return d_->encoder != d_->decoder; }

// - Attach -

bool AbstractEngine::load()
{
  bool ok = attach();
  if (ok)
    d_->finalize();
  return ok;
}

bool AbstractEngine::unload()
{
  if ( d_->exchangeTimer)
    d_->exchangeTimer->stop();
  return detach();
}

// - Dispatch -

QByteArray AbstractEngine::dispatchTextA(const QByteArray &data, long signature, int role)
{
  QString text = d_->decode(data);
  if (text.isEmpty())
    return data;

  if (!role)
    role = d_->settings->textRoleOf(signature);

  auto p = EmbedManager::instance();

  bool canceled = !d_->settings->enabled ||
      d_->settings->detectsControl && WinKey::isKeyControlPressed();

  qint64 hash = canceled ? 0 : Engine::hashByteArray(data);
  if (!canceled && d_->settings->extractionEnabled[role] && !d_->settings->translationEnabled[role]) {
    enum { NeedsTranslation = false };
    p->sendText(text, hash, signature, role, NeedsTranslation);
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
  p->sendText(text, hash, signature, role, needsTranslation);
  if (needsTranslation && d_->testAttribute(BlockingAttribute))
    repl = p->waitForTranslation(hash, role);

  if (repl.isEmpty())
    repl = text;
  else if (role == Engine::NameRole && d_->settings->nameTextVisible && repl != text)
    repl = QString("%1(%2)").arg(text, repl);

  return d_->encode(repl);
}

//QString AbstractEngine::dispatchTextW(const QString &text, int role, bool blocking)
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

// Qt is not allowed to appear in this function
const char *AbstractEngine::exchangeTextA(const char *data, long signature, int role)
{
  auto d_mem = d_->exchangeMemory;
  if (!d_mem || !data)
    return data;

  ulong key = ::GetTickCount();
  d_mem->setRequestStatus(EngineSharedMemory::BusyStatus);
  d_mem->setRequestKey(key);
  d_mem->setRequestSignature(signature);
  d_mem->setRequestRole(role);
  d_mem->setRequestText(data);
  d_mem->setRequestStatus(EngineSharedMemory::ReadyStatus);

  // Spin lock
  while (d_mem->responseKey() != key ||
         d_mem->responseStatus() != EngineSharedMemory::ReadyStatus) {
    if (d_mem->responseStatus() == EngineSharedMemory::CancelStatus)
      return data;
    ::Sleep(D::ExchangeInterval);
  }
  return d_mem->responseText();
}

// EOF
