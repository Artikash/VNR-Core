#pragma once

// enginemodel.h
// 6/10/2014 jichi

#include "sakurakit/skglobal.h"
#include <QtCore/QStringList>

class EngineModel
{
  SK_CLASS(EngineModel)
public:
  enum Encoding {
    AsciiEncoding = 0,
    Utf16Encoding,
    Utf8Encoding,
  };

  EngineModel()
    : name(nullptr)
    , encoding(AsciiEncoding)   // scenario thread encoding

    , enableDynamicEncoding(false) // whether use dynamic codec to fix ascii text
    , enableLocaleEmulation(false) // fix inconsistent game locale
    , enableGDIFont(false)      // change GDI device context font

    , matchFunction(nullptr)    // determine whether apply engine
    , attachFunction(nullptr)   // apply the engine
    , detachFunction(nullptr)   // remove the applied engine
    , textFilterFunction(nullptr)   // fix scenario text before sending out
    , translationFilterFunction(nullptr)    // fix translation text before sending out
  {}

  const char *name;
  Encoding encoding;
  bool enableDynamicEncoding;
  bool enableLocaleEmulation;
  bool enableGDIFont;

  QStringList matchFiles; // files existing in the game directory

  // Global match functions
  typedef bool (* match_function)();
  match_function matchFunction, // override match files
                 attachFunction, // override searchFunction and hookFunction
                 detachFunction; // not used

  // Filter functions
  typedef QString (* filter_function)(const QString &text, int role);
  filter_function textFilterFunction,        ///< modify game text
                  translationFilterFunction; ///< modify translation
};

  // Memory search function
  //typedef ulong (* search_function)(ulong startAddress, ulong stopAddress);
  //search_function searchFunction; ///< return the hook address

  //// Hijacked function
  //struct HookStack
  //{
  //  ulong eflags;  // pushaf
  //  ulong edi,     // pushad
  //        esi,
  //        ebp,
  //        esp,
  //        ebx,
  //        edx,
  //        ecx,     // this
  //        eax;     // 0x24
  //  ulong retaddr; // 0x2c, &retaddr == esp
  //  ulong args[1]; // 0x2e
  //};
  //typedef void (* hook_function)(HookStack *);
  //hook_function hookFunction; ///< callback of hooked function

// EOF
