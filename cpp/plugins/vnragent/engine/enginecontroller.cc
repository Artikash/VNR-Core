// enginecontroller.cc
// 4/20/2014 jichi

#include "config.h"
#include "engine/enginecontroller.h"
#include "engine/enginehash.h"
#include "engine/enginemodel.h"
#include "engine/engineutil.h"
//#include "engine/enginememory.h"
#include "engine/enginesettings.h"
#include "embed/embedmanager.h"
#include "util/codepage.h"
#include "util/textutil.h"
//#include "windbg/util.h"
#include "winhook/winhook.h"
#include "winkey/winkey.h"
#include <qt_windows.h>
//#include "mhook/mhook.h" // must after windows.h
#include <QtCore/QTimer>
#include <QtCore/QTextCodec>

#define DEBUG "enginecontroller"
#include "sakurakit/skdebug.h"

/** Private class */

class EngineControllerPrivate
{
  typedef EngineController Q;

  static Engine::address_type globalOldHookFun;
public:
  static EngineModel::hook_function globalDispatchFun;
  static Q *globalInstance;

  enum { ExchangeInterval = 10 };

  EngineSettings settings;

  EngineModel *model;

  uint codePage;
  //Q::RequiredAttributes attributes;

  QTextCodec *encoder,
             *decoder,
             *spaceCodec;

  Engine::address_type oldHookFun;

  bool finalized;

public:
  EngineControllerPrivate(EngineModel *model)
    : model(model)
    , codePage(0)
    , encoder(nullptr), decoder(nullptr), spaceCodec(nullptr)
    , oldHookFun(0)
    , finalized(false)
  {}

  void finalize()
  {
    if (!finalized) {
      finalizeCodecs();

      globalOldHookFun = oldHookFun;
      globalDispatchFun = model->hookFunction;

      finalized = true;
    }
  }

  // Property helpers

  //bool testAttribute(Q::RequiredAttribute v) const { return attributes & v; }
  //bool testAttributes(Q::RequiredAttributes v) const { return attributes & v; }

  // Encoding

  QByteArray encode(const QString &text) const
  { return encoder ? encoder->fromUnicode(text) : text.toLocal8Bit(); }

  QByteArray encode(const QChar &c) const
  { return encoder ? encoder->fromUnicode(&c, 1) : QByteArray(); }

  QString decode(const QByteArray &data) const
  { return decoder ? decoder->toUnicode(data) : QString::fromLocal8Bit(data); }

private:
  void finalizeCodecs()
  {
    const char *engineEncoding = Util::encodingForCodePage(codePage);
    decoder = engineEncoding ? Util::codecForName(engineEncoding) : nullptr;

    const char *systemEncoding = Util::encodingForCodePage(::GetACP());
    //systemEncoding = "gbk";
    //systemEncoding = ENC_KSC;
    encoder = Util::codecForName(systemEncoding ? systemEncoding : ENC_SJIS);

    DOUT("encoding =" << engineEncoding  << ", system =" << systemEncoding);
  }

  static QString alwaysInsertSpaces(const QString &text)
  {
    QString ret;
    foreach (QChar c, text) {
      ret.append(c);
      if (c.unicode() >= 32) // ignore non-printable characters
        ret.append(' '); // or insert \u3000 if needed
    }
    return ret;
  }

  static QString insertSpacesAfterUnencodable(const QString &text, const QTextCodec *codec)
  {
    QString ret;
    foreach (const QChar &c, text) {
      ret.append(c);
      if (!Util::charEncodable(c, codec))
        ret.append(' ');
    }
    return ret;
  }

public:
  QString postProcessW(const QString &text) const
  {
    if (settings.alwaysInsertsSpaces)
      return alwaysInsertSpaces(text);
    if (settings.smartInsertsSpaces) {
      auto codec = spaceCodec ? spaceCodec : encoder;
      if (codec)
        return insertSpacesAfterUnencodable(text, codec);
    }
    return text;
  }
};

