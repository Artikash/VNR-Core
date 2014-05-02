// uidriver_p.cc
// 2/1/2013 jichi

#include "ui/uidriver_p.h"
#include "ui/uihash.h"
#include "ui/uimanager.h"
#include "winiter/winiter.h"
#include "winiter/winitertl.h"
#include <commctrl.h>
#include <QtCore/QHash>
#include <QtCore/QTextCodec>
#include <QtCore/QTimer>
//#include <boost/bind.hpp>

enum { TEXT_BUFFER_SIZE = 256 };

#define NTLEA_COMPAT // Fix the text of NTLEA

// - Construction -

static UiDriverPrivate *instance_;
UiDriverPrivate *UiDriverPrivate::instance() { return instance_; }

UiDriverPrivate::UiDriverPrivate(QObject *parent)
  : Base(parent), enabled(true) // enable by default
{
  manager = new UiManager(this);

  refreshTimer = new QTimer(this);
  refreshTimer->setSingleShot(false);
  refreshTimer->setInterval(refreshInterval);
  connect(refreshTimer, SIGNAL(timeout()), SLOT(refresh()));

  connect(this, SIGNAL(updateContextMenuRequested(void*,void*)), SLOT(onUpdateContextMenuRequested(void*,void*)),
          Qt::QueuedConnection);

  ::instance_ = this;

  refreshTimer->start();
}

UiDriverPrivate::~UiDriverPrivate() { ::instance_ = nullptr; }

// - Processes and threads -

void UiDriverPrivate::updateThreadWindows(DWORD threadId)
{
  WinIter::iterThreadChildWindows(threadId, [](HWND hWnd) {
    ::instance_->updateAbstractWindow(hWnd);
  });
  //WinIter::iterThreadChildWindows(threadId,
  //  boost::bind(&Self::updateAbstractWindow, ::instance_, _1));
}

void UiDriverPrivate::updateProcessWindows(DWORD processId)
{ WinIter::iterProcessThreadIds(processId, updateThreadWindows); }

// - Windows -

void UiDriverPrivate::updateAbstractWindow(HWND hWnd)
{
  wchar_t buf[TEXT_BUFFER_SIZE];

  if (::RealGetWindowClassW(hWnd, buf, TEXT_BUFFER_SIZE)) {
    if (!::wcscmp(buf, L"SysTabControl32")) {
      updateTabControl(hWnd, buf, TEXT_BUFFER_SIZE);
      return;
    }
    if (!::wcscmp(buf, L"SysListView32")) {
      updateListView(hWnd, buf, TEXT_BUFFER_SIZE);
      return;
    }
  }

  if (updateStandardWindow(hWnd, buf, TEXT_BUFFER_SIZE))
    repaintWindow(hWnd);

  if (HMENU hMenu = ::GetMenu(hWnd))
    if (updateMenu(hMenu, hWnd, buf, TEXT_BUFFER_SIZE))
      repaintMenuBar(hWnd);
}

void UiDriverPrivate::updateContextMenu(HMENU hMenu, HWND hWnd)
{
  wchar_t buf[TEXT_BUFFER_SIZE];
  updateMenu(hMenu, hWnd, buf, TEXT_BUFFER_SIZE);
}

void UiDriverPrivate::repaintWindow(HWND hWnd) { ::InvalidateRect(hWnd, nullptr, TRUE); }
void UiDriverPrivate::repaintMenuBar(HWND hWnd) { ::DrawMenuBar(hWnd); }

