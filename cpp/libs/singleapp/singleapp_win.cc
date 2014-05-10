// singleapp_win.cc
// 2/28/2013 jichi
// See: http://bcbjournal.org/articles/vol3/9911/Single-instance_applications.htm
#include "singleapp/singleapp.h"
#include <windows.h>

#ifdef _MSC_VER
# pragma warning (disable:4800)   // C4800: forcing BOOL to bool
#endif // _MSC_VER

//#define DEBUG "singleapp_win.cc"
//#include "sakurakit/skdebug.h"

namespace { // unnamed

class SingleApp
{
  HANDLE hMutex;           // mutex handle
  WCHAR wszName[MAX_PATH]; // mutex name

  // If lpName matches the name of an existing event, semaphore, waitable timer, job, or file-mapping object, the function fails and the GetLastError function returns ERROR_INVALID_HANDLE. This occurs because these objects share the same namespace.
  // So, replace ":" with "|" or whatever random char
  //
  // Also, according to MSDN, "\\" is illegal
  static void escapeMutexName(LPWSTR wsz)
  // Replace all '\\' in wsz with '_'
  { while (*wsz == L'\\' ? *wsz++ = L'_' : *wsz++); }

public:
  SingleApp() : hMutex(nullptr)
  {
    if (::GetModuleFileNameW(nullptr, wszName, MAX_PATH)) {
      escapeMutexName(wszName);
      if (HANDLE h = ::OpenMutexW(MUTEX_ALL_ACCESS, FALSE, wszName))
        ::CloseHandle(h);
      else
        hMutex = ::CreateMutexW(nullptr, FALSE, wszName);
    }
  }

  ~SingleApp()
  {
    if (hMutex)
      ::ReleaseMutex(hMutex);
  }

  bool ownMutex() const { return hMutex; }
} a;

} // unnamed namespace

SINGLEAPP_BEGIN_NAMESPACE

bool singleapp() { return a.ownMutex(); }

SINGLEAPP_END_NAMESPACE

// EOF