EngineController *EngineControllerPrivate::globalInstance;
Engine::address_type EngineControllerPrivate::globalOldHookFun;
EngineModel::hook_function EngineControllerPrivate::globalDispatchFun;

/**
 *  The stack must be consistent with struct HookStack
 *
 *  Note for detours
 *  - It simply replaces the code with jmp and int3. Jmp to newHookFun
 *  - oldHookFun is the address to a code segment that jmp back to the original function
 */

__declspec(naked) static int newHookFun()
{
  // The push order must be consistent with struct HookStack in enginemodel.h
  //static DWORD lastArg2;
  __asm // consistent with struct HookStack
  {
    //pushfd      // 5/25/2015: pushfd twice according to ith, not sure if it is really needed
    pushad      // increase esp by 0x20 = 4 * 8, push ecx for thiscall is enough, though
    pushfd      // eflags
    push esp    // arg1
    call EngineControllerPrivate::globalDispatchFun
    //add esp,4   // pop esp
    popfd
    popad
    //popfd
    // TODO: instead of jmp, allow modify the stack after calling the function
    jmp EngineControllerPrivate::globalOldHookFun
  }
}

/** Public class */

// - Detection -

EngineController *EngineController::instance() { return D::globalInstance; }

// - Construction -

EngineController::EngineController(EngineModel *model)
  : d_(new D(model))
{
  setEncoding(model->wideChar ? ENC_UTF16 : ENC_SJIS);
}

EngineController::~EngineController() { delete d_; }

EngineSettings *EngineController::settings() const { return &d_->settings; }
const char *EngineController::name() const { return d_->model->name; }

const char *EngineController::encoding() const
{ return Util::encodingForCodePage(d_->codePage); }

void EngineController::setCodePage(uint v)
{
  if (v != d_->codePage) {
    d_->codePage = v;

    if (d_->finalized) {
      const char *encoding = Util::encodingForCodePage(v);
      d_->decoder = encoding ? Util::codecForName(encoding) : nullptr;
    }
  }
}

void EngineController::setEncoding(const QString &v)
{ setCodePage(Util::codePageForEncoding(v)); }

bool EngineController::isTranscodingNeeded() const
{ return d_->encoder != d_->decoder; }

void EngineController::setSpacePolicyEncoding(const QString &v)
{ d_->spaceCodec = v.isEmpty() ? nullptr : Util::codecForName(v.toAscii()); }

// - Attach -

bool EngineController::attach()
{
  if (d_->model->attachFunction)
    return d_->model->attachFunction();
  if (!d_->model->searchFunction)
    return false;
  ulong startAddress,
        stopAddress;
  if (!Engine::getCurrentMemoryRange(&startAddress, &stopAddress))
    return false;
  ulong addr = d_->model->searchFunction(startAddress, stopAddress);
  //ulong addr = startAddress + 0x31850; // 世界と世界の真ん中 体験版
  //ulong addr = 0x41af90; // レミニセンス function address
  if (addr) {
    DOUT("attached, engine =" << name() << ", absaddr =" << QString::number(addr, 16) << "reladdr =" << QString::number(addr - startAddress, 16));
    auto fun = d_->globalDispatchFun;
    auto callback = [addr, fun](winhook::hook_stack *s) {
      fun((EngineModel::HookStack *)s);
    };
    winhook::hook(addr, callback);

    //WinDbg::ThreadsSuspender suspendedThreads; // lock all threads to prevent crashing
    //d_->oldHookFun = Engine::replaceFunction<Engine::address_type>(addr, ::newHookFun);
    return true;
  }
  return false;
}

bool EngineController::load()
{
  bool ok = attach();
  if (ok) {
    d_->finalize();
    D::globalInstance = this;
  }
  return ok;
}

bool EngineController::unload() { return false; }

// - Exists -

bool EngineController::match()
{
  return d_->model->matchFunction ?
      d_->model->matchFunction() :
      matchFiles(d_->model->matchFiles);
}

