// mainobj.cc
// 2/1/2013 jichi

#include "main.h"
#include "driver/mainobj.h"
#include "driver/mainobj_p.h"
#include "driver/rpccli.h"
#include "ui/uihash.h"
#include "ui/uitextman.h"
#include "qtembedded/applicationloader.h"
//#include "wintimer/wintimer.h"
#include <Commctrl.h>
#include <TlHelp32.h>
#include <QtCore/QHash>
#include <QtCore/QTextCodec>

enum { TEXT_BUFFER_SIZE = 256 };

#define NTLEA_COMPAT

/** Public class */

// - Initialization -

namespace { namespace detail { // unnamed
QtEmbedded::ApplicationLoader *loader;
MainObject *instance;

// Because boost::bind does not support __stdcall.

inline void RepaintWindow(HWND hWnd)
{ ::InvalidateRect(hWnd, nullptr, TRUE); }

inline void RepaintMenuBar(HWND hWnd)
{ ::DrawMenuBar(hWnd); }

inline bool IsTranslated(const QString &t)
{ return t.contains('<') && !t.contains('>'); }

}} // unnamed detail

MainObject *MainObject::instance() { return detail::instance; }
void MainObject::init()
{
  QTextCodec *codec = QTextCodec::codecForName("UTF-8");
  QTextCodec::setCodecForCStrings(codec);
  //QTextCodec::setCodecForTr(codec);

  //detail::loader = QtEmbedded::ApplicationLoader::createInstance();
  QCoreApplication *app = QtEmbedded::ApplicationLoader::createApplication();
  detail::loader = new QtEmbedded::ApplicationLoader(app, Main::EventLoopInterval);
  detail::instance = new Self();
}

void MainObject::destroy()
{
  if (detail::instance) {
    delete detail::instance;
    detail::instance = nullptr;
  }

  if (detail::loader) {
    detail::loader->quit();
    delete detail::loader;
    detail::loader = nullptr;
  }
}

// - Construction -

MainObject::MainObject(QObject *parent) : Base(parent), d_(new D(this)) {}
MainObject::~MainObject() { delete d_; }

// - Process and Thread -

namespace { // unnamed
BOOL CALLBACK EnumThreadWndProc(HWND hWnd, LPARAM lParam)
{
  //if (!NotSupportedWindow(hWnd))
  //  return TRUE;
  if (MainObject::instance()) {
    MainObject::instance()->updateWindow(hWnd);
    ::EnumChildWindows(hWnd, (WNDENUMPROC)EnumThreadWndProc, lParam);
  }
  return TRUE;
}
} // unnamed namespace

void MainObject::updateThreadWindows(DWORD threadId)
{
  if (!threadId)
    threadId = ::GetCurrentThreadId();
  ::EnumThreadWindows(threadId, (WNDENUMPROC)EnumThreadWndProc, 0);
}

void MainObject::updateProcessWindows(DWORD processId)
 {
  HANDLE h = ::CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, processId);
  if (h == INVALID_HANDLE_VALUE)
    return;
  if (!processId)
    processId = ::GetCurrentProcessId();
  THREADENTRY32 thread = {0};
  thread.dwSize = sizeof(thread);
  if (::Thread32First(h, &thread))
    do if (thread.th32OwnerProcessID == processId)
      updateThreadWindows(thread.th32ThreadID);
    while (::Thread32Next(h, &thread));
  ::CloseHandle(h);
}

// - Windows -

void MainObject::updateWindow(HWND hWnd)
{
  wchar_t buf[TEXT_BUFFER_SIZE];

  if (::RealGetWindowClassW(hWnd, buf, TEXT_BUFFER_SIZE)) {
    if (!::wcscmp(buf, L"SysTabControl32")) {
      d_->updateTabControl(hWnd, buf, TEXT_BUFFER_SIZE);
      return;
    }
    if (!::wcscmp(buf, L"SysListView32")) {
      d_->updateListView(hWnd, buf, TEXT_BUFFER_SIZE);
      return;
    }
  }

  if (d_->updateWindow(hWnd, buf, TEXT_BUFFER_SIZE)) {
    // No effect?
    //WinTimer::singleShot(200, boost::bind(
    //    &detail::RepaintWindow, hWnd));
  }

  if (HMENU hMenu = ::GetMenu(hWnd))
    if (d_->updateMenu(hMenu, hWnd, buf, TEXT_BUFFER_SIZE)) {
      // No effect?!
      //WinTimer::singleShot(200, boost::bind(
      //    &detail::RepaintMenuBar, hWnd));
    }
}

void MainObject::updateContextMenu(HMENU hMenu, HWND hWnd)
{
  wchar_t buf[TEXT_BUFFER_SIZE];
  d_->updateMenu(hMenu, hWnd, buf, TEXT_BUFFER_SIZE);
}

/** Private class */

// - Construction -

