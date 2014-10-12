#pragma once

// aiaudioapi.h
// 10/12/2014 jichi
#include "aitalk/aiaudiodef.h"
#include "aitalk/_windef.h" // for HMODULE
#include <sal.h> // for _Out_

namespace AITalk
{

/* Exported DLL functions of aitalked.dll */

// Initialization

// AITalkAudio.cs:
// [DllImport("aitalked.dll", EntryPoint="AIAudioAPI_Close")]
// private static extern AIAudioResultCode _Close();
#define _AIAudioAPI_Close "_AIAudioAPI_Close@0"
typedef AIAudioResultCode (__stdcall *AIAudioAPI_Close)();

// AITalkAudio.cs:
// [DllImport("aitalked.dll", EntryPoint="AIAudioAPI_Open")]
// private static extern AIAudioResultCode _Open(ref AIAudio_TConfig config);
#define _AIAudioAPI_Open "_AIAudioAPI_Open@4"
typedef AIAudioResultCode (__stdcall *AIAudioAPI_Open)(const AIAudio_TConfig *config);

/* Manager class */

class AIAudioAPI
{
public:
  AIAudioAPI_Open Open;
  AIAudioAPI_Close Close;

  /**
   * @param  h  aitalked.dll module
   *
   * Initialize function pointers in the DLL module.
   */
  explicit AIAudioAPI(HMODULE h);

  ///  Do nothing.
  ~AIAudioAPI();

  ///  Return if there are null methods.
  bool IsValid() const
  { return Open && Close; }
};

} // namespace AITalk
