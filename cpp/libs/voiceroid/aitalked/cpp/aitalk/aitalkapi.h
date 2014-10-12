#pragma once

// aitalkapi.h
// 10/11/2014 jichi
#include "aitalk/aitalkdef.h"
#include "aitalk/_windef.h" // for HMODULE
#include <sal.h> // for _Out_

namespace AITalk
{

/* Exported DLL functions of aitalked.dll */

// Initialization

// C#: [DllImport("aitalked.dll", EntryPoint="AITalkAPI_Init")]
//     public static extern AITalkResultCode Init(ref AITalk_TConfig config);
#define _AITalkAPI_Init "_AITalkAPI_Init@4"
typedef AITalkResultCode (__stdcall *AITalkAPI_Init)(const AITalk_TConfig *config);

// C#: [DllImport("aitalked.dll", EntryPoint="AITalkAPI_End")]
//     private static extern AITalkResultCode _End();
#define _AITalkAPI_End "_AITalkAPI_End@0"
typedef AITalkResultCode (__stdcall *AITalkAPI_End)();

// Settings

// C#: [DllImport("aitalked.dll", EntryPoint="AITalkAPI_ReloadPhraseDic")]
//     public static extern AITalkResultCode ReloadPhraseDic(string pathDic);

// C#: [DllImport("aitalked.dll", EntryPoint="AITalkAPI_ReloadSymbolDic")]
//     public static extern AITalkResultCode ReloadSymbolDic(string pathDic);

// C#: [DllImport("aitalked.dll", EntryPoint="AITalkAPI_ReloadWordDic")]
//     public static extern AITalkResultCode SetParam(IntPtr pParam);

// Actions

// C#: [DllImport("aitalked.dll", EntryPoint="AITalkAPI_GetParam")]
//     public static extern AITalkResultCode GetParam(IntPtr pParam, out uint size);

// C#: [DllImport("aitalked.dll", EntryPoint="AITalkAPI_SetParam")]
//     public static extern AITalkResultCode SetParam(IntPtr pParam);

// C#: [DllImport("aitalked.dll", EntryPoint="AITalkAPI_TextToSpeech")]
//     private static extern AITalkResultCode _TextToSpeech(out int jobID, ref AITalk_TJobParam param, string text);
#define _AITalkAPI_TextToSpeech "_AITalkAPI_TextToSpeech@12"
typedef AITalkResultCode (__stdcall *AITalkAPI_TextToSpeech)(
    _Out_ int *jobID, const AITalk_TJobParam *param, const char *text);

// C#: [DllImport("aitalked.dll", EntryPoint="AITalkAPI_CloseSpeech")]
//     private static extern AITalkResultCode _CloseSpeech(int jobID, int useEvent = 0);
#define _AITalkAPI_CloseSpeech "_AITalkAPI_CloseSpeech@8"
typedef AITalkResultCode (__stdcall *AITalkAPI_CloseSpeech)(int jobID, int useEvent);

// C#: [DllImport("aitalked.dll", EntryPoint="AITalkAPI_GetStatus")]
//     private static extern AITalkResultCode _GetStatus(int jobID, out AITalkStatusCode status);
#define _AITalkAPI_GetStatus "_AITalkAPI_GetStatus"
typedef AITalkResultCode (__stdcall *AITalkAPI_GetStatus)(int jobID, _Out_ AITalkStatusCode *status);

/* Manager class */

class AITalkAPI
{
public:
  AITalkAPI_Init Init;
  AITalkAPI_CloseSpeech CloseSpeech;
  AITalkAPI_End End;
  AITalkAPI_GetStatus GetStatus;
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
  {
    return Init
      && CloseSpeech
      && End
      && GetStatus
      && TextToSpeech
    ;
  }
};

} // namespace AITalk
