// engine.cc
// 4/20/2014 jichi

#include "config.h"
#include "engine/engine.h"
#include "engine/engineenv.h"
#include "engine/enginehash.h"
#include "engine/engineloader.h"
//#include "engine/enginememory.h"
#include "engine/enginesettings.h"
#include "embed/embedmanager.h"
#include "detoursutil/detoursutil.h"
#include "util/codepage.h"
#include "util/textutil.h"
#include "winkey/winkey.h"
#include <qt_windows.h>
//#include "mhook/mhook.h" // must after windows.h
#include <QtCore/QTimer>
#include <QtCore/QTextCodec>

#define DEBUG "engine"
#include "sakurakit/skdebug.h"

/** Private class */

class AbstractEnginePrivate
{
  typedef AbstractEngine Q;

  static Q::address_type globalOldHookFun;
  static Q::hook_function globalDispatchFun;
public:

  enum { ExchangeInterval = 10 };

  const char *name;
  uint codePage;
  //Q::RequiredAttributes attributes;

  QTextCodec *encoder,
             *decoder;

  Q::address_type oldHookFun;
  Q::hook_function dispatchFun;

  Q::filter_function textFilter,
                     translationFilter;

  EngineSettings *settings;
  bool finalized;

public:
  AbstractEnginePrivate()
    : name(""), codePage(0)
    , encoder(nullptr), decoder(nullptr)
    , dispatchFun(0), oldHookFun(0)
    , textFilter(nullptr), translationFilter(nullptr)
    , settings(new EngineSettings)
    , finalized(false)
  {}

  ~AbstractEnginePrivate() { delete settings; }

  void finalize()
  {
    if (!finalized) {
      finalizeCodecs();

      globalOldHookFun = oldHookFun;
      globalDispatchFun = dispatchFun;

      finalized = true;
    }
  }

  // Property helpers

  bool testAttribute(Q::RequiredAttribute v) const { return attributes & v; }
  bool testAttributes(Q::RequiredAttributes v) const { return attributes & v; }

  // Encoding

  QByteArray encode(const QString &text) const
  { return encoder ? encoder->fromUnicode(text) : text.toLocal8Bit(); }

  QString decode(const QByteArray &data) const
  { return decoder ? decoder->toUnicode(data) : QString::fromLocal8Bit(data); }

private:
  void finalizeCodecs()
  {
    const char *engineEncoding = Util::encodingForCodePage(codePage);
    decoder = engineEncoding ? QTextCodec::codecForName(engineEncoding) : nullptr;

    const char *systemEncoding = Util::encodingForCodePage(::GetACP());
    //systemEncoding = "gbk";
    encoder = QTextCodec::codecForName(systemEncoding ? systemEncoding : ENC_SJIS);

    DOUT("encoding =" << engineEncoding  << ", system =" << systemEncoding);
  }

public:
};

AbstractEngine::address_type AbstractEnginePrivate::globalOldHookFun;
AbstractEngine::hook_function AbstractEnginePrivate::globalDispatchFun;

/**
 *  The stack must be consistent with struct HookStack
 *
 *  Note for detours
 *  - It simply replaces the code with jmp and int3. Jmp to newHookFun
 *  - oldHookFun is the address to a code segment that jmp back to the original function
 */