MainObjectPrivate::MainObjectPrivate(Q *q)
  : Base(q), q_(q), enabled(true)
{
  dm = new DataManager(q);
  rpc = new RpcClient(q);

  connect(dm, SIGNAL(textsChanged(QString)), rpc, SLOT(sendData(QString)));
  //connect(dm, SIGNAL(translationChanged()), q, SLOT(updateWindows()));
  connect(rpc, SIGNAL(dataReceived(QString)), dm, SLOT(updateTranslation(QString)));
  connect(rpc, SIGNAL(clearRequested()), dm, SLOT(clearTranslation()));
  connect(rpc, SIGNAL(enableRequested()), SLOT(enable()));
  connect(rpc, SIGNAL(disableRequested()), SLOT(disable()));
  //connect(this, SIGNAL(enabledChanged(bool)), q, SLOT(updateWindows()));
};

// - Windows -

bool MainObjectPrivate::updateWindow(HWND hWnd, LPWSTR buffer, int bufferSize)
{
  qint64 h = 0;
  int sz = ::GetWindowTextW(hWnd, buffer, bufferSize);
  if (sz)
    h = My::hashWCharArray(buffer, sz);

  long anchor = My::hashWindow(hWnd);
  const DataManager::TextEntry &e = dm->findTextEntryAtAnchor(anchor);
  QString trans;
  if (e.hash)
    trans = dm->findTextTranslation(e.hash);

  if (h && trans.isEmpty())
    if (!dm->containsTranslation(h)) {
#ifdef NTLEA_COMPAT // NTLEA could mess up the length of the string orz
      QString t = QString::fromWCharArray(buffer);
#else
      QString t = QString::fromWCharArray(buffer, sz);
#endif // NTLEA_COMPAT
      if (!detail::IsTranslated(t)) {
        dm->updateText(t, h, anchor);
        trans = dm->findTextTranslation(h);
      }
    }

  if (!enabled)
    trans = e.text;

  // DefWindowProcW is used instead of SetWindowTextW
  // https://groups.google.com/forum/?fromgroups#!topic/microsoft.public.platformsdk.mslayerforunicode/nWi3dlZeS60
  if (!trans.isEmpty() && h != My::hashString(trans)) {
    if (trans.size() >= bufferSize)
      //::SetWindowTextW(hWnd, trans.toStdWString().c_str());
      ::DefWindowProcW(hWnd, WM_SETTEXT, 0, (LPARAM)trans.toStdWString().c_str());
    else {
      buffer[trans.size()] = 0;
      trans.toWCharArray(buffer);
      //::SetWindowTextW(hWnd, buffer);
      ::DefWindowProcW(hWnd, WM_SETTEXT, 0, (LPARAM)buffer);
    }

    return true;
  }
  return false;
}

bool MainObjectPrivate::updateMenu(HMENU hMenu, HWND hWnd, LPWSTR buffer, int bufferSize)
{
  if (!hMenu)
    return false;
  int count = ::GetMenuItemCount(hMenu);
  if (count <= 0)
    return false;

  bool ret = false;

  MENUITEMINFOW info = {0};
  info.cbSize = sizeof(info);
  for (int i = 0; i < count; i++) {
    info.fMask = MIIM_SUBMENU | MIIM_TYPE | MIIM_ID;
    info.cch = bufferSize;
    info.dwTypeData = buffer;
    if (::GetMenuItemInfoW(hMenu, i, TRUE, &info)) { // fByPosition: TRUE

      qint64 h = 0;
      if (info.cch)
        h = My::hashWCharArray(info.dwTypeData, info.cch);

      QString trans;
      long anchor = My::hashWindowItem(hWnd, My::MenuTextRole, info.wID + (i<<4));
      const DataManager::TextEntry &e = dm->findTextEntryAtAnchor(anchor);
      if (e.hash)
        trans = dm->findTextTranslation(e.hash);

      if (h && trans.isEmpty())
        if (!dm->containsTranslation(h)) {
#ifdef NTLEA_COMPAT // NTLEA could mess up the length of the string orz
          QString t = QString::fromWCharArray(buffer);
#else
          QString t = QString::fromWCharArray(buffer, info.cch);
#endif // NTLEA_COMPAT
          if (!detail::IsTranslated(t)) {
            dm->updateText(t, h, anchor);
            trans = dm->findTextTranslation(h);
          }
        }

      if (!enabled)
        trans = e.text;

      if (!trans.isEmpty() && h != My::hashString(trans)) {
        int sz = qMin(trans.size(), bufferSize);
        info.fMask = MIIM_STRING;
        info.dwTypeData = buffer;
        info.cch = sz;
        info.dwTypeData[sz] = 0;
        trans.toWCharArray(info.dwTypeData);
        ::SetMenuItemInfoW(hMenu, i, TRUE, &info);
        ret = true;
      }

      if (info.hSubMenu)
        ret = updateMenu(info.hSubMenu, hWnd, buffer, bufferSize) || ret;
    }
  }
  return ret;
}

