// mono.cc
// 7/27/2015 jichi
// https://github.com/mono/mono/blob/master/mono/metadata/object.h
#include "engine/model/mono.h"
#include "engine/enginecontroller.h"
#include "engine/enginedef.h"
#include "engine/enginehash.h"
#include "engine/engineutil.h"
#include "util/textutil.h"
//#include "hijack/hijackmanager.h"
#include "cpputil/cppcstring.h"
#include "memdbg/memsearch.h"
#include "mono/monoobject.h"
#include "mono/monotype.h"
#include "winhook/hookfun.h"
#include "winasm/winasmdef.h"
//#include "ntdll/ntdll.h"
#include <qt_windows.h>
#include <cstdint>

#define DEBUG "model/mono"
#include "sakurakit/skdebug.h"

//#pragma intrinsic(_ReturnAddress)

namespace { // unnamed

mono_object_get_domain_fun_t mono_object_get_domain;
mono_string_new_utf16_fun_t mono_string_new_utf16;

namespace MonoHook {
namespace Private {

  //void *allocate(size_t size) { return new uint8_t[size]; }
  //void deallocate(void *p) { delete[] (uint8_t *)p; }

  //void MonoStringFree(MonoString *p) { deallocate(p); }

  //MonoString *MonoStringFromQString(const QString &s)
  //{
  //  size_t size = sizeof(MonoString) + sizeof(wchar_t) * (s.size() - 1);
  //  MonoString *ret = (MonoString *)allocate(size);
  //  ret->length = s.size();
  //  s.toWCharArray(ret->chars);
  //  return ret;
  //}

  mono_string_to_utf8_fun_t old_mono_string_to_utf8;

  bool skipsText(const wchar_t *s, size_t size)
  { return Util::allAscii(s, size) || ::cpp_wcsnchr(s, size, '/'); }

  char *new_mono_string_to_utf8(MonoString *s)
  {
    if (s && !skipsText(s->chars, s->length)) {
      enum { role = Engine::OtherRole, sig = Engine::OtherThreadSignature };
      //auto split = (ulong)_ReturnAddress();
      //auto sig = Engine::hashThreadSignature(role, split);

      QString oldText = QString::fromUtf16(s->chars, s->length),
              newText = EngineController::instance()->dispatchTextW(oldText, role, sig);
      if (!newText.isEmpty() && newText != oldText) {
        MonoDomain *domain = mono_object_get_domain(s);
        MonoString *t = mono_string_new_utf16(domain, newText.utf16(), newText.size());
        return old_mono_string_to_utf8(t);
      }
    }
    return old_mono_string_to_utf8(s);
  }

} // namespace Private

bool attach()
{
  ulong fun = Engine::getModuleFunction("mono.dll", "mono_string_to_utf8");
  return fun && (Private::old_mono_string_to_utf8 = (mono_string_to_utf8_fun_t)winhook::replace_fun(fun, (ulong)Private::new_mono_string_to_utf8));
}

} // namespace MonoHook
} // unnamed namespace

/** Public class */

bool MonoEngine::match() { return ::GetModuleHandleA("mono.dll"); }

bool MonoEngine::attach()
{
  mono_object_get_domain = (mono_object_get_domain_fun_t)Engine::getModuleFunction("mono.dll", "mono_object_get_domain");
  mono_string_new_utf16 = (mono_string_new_utf16_fun_t)Engine::getModuleFunction("mono.dll", "mono_string_new_utf16");

  return MonoHook::attach();
}

// EOF