bool UiDriverPrivate::updateStandardWindow(HWND hWnd, LPWSTR buffer, int bufferSize)
{
  qint64 h = 0;
  int sz = ::GetWindowTextW(hWnd, buffer, bufferSize);
  if (sz)
    h = Ui::hashWCharArray(buffer, sz);

  long anchor = Ui::hashWindow(hWnd);
  const UiManager::TextEntry &e = manager->findTextEntryAtAnchor(anchor);
  QString trans;
  if (e.hash)
    trans = manager->findTextTranslation(e.hash);

  if (h && trans.isEmpty())
    if (!manager->containsTranslation(h)) {
#ifdef NTLEA_COMPAT // NTLEA could mess up the length of the string orz
      QString t = QString::fromWCharArray(buffer);
#else
      QString t = QString::fromWCharArray(buffer, sz);
#endif // NTLEA_COMPAT
      if (!Ui::isTranslatedText(t)) {
        manager->updateText(t, h, anchor);
        trans = manager->findTextTranslation(h);
      }
    }

  if (!enabled)
    trans = e.text;

  // DefWindowProcW is used instead of SetWindowTextW
  // https://groups.google.com/forum/?fromgroups#!topic/microsoft.public.platformsdk.mslayerforunicode/nWi3dlZeS60
  if (!trans.isEmpty() && h != Ui::hashString(trans)) {
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

bool UiDriverPrivate::updateMenu(HMENU hMenu, HWND hWnd, LPWSTR buffer, int bufferSize)
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
        h = Ui::hashWCharArray(info.dwTypeData, info.cch);

      QString trans;
      long anchor = Ui::hashWindowItem(hWnd, Ui::MenuTextRole, info.wID + (i<<4));
      const UiManager::TextEntry &e = manager->findTextEntryAtAnchor(anchor);
      if (e.hash)
        trans = manager->findTextTranslation(e.hash);

      if (h && trans.isEmpty())
        if (!manager->containsTranslation(h)) {
#ifdef NTLEA_COMPAT // NTLEA could mess up the length of the string orz
          QString t = QString::fromWCharArray(buffer);
#else
          QString t = QString::fromWCharArray(buffer, info.cch);
#endif // NTLEA_COMPAT
          if (!Ui::isTranslatedText(t)) {
            manager->updateText(t, h, anchor);
            trans = manager->findTextTranslation(h);
          }
        }

      if (!enabled)
        trans = e.text;

      if (!trans.isEmpty() && h != Ui::hashString(trans)) {
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

bool UiDriverPrivate::updateTabControl(HWND hWnd, LPWSTR buffer, int bufferSize)
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
    qint64 h = Ui::hashString(t);

    QString trans;
    long anchor = Ui::hashWindowItem(hWnd, Ui::TabTextRole, i);
    const UiManager::TextEntry &e = manager->findTextEntryAtAnchor(anchor);
    if (e.hash)
      trans = manager->findTextTranslation(e.hash);

    if (h && trans.isEmpty())
      if (!manager->containsTranslation(h) && !Ui::isTranslatedText(t)) {
        manager->updateText(t, h, anchor);
        trans = manager->findTextTranslation(h);
      }

    if (!enabled)
      trans = e.text;

    if (!trans.isEmpty() && h != Ui::hashString(trans)) {
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

bool UiDriverPrivate::updateListView(HWND hWnd, LPWSTR buffer, int bufferSize)
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
    qint64 h = Ui::hashString(t);

    QString trans;
    long anchor = Ui::hashWindowItem(hWnd, Ui::ListTextRole, i);
    const UiManager::TextEntry &e = manager->findTextEntryAtAnchor(anchor);
    if (e.hash)
      trans = manager->findTextTranslation(e.hash);

    if (h && trans.isEmpty())
      if (!manager->containsTranslation(h) && !Ui::isTranslatedText(t)) {
        manager->updateText(t, h, anchor);
        trans = manager->findTextTranslation(h);
      }

    if (!enabled)
      trans = e.text;

    if (!trans.isEmpty() && h != Ui::hashString(trans)) {
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

// - Helpers -

void UiDriverPrivate::updateWindows()
{
  QHash<HWND, uint> w;
  foreach (const UiManager::Entry &e, manager->entries())
    w[e.window] |= e.role;

  enum { BUFFER_SIZE = 256 };
  wchar_t buf[BUFFER_SIZE];

  for (auto it = w.begin(); it != w.end(); ++it) {
    HWND h = it.key();
    if (::IsWindow(h)) {
      uint roles = it.value();
      if (roles & Ui::WindowTextRole)
        updateWindow(h, buf, BUFFER_SIZE);
      if (roles & Ui::MenuTextRole)
        if (HMENU hMenu = ::GetMenu(h)) {
          updateMenu(hMenu, h, buf, BUFFER_SIZE);
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
