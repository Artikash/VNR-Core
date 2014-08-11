#pragma once

// wincomptr.h
// 8/11/2014 jichi
// Smart pointers for COM.

#include "wincom/wincom.h"
#include <unknown.h> // IUnknown

WINCOM_BEGIN_NAMESPACE

class ScopedUnknownPtr
{
  IUnknown *p;
public:
  ScopedUnknownPtr(IUnknown *p) : p(p) {}
  ~ScopedUnknownPtr() { if (p) p->Release(); }
};

WINCOM_END_NAMESPACE