__declspec(naked) static int newHookFun()
{
  //static DWORD lastArg2;
  __asm // consistent with struct HookStack
  {
    pushad              // increase esp by 0x20 = 4 * 8, push ecx for thiscall is enough, though
    pushfd              // eflags
    push esp            // arg1
    call AbstractEnginePrivate::globalDispatchFun
    add esp,4           // pop esp
    popfd
    popad
    // TODO: instead of jmp, allow modify the stack after calling the function
    jmp AbstractEnginePrivate::globalOldHookFun
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

AbstractEngine::AbstractEngine() : d_(new D) {}

AbstractEngine::~AbstractEngine() { delete d_; }

EngineSettings *AbstractEngine::settings() const { return d_->settings; }
const char *AbstractEngine::name() const { return d_->name; }

void AbstractEngine::setName(const char *v) { d_->name = v; }

const char *AbstractEngine::encoding() const
{ return Util::encodingForCodePage(d_->codePage); }

void AbstractEngine::setCodePage(uint v)
{
  if (v != d_->codePage) {
    d_->codePage = v;

    if (d_->finalized) {
      const char *encoding = Util::encodingForCodePage(v);
      d_->decoder = encoding ? QTextCodec::codecForName(encoding) : nullptr;
    }
  }
}

void AbstractEngine::setEncoding(const QString &v)
{ setCodePage(Util::codePageForEncoding(v)); }

void AbstractEngine::setWideChar(bool t)
{ setEncoding(t ? ENC_UTF16 : ENC_SJIS); }

bool AbstractEngine::isTranscodingNeeded() const
{ return d_->encoder != d_->decoder; }

void AbstractEngine::setHookFunction(hook_function v)
{ d_->dispatchFun = v; }

void AbstractEngine::setTextFilter(filter_function v)
{ d_->textFilter = v; }

void AbstractEngine::setTranslationFilter(filter_function v)
{ d_->translationFilter = v; }

// - Attach -

bool AbstractEngine::load()
{
  bool ok = attach();
  if (ok)
    d_->finalize();
  return ok;
}

bool AbstractEngine::unload() { return detach(); }

// - Exists -

bool AbstractEngine::matchFiles(const QStringList &relpaths)
{
  if (relpaths.isEmpty())
    return false;
  foreach (const QString &path, relpaths)
    if (path.contains('*') && !Engine::globs(path)
        || !Engine::exists(path))
      return false;
  return true;
}

// - Detours -

AbstractEngine::address_type AbstractEngine::replaceFunction(address_type old_addr, const_address_type new_addr)
{
#ifdef VNRAGENT_ENABLE_DETOURS
  return detours::replace(old_addr, new_addr);
#endif // VNRAGENT_ENABLE_DETOURS
#ifdef VNRAGENT_ENABLE_MHOOK
  DWORD addr = old_addr;
  return Mhook_SetHook (&addr, new_addr) ? addr : 0;
#endif // VNRAGENT_ENABLE_MHOOK
}

// Not used
//AbstractEngine::address_type AbstractEngine::restoreFunction(address_type restore_addr, const_address_type old_addr)
//{
//#ifdef VNRAGENT_ENABLE_DETOURS
//  return detours::restore(restore_addr, old_addr);
//#endif // VNRAGENT_ENABLE_DETOURS
//}

bool AbstractEngine::hookAddress(ulong addr)
{ return d_->oldHookFun = !addr ? 0 : replaceFunction<address_type>(addr, ::newHookFun); }

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
  if (!canceled && !d_->settings->translationEnabled[role] &&
      (d_->settings->extractionEnabled[role] || d_->settings->extractsAllTexts)) {
    enum { NeedsTranslation = false };
    if (d_->textFilter) {
      QString t = d_->textFilter(text, role);
      if (!t.isEmpty())
        p->sendText(t, hash, signature, role, NeedsTranslation);
    } else
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
  if (d_->textFilter) {
    QString t = d_->textFilter(text, role);
    if (!t.isEmpty())
      p->sendText(t, hash, signature, role, needsTranslation);
    else
      return data;
  } else
    p->sendText(text, hash, signature, role, needsTranslation);

  if (needsTranslation && d_->testAttribute(BlockingAttribute)) {
    repl = p->waitForTranslation(hash, role);
    if (!repl.isEmpty() && d_->translationFilter)
      repl = d_->translationFilter(repl, role);
  }

  if (repl.isEmpty())
    repl = text;
  else if (role == Engine::NameRole && d_->settings->nameTextVisible && repl != text)
    repl = QString("%1(%2)").arg(text, repl);

  return d_->encode(repl);
}

QString AbstractEngine::dispatchTextW(const QString &text, long signature, int role)
{
  if (text.isEmpty())
    return text;
  if (!role)
    role = d_->settings->textRoleOf(signature);

  auto p = EmbedManager::instance();

  bool canceled = !d_->settings->enabled ||
      d_->settings->detectsControl && WinKey::isKeyControlPressed();

  qint64 hash = canceled ? 0 : Engine::hashWString(text);
  if (!canceled && !d_->settings->translationEnabled[role] &&
      (d_->settings->extractionEnabled[role] || d_->settings->extractsAllTexts)) {
    enum { NeedsTranslation = false };
    if (d_->textFilter) {
      QString t = d_->textFilter(text, role);
      if (!t.isEmpty())
        p->sendText(t, hash, signature, role, NeedsTranslation);
    } else
      p->sendText(text, hash, signature, role, NeedsTranslation);
  }

  if (!d_->settings->textVisible[role])
    return QString();

  if (!d_->settings->translationEnabled[role])
    return text;
    //return d_->settings->transcodingEnabled[role] ? d_->encode(data) : data;
  if (canceled ||
      role == Engine::OtherRole && !Util::needsTranslation(text))
    return text;
    //return d_->encode(data);

  QString repl = p->findTranslation(hash, role);
  bool needsTranslation = repl.isEmpty();
  if (d_->textFilter) {
    QString t = d_->textFilter(text, role);
    if (!t.isEmpty())
      p->sendText(t, hash, signature, role, needsTranslation);
    else
      return text;
  } else
    p->sendText(text, hash, signature, role, needsTranslation);

  if (needsTranslation && d_->testAttribute(BlockingAttribute)) {
    repl = p->waitForTranslation(hash, role);
    if (!repl.isEmpty() && d_->translationFilter)
      repl = d_->translationFilter(repl, role);
  }

  if (repl.isEmpty())
    repl = text;
  else if (role == Engine::NameRole && d_->settings->nameTextVisible && repl != text)
    repl = QString("%1(%2)").arg(text, repl);

  return repl;
  //return d_->encode(repl);
}

// EOF

/*
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
*/