bool EngineController::matchFiles(const QStringList &relpaths)
{
  if (relpaths.isEmpty())
    return false;
  foreach (const QString &path, relpaths)
    if (!(path.contains('*') && Engine::globs(path)
        || Engine::exists(path)))
      return false;

  DOUT("ret = true, relpaths =" << relpaths);
  return true;
}

// - Dispatch -

QByteArray EngineController::dispatchTextA(const QByteArray &data, long signature, int role)
{
  QString text = d_->decode(data);
  if (text.isEmpty())
    return data;

  if (!role)
    role = d_->settings.textRoleOf(signature);

  auto p = EmbedManager::instance();

  bool canceled = !d_->settings.enabled
      || WinKey::isKeyControlPressed() //d_->settings.detectsControl &&
      || WinKey::isKeyShiftPressed();

  //EmbedManagerLock lock(p);

  qint64 hash = canceled ? 0 : Engine::hashByteArray(data);
  if (!canceled && !d_->settings.translationEnabled[role] &&
      (d_->settings.extractionEnabled[role] || d_->settings.extractsAllTexts)) {
    enum { NeedsTranslation = false };
    if (d_->model->textFilterFunction) {
      QString t = d_->model->textFilterFunction(text, role);
      if (!t.isEmpty())
        p->sendText(t, hash, signature, role, NeedsTranslation);
    } else
      p->sendText(text, hash, signature, role, NeedsTranslation);
  }

  if (!d_->settings.textVisible[role])
    return QByteArray();

  if (!d_->settings.translationEnabled[role])
    return d_->settings.transcodingEnabled[role] ? d_->encode(text) : data;
  if (canceled ||
      role == Engine::OtherRole && !Util::needsTranslation(text))
    return d_->encode(text);

  QString repl = p->findTranslation(hash, role);
  bool needsTranslation = repl.isEmpty();
  if (d_->model->textFilterFunction) {
    QString t = d_->model->textFilterFunction(text, role);
    if (!t.isEmpty())
      p->sendText(t, hash, signature, role, needsTranslation);
    else
      return data;
  } else
    p->sendText(text, hash, signature, role, needsTranslation);

  if (needsTranslation) {
    repl = p->waitForTranslation(hash, role);
    if (!repl.isEmpty() && d_->model->translationFilterFunction)
      repl = d_->model->translationFilterFunction(repl, role);
  }

  if (repl.isEmpty())
    repl = text;
  else if (repl != text)
    switch (role) {
    case Engine::ScenarioRole:
      if (d_->settings.scenarioTextVisible)
        repl = QString("%1\n%2").arg(repl, text);
      break;
    case Engine::NameRole:
      if (d_->settings.nameTextVisible)
        repl = QString("%1 / %2").arg(repl, text);
      break;
    case Engine::OtherRole:
      if (d_->settings.otherTextVisible)
        repl = QString("%1 / %2").arg(repl, text);
      break;
    }

  return d_->encode(repl);
}

