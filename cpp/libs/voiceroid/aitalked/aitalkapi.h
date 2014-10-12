#pragma once

// aitalkapi.h
// 10/11/2014 jichi

namespace AITalk
{

/* Contants */

// namespace AITalk
// {
//   using System;
//
//   public enum AITalkJobInOut
//   {
//     AITALKIOMODE_AIKANA_TO_JEITA = 0x17,
//     AITALKIOMODE_AIKANA_TO_WAVE = 12,
//     AITALKIOMODE_JEITA_TO_WAVE = 13,
//     AITALKIOMODE_PLAIN_TO_AIKANA = 0x15,
//     AITALKIOMODE_PLAIN_TO_WAVE = 11
//   }
// }
enum AITalkJobInOut : int
{
  AITALKIOMODE_AIKANA_TO_JEITA = 23
  , AITALKIOMODE_PLAIN_TO_AIKANA = 21
  , AITALKIOMODE_JEITA_TO_WAVE = 13
  , AITALKIOMODE_AIKANA_TO_WAVE = 12
  , AITALKIOMODE_PLAIN_TO_WAVE = 11
};

/* Structures */

// namespace AITalk
// {
//   using System;
//   using System.Runtime.InteropServices;
//
//   [StructLayout(LayoutKind.Sequential)]
//   public struct AITalk_TConfig
//   {
//     public int hzVoiceDB;
//     public string dirVoiceDBS;
//     public uint msecTimeout;
//     public string pathLicense;
//     public string codeAuthSeed;
//     public uint lenAuthSeed;
//   }
// }
struct AITalk_TConfig
{
  int hzVoiceDB;
  const char *dirVoiceDBS;
  unsigned int msecTimeout;
  const char *pathLicense;
  const char *codeAuthSeed;
  unsigned int lenAuthSeed;
};

// namespace AITalk
// {
//   using System;
//   using System.Runtime.InteropServices;
//
//   [StructLayout(LayoutKind.Sequential)]
//   public struct AITalk_TJobParam
//   {
//     public AITalkJobInOut modeInOut;
//     public IntPtr userData;
//   }
// }
struct AITalk_TJobParam
{
  AITalkJobInOut modeInOut;
  int *userData;
};

/* Functions, ctypes.WinDLL instead of ctypes.CDLL */

// Initialization

// [DllImport("aitalked.dll", EntryPoint="AITalkAPI_Init")]
// public static extern AITalkResultCode Init(ref AITalk_TConfig config);
typedef int (__stdcall *_AITalkAPI_Init)(const AITalk_TConfig *config);
extern _AITalkAPI_Init AITalkAPI_Init;

// [DllImport("aitalked.dll", EntryPoint="AITalkAPI_End")]
// private static extern AITalkResultCode _End();
typedef int (__stdcall *_AITalkAPI_End)();
extern _AITalkAPI_End AITalkAPI_End;

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
typedef int (__stdcall *_AITalkAPI_TextToSpeech)(int *jobID, AITalk_TJobParam *param, const char *text);
extern _AITalkAPI_TextToSpeech AITalkAPI_TextToSpeech;

// [DllImport("aitalked.dll", EntryPoint="AITalkAPI_CloseSpeech")]
// private static extern AITalkResultCode _CloseSpeech(int jobID, int useEvent = 0);

} // namespace AITalk
