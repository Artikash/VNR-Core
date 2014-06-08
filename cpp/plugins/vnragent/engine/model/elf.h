#pragma once

// elf.h
// 5/31/2014 jichi

#include "engine/engine.h"
#include "util/codepage.h"

class ElfEngine : public AbstractEngine
{
  SK_EXTEND_CLASS(ElfEngine, AbstractEngine)
  SK_DISABLE_COPY(ElfEngine)

public:
  ElfEngine()
    : Base("Elf", Util::SjisCodePage, BlockingAttribute|SpecialHookAttribute)
  {} // Need restore the old text to be freed using FreeHeap

  static bool match();
protected:
  bool attach() override;
};

// EOF
