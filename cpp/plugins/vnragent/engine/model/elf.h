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
  ElfEngine() //: Base("Elf", Util::SjisCodePage, BlockingAttribute|SpecialHookAttribute) // Need restore the old text to be freed using FreeHeap
  {
    setName("Elf");
    setWideChar(false);
  }

  // "Silkys.exe" or AI6WIN.exe might not exist
  // All arc files in 愛姉妹4 are: data, effect, layer, mes, music
  // mes.arc is the scenario
  static bool match() { return matchFiles(QStringList() << "data.arc" << "effect.arc" << "mes.arc"); }

protected:
  bool attach() override;
};

// EOF
