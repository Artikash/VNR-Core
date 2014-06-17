#pragma once

// circus.h
// 6/4/2014 jichi

#include "engine/enginemodel.h"

// Currently, only CIRCUS Type#2 is implemented
class CircusEngine : public EngineModel
{
  SK_EXTEND_CLASS(CircusEngine, EngineModel)
  static ulong search(ulong startAddress, ulong stopAddress);
  static void hook(HookStack *stack);
  static QString textFilter(const QString &text, int role); // remove "\n"
  static QString translationFilter(const QString &text, int role); // insert "\n" to wrap long lines
public:
  CircusEngine()
  {
    name = "CIRCUS";
    matchFiles << "advdata/grp/names.dat";
    // TODO: Unify the text/size/split asm
    //textAsm("[esp+8]")  // arg2
    //sizeAsm(nullptr)  // none
    //splitAsm("[esp]") // return address
    searchFunction = &Self::search;
    hookFunction = &Self::hook;
    textFilterFunction = &Self::textFilter;
    translationFilterFunction = &Self::translationFilter;
  }
};

// EOF