bool MainObjectPrivate::updateTabControl(HWND hWnd, LPWSTR buffer, int bufferSize)
{
  bool ret = false;
  LRESULT count = ::SendMessageW(hWnd, TCM_GETITEMCOUNT, 0, 0);
  TCITEMW item = {0};
  for (int i = 0; i < count; i++) {
    item.mask = TCIF_TEXT;
    item.pszText = buffer;
    item.cchTextMax = bufferSize;
    if (!::SendMessageW(hWnd, TCM_GETITEM, i, (LPARAM)&item))
      break;
    QString t = QString::fromWCharArray(item.pszText);
    qint64 h = My::hashString(t);

    QString trans;
    long anchor = My::hashWindowItem(hWnd, My::TabTextRole, i);
    const DataManager::TextEntry &e = dm->findTextEntryAtAnchor(anchor);
    if (e.hash)
      trans = dm->findTextTranslation(e.hash);

    if (h && trans.isEmpty())
      if (!dm->containsTranslation(h) && !detail::IsTranslated(t)) {
        dm->updateText(t, h, anchor);
        trans = dm->findTextTranslation(h);
      }

    if (!enabled)
      trans = e.text;

    if (!trans.isEmpty() && h != My::hashString(trans)) {
      ret = true;
      if (trans.size() < bufferSize) {
        buffer[trans.size()] = 0;
        trans.toWCharArray(buffer);
        item.pszText = buffer;
        item.mask = TCIF_TEXT;
        ::SendMessageW(hWnd, TCM_SETITEM, i, (LPARAM)&item);
      } else {
        wchar_t *w = new wchar_t[trans.size() +1];
        w[trans.size()] = 0;
        trans.toWCharArray(w);
        item.pszText = w;
        item.mask = TCIF_TEXT;
        ::SendMessageW(hWnd, TCM_SETITEM, i, (LPARAM)&item);
        delete[] w;
      }
    }
  }
  return ret;
}

bool MainObjectPrivate::updateListView(HWND hWnd, LPWSTR buffer, int bufferSize)
{
  enum { MAX_COLUMN = 100 };
  bool ret = false;
  for (int i = 0; i < MAX_COLUMN; i++) {
    LVCOLUMNW column = {0};
    column.mask = LVCF_TEXT;
    column.pszText = buffer;
    column.cchTextMax = bufferSize;
    if (!ListView_GetColumn(hWnd, i, &column))
      break;

    QString t = QString::fromWCharArray(column.pszText);
    qint64 h = My::hashString(t);

    QString trans;
    long anchor = My::hashWindowItem(hWnd, My::ListTextRole, i);
    const DataManager::TextEntry &e = dm->findTextEntryAtAnchor(anchor);
    if (e.hash)
      trans = dm->findTextTranslation(e.hash);

    if (h && trans.isEmpty())
      if (!dm->containsTranslation(h) && !detail::IsTranslated(t)) {
        dm->updateText(t, h, anchor);
        trans = dm->findTextTranslation(h);
      }

    if (!enabled)
      trans = e.text;

    if (!trans.isEmpty() && h != My::hashString(trans)) {
      ret = true;
      if (trans.size() < bufferSize) {
        buffer[trans.size()] = 0;
        trans.toWCharArray(buffer);
        column.mask = TCIF_TEXT;
        column.pszText = buffer;
        ListView_SetColumn(hWnd, i, &column);
      } else {
        wchar_t *w = new wchar_t[trans.size() +1];
        w[trans.size()] = 0;
        trans.toWCharArray(w);
        column.mask = TCIF_TEXT;
        column.pszText = w;
        ListView_SetColumn(hWnd, i, &column);
        delete[] w;
      }
    }
  }
  return ret;
}

// EOF

/*
void MainObject::updateWindows()
{
  QHash<HWND, uint> w;
  foreach (const DataManager::Entry &e, d_->dm->entries())
    w[e.window] |= e.role;

  enum { BUFFER_SIZE = 256 };
  wchar_t buf[BUFFER_SIZE];

  for (auto it = w.begin(); it != w.end(); ++it) {
    HWND h = it.key();
    if (::IsWindow(h)) {
      uint roles = it.value();
      if (roles & My::WindowTextRole)
        d_->updateWindow(h, buf, BUFFER_SIZE);
      if (roles & My::MenuTextRole)
        if (HMENU hMenu = ::GetMenu(h)) {
          d_->updateMenu(hMenu, h, buf, BUFFER_SIZE);
          ::DrawMenuBar(h);
        }
}

  inline bool NotSupportedWindow(HWND hWnd)
  {
    enum { bufsz = 64 };
    char buf[bufsz];
    return !::RealGetWindowClassA(hWnd, buf, bufsz) ||
        ::strcmp(buf, "ComboBox") &&
        ::strcmp(buf, "ComboBoxEx32") &&
        ::strcmp(buf, "Edit") &&
        ::strcmp(buf, "ListBox") &&
        ::strcmp(buf, "SysLink") &&
        ::strcmp(buf, "SysTreeView32");
  }

*/
