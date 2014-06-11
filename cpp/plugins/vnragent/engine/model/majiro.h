#pragma once

// majiro.h
// 4/20/2014 jichi

#include "engine/enginemodel.h"

struct MajiroEngine : EngineModel
{
  MajiroEngine()
    : name("Majiro")
    , matchFiles(QStringList() << "data*.arc" << "stream*.arc")
    , searchFunction(search)
    , hookFunction(hook)
  {}

private:
  static ulong search(ulong startAddress, ulong stopAddress);
  static void hook(HookStack *stack);
};

// EOF
