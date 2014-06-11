#pragma once

// circus.h
// 6/4/2014 jichi

#include "engine/enginemodel.h"

// Currently, only CIRCUS Type#2 is implemented
struct CircusEngine : EngineModel
{
  CircusEngine()
    : name("CIRCUS")
    , matchFiles("advdata/grp/names.dat")
    //, textAsm("[esp+8]")  // arg2
    //, sizeAsm(nullptr)  // none
    //, splitAsm("[esp]") // return address
    , searchFunction(search)
    , hookFunction(hook)
    , textFilterFunction(textFilter)
    , translationFilterFunction(translationFilter)
  {}

private:
  static ulong search(ulong startAddress, ulong stopAddress);
  static void hook(HookStack *stack);
  static QString textFilter(const QString &text, int role); // remove "\n"
  static QString translationFilter(const QString &text, int role); // insert "\n"
};

// EOF
