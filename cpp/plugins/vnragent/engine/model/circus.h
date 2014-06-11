#pragma once

// circus.h
// 6/4/2014 jichi

#include "engine/engine.h"
#include "util/codepage.h"

// Currently, only CIRCUS Type#2 is implemented
class CircusEngine : public AbstractEngine
{
  SK_EXTEND_CLASS(CircusEngine, AbstractEngine)
  SK_DISABLE_COPY(CircusEngine)

  static void hookFunction(HookStack *stack);
  static QString textFilter(const QString &text, int role); // remove "\n"
  static QString translationFilter(const QString &text, int role); // insert "\n"
public:
  static bool match() { return matchFiles("advdata/grp/names.dat"); }

  CircusEngine()
  {
    setName("CIRCUS");
    setWideChar(false);
    //setMatchFiles("advdata/grp/names.dat");
    //setReadAsm("[esp+8]");  // arg2
    //setSplitAsm("[esp]"); // return address

    setHookFunction(hookFunction);
    setTextFilter(textFilter);
    setTranslationFilter(translationFilter);
  }

protected:
  bool attach() override;
  //bool detach() override;
};

// EOF
