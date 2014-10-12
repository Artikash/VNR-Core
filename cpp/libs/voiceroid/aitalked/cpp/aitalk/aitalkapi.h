#pragma once

// aitalkapi.h
// 10/11/2014 jichi
#include "aitalk/aitalkdef.h"
#include "aitalk/_windef.h" // for HMODULE

namespace AITalk
{

/* Exported DLL functions of aitalked.dll */

// Initialization

// [DllImport("aitalked.dll", EntryPoint="AITalkAPI_Init")]
// public static extern AITalkResultCode Init(ref AITalk_TConfig config);
#define _AITalkAPI_Init "_AITalkAPI_Init@4"
typedef int (__stdcall *AITalkAPI_Init)(const AITalk_TConfig *config);

// [DllImport("aitalked.dll", EntryPoint="AITalkAPI_End")]
// private static extern AITalkResultCode _End();
#define _AITalkAPI_End "_AITalkAPI_End@0"
typedef int (__stdcall *AITalkAPI_End)();

// Settings

// [DllImport("aitalked.dll", EntryPoint="AITalkAPI_ReloadPhraseDic")]
// public static extern AITalkResultCode ReloadPhraseDic(string pathDic);

// [DllImport("aitalked.dll", EntryPoint="AITalkAPI_ReloadSymbolDic")]
// public static extern AITalkResultCode ReloadSymbolDic(string pathDic);

// [DllImport("aitalked.dll", EntryPoint="AITalkAPI_ReloadWordDic")]
// public static extern AITalkResultCode SetParam(IntPtr pParam);

// Actions

// [DllImport("aitalked.dll", EntryPoint="AITalkAPI_GetParam")]
// public static extern AITalkResultCode GetParam(IntPtr pParam, out uint size);

// [DllImport("aitalked.dll", EntryPoint="AITalkAPI_SetParam")]
// public static extern AITalkResultCode SetParam(IntPtr pParam);

// [DllImport("aitalked.dll", EntryPoint="AITalkAPI_TextToSpeech")]
// private static extern AITalkResultCode _TextToSpeech(out int jobID, ref AITalk_TJobParam param, string text);
#define _AITalkAPI_TextToSpeech "_AITalkAPI_TextToSpeech@12"
typedef int (__stdcall *AITalkAPI_TextToSpeech)(int *jobID, AITalk_TJobParam *param, const char *text);

// [DllImport("aitalked.dll", EntryPoint="AITalkAPI_CloseSpeech")]
// private static extern AITalkResultCode _CloseSpeech(int jobID, int useEvent = 0);

/* Manager class */

class AITalkAPI
{
public:
  AITalkAPI_Init Init;
  AITalkAPI_End End;
  AITalkAPI_TextToSpeech TextToSpeech;

  /**
   * @param  h  aitalked.dll module
   *
   * Initialize function pointers in the DLL module.
   */
  explicit AITalkAPI(HMODULE h);

  ///  Do nothing.
  ~AITalkAPI();

  ///  Return if there are null methods.
  bool IsValid() const
  { return Init && End && TextToSpeech; }
};

} // namespace AITalk