QString EngineController::dispatchTextW(const QString &text, long signature, int role)
{
  if (text.isEmpty())
    return text;
  if (!role)
    role = d_->settings.textRoleOf(signature);

  auto p = EmbedManager::instance();

  bool canceled = !d_->settings.enabled
      || WinKey::isKeyControlPressed() //d_->settings.detectsControl &&
      || WinKey::isKeyShiftPressed();

  // FIXME: This will serialize send operation. Use queued/cached shared memory instead
  //EmbedManagerLock lock(p);

  qint64 hash = canceled ? 0 : Engine::hashWString(text);
  if (!canceled && !d_->settings.translationEnabled[role] &&
      (d_->settings.extractionEnabled[role] || d_->settings.extractsAllTexts)) {
    enum { NeedsTranslation = false };
    if (d_->model->textFilterFunction) {
      QString t = d_->model->textFilterFunction(text, role);
      if (!t.isEmpty())
        p->sendText(t, hash, signature, role, NeedsTranslation);
    } else
      p->sendText(text, hash, signature, role, NeedsTranslation);
  }

  if (!d_->settings.textVisible[role])
    return QString();

  if (!d_->settings.translationEnabled[role])
    return text;
    //return d_->settings.transcodingEnabled[role] ? d_->encode(data) : data;
  if (canceled ||
      role == Engine::OtherRole && !Util::needsTranslation(text))
    return d_->postProcessW(text);
    //return d_->encode(data);

  QString repl = p->findTranslation(hash, role);
  bool needsTranslation = repl.isEmpty();
  if (d_->model->textFilterFunction) {
    QString t = d_->model->textFilterFunction(text, role);
    if (!t.isEmpty())
      p->sendText(t, hash, signature, role, needsTranslation);
    else
      return d_->postProcessW(text);
  } else
    p->sendText(text, hash, signature, role, needsTranslation);

  if (needsTranslation) {
    repl = p->waitForTranslation(hash, role);
    if (!repl.isEmpty() && d_->model->translationFilterFunction)
      repl = d_->model->translationFilterFunction(repl, role);
  }

  if (repl.isEmpty())
    repl = text; // prevent from deleting text
  else if (repl != text)
    switch (role) {
    case Engine::ScenarioRole:
      if (d_->settings.scenarioTextVisible)
        repl = QString("%1\n%2").arg(repl, text);
      break;
    case Engine::NameRole:
      if (d_->settings.nameTextVisible)
        repl = QString("%1 / %2").arg(repl, text);
      break;
    case Engine::OtherRole:
      if (d_->settings.otherTextVisible)
        repl = QString("%1 / %2").arg(repl, text);
      break;
    }

  //repl = QString::fromWCharArray(L"\u76ee\u899a");
  return d_->postProcessW(repl); // post-process
  //return d_->encode(repl);
}

// EOF

/*
// - Exchange -

// Qt is not allowed to appear in this function
const char *EngineController::exchangeTextA(const char *data, long signature, int role)
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

void EngineControllerPrivate::exchange()
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



typedef int (WINAPI *MultiByteToWideCharFun)(UINT CodePage, DWORD dwFlags, LPCSTR lpMultiByteStr, int cbMultiByte, LPWSTR lpWideCharStr, int cchWideChar);
typedef int (WINAPI *WideCharToMultiByteFun)(UINT CodePage, DWORD dwFlags, LPCWSTR lpWideCharStr, int cchWideChar, LPSTR lpMultiByteStr, int cbMultiByte, LPCSTR lpDefaultChar, LPBOOL lpUsedDefaultChar);
MultiByteToWideCharFun oldMultiByteToWideChar;
WideCharToMultiByteFun oldWideCharToMultiByte;
int WINAPI newMultiByteToWideChar(UINT CodePage, DWORD dwFlags, LPCSTR lpMultiByteStr, int cbMultiByte, LPWSTR lpWideCharStr, int cchWideChar)
{
  if (CodePage == 932)
    CodePage = 936;
  return ::oldMultiByteToWideChar(CodePage, dwFlags, lpMultiByteStr, cbMultiByte, lpWideCharStr, cchWideChar);
}

int WINAPI newWideCharToMultiByte(UINT CodePage, DWORD dwFlags, LPCWSTR lpWideCharStr, int cchWideChar, LPSTR lpMultiByteStr, int cbMultiByte, LPCSTR lpDefaultChar, LPBOOL lpUsedDefaultChar)
{
  if (CodePage == 932)
    CodePage = 936;
  return ::oldWideCharToMultiByte(CodePage, dwFlags, lpWideCharStr, cchWideChar, lpMultiByteStr, cbMultiByte, lpDefaultChar, lpUsedDefaultChar);
}
    ::oldMultiByteToWideChar = Engine::replaceFunction<MultiByteToWideCharFun>(addr, ::newMultiByteToWideChar);
    ::oldWideCharToMultiByte = Engine::replaceFunction<WideCharToMultiByteFun>(addr, ::newWideCharToMultiByte);
*/
