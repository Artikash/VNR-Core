#pragma once

// elf.h
// 5/31/2014 jichi

#include "engine/enginemodel.h"

struct ElfEngine : EngineModel
{
  ElfEngine() //: Base("Elf", Util::SjisCodePage, BlockingAttribute|SpecialHookAttribute) // Need restore the old text to be freed using FreeHeap
    : name("Elf")
    // "Silkys.exe" or AI6WIN.exe might not exist
    // All arc files in 愛姉妹4 are: data, effect, layer, mes, music
    // mes.arc is the scenario
    , matchFiles(QStringList() << "data.arc" << "effect.arc" << "mes.arc")
    , attachFunction(attach)
  {}

private:
  static bool attach();
  //static ulong search(ulong startAddress, ulong stopAddress);
};

// EOF
