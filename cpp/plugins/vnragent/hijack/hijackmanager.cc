// hijackmanager.cc
// 6/3/2015 jichi
#include "hijack/hijackmanager.h"
#include "hijack/hijackfuns.h"
#include "winhook/hookfun.h"
#include <windows.h>
#include <unordered_map>

#define DEBUG "hijackmanager"
#include "sakurakit/skdebug.h"

/** Private data */

class HijackManagerPrivate
{
public:
  struct FunctionInfo {
    const char *name; // for debugging purpose
    ulong *oldFunction,
          newFunction;
    bool attached;

    explicit FunctionInfo(const char *name = "", ulong *oldFunction = nullptr, ulong newFunction = 0, bool attached = false)
      : name(name), oldFunction(oldFunction), newFunction(newFunction), attached(attached)
    {}
  };
  std::unordered_map<ulong, FunctionInfo> funs; // attached functions


  HijackManagerPrivate();
};

HijackManagerPrivate::HijackManagerPrivate()
{
#define ADD_FUN(_f) funs[(ulong)::_f] = FunctionInfo(#_f, (ulong *)&Hijack::old##_f, (ulong)Hijack::new##_f);
  ADD_FUN(GetGlyphOutlineA)
  ADD_FUN(GetTextExtentPoint32A)
#undef ADD_FUN
}

/** Public data */

HijackManager *HijackManager::instance() { static Self g; return &g; }

HijackManager::HijackManager() : d_(new D) {}
HijackManager::~HijackManager() { delete d_; }

bool HijackManager::isFunctionAttached(ulong addr) const
{
  auto p = d_->funs.find(addr);
  if (p == d_->funs.end())
    return false;
  return p->second.attached;
}

void HijackManager::attachFunction(ulong addr)
{
  auto p = d_->funs.find(addr);
  if (p == d_->funs.end())
    return;
  auto &info = p->second;
  if (info.attached)
    return;
  DOUT(info.name);
  info.attached = true;
  *info.oldFunction = winhook::replace_fun(addr, info.newFunction);
}

void HijackManager::detachFunction(ulong addr)
{
  auto p = d_->funs.find(addr);
  if (p == d_->funs.end())
    return;
  auto &info = p->second;
  if (!info.attached)
    return;
  info.attached = false;
  *info.oldFunction = addr;
  winhook::restore_fun(addr);
}

// EOF
