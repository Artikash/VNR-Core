// winhook.cc
// 5/25/2015 jichi
#include "winhook/winhook.h"

// Hook manager

namespace { // unnamed

class HookManager
{
public:
};

HookManager *hookManager;
HookManager *createHookManager() { return new HookManager; }

} // unnamed namespace

WINHOOK_BEGIN_NAMESPACE

bool hook(ulong address, hook_fun_t callback)
{
  if (!::hookManager)
    ::hookManager = ::createHookManager();
  return true;
}

bool unhook(ulong address)
{
  if (!::hookManager)
    return true;
  return false;
}

WINHOOK_END_NAMESPACE

// EOF
