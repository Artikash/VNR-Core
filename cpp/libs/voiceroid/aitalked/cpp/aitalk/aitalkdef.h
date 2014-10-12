#pragma once

// aitalkdef.h
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

} // namespace AITalk
