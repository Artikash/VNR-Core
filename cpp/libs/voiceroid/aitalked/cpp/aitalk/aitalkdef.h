#pragma once

// aitalkdef.h
// 10/11/2014 jichi

namespace AITalk
{

/* Contants */

// C#:
//   namespace AITalk
//   {
//     using System;
//
//     public enum AITalkJobInOut
//     {
//       AITALKIOMODE_AIKANA_TO_JEITA = 0x17,
//       AITALKIOMODE_AIKANA_TO_WAVE = 12,
//       AITALKIOMODE_JEITA_TO_WAVE = 13,
//       AITALKIOMODE_PLAIN_TO_AIKANA = 0x15,
//       AITALKIOMODE_PLAIN_TO_WAVE = 11
//     }
//   }
enum AITalkJobInOut : int
{
  AITALKIOMODE_AIKANA_TO_JEITA = 23
  , AITALKIOMODE_PLAIN_TO_AIKANA = 21
  , AITALKIOMODE_JEITA_TO_WAVE = 13
  , AITALKIOMODE_AIKANA_TO_WAVE = 12
  , AITALKIOMODE_PLAIN_TO_WAVE = 11
};

// C#:
//   namespace AITalk
//   {
//     using System;
//
//     public enum AITalkStatusCode
//     {
//       AITALKSTAT_DONE = 12,
//       AITALKSTAT_INPROGRESS = 10,
//       AITALKSTAT_STILL_RUNNING = 11,
//       AITALKSTAT_WRONG_STATE = -1
//     }
enum AITalkStatusCode
{
  AITALKSTAT_WRONG_STATE = -1
  , AITALKSTAT_INPROGRESS = 10
  , AITALKSTAT_STILL_RUNNING = 11
  , AITALKSTAT_DONE = 12
};

// C#:
//   namespace AITalk
//   {
//     using System;
//
//     public enum AITalkResultCode
//     {
//       AITALKERR_ALREADY_INITIALIZED = 10,
//       AITALKERR_ALREADY_LOADED = 11,
//       AITALKERR_FILE_NOT_FOUND = -1001,
//       AITALKERR_INSUFFICIENT = -20,
//       AITALKERR_INTERNAL_ERROR = -1,
//       AITALKERR_INVALID_ARGUMENT = -3,
//       AITALKERR_INVALID_JOBID = -202,
//       AITALKERR_LICENSE_ABSENT = -100,
//       AITALKERR_LICENSE_EXPIRED = -101,
//       AITALKERR_LICENSE_REJECTED = -102,
//       AITALKERR_NOMORE_DATA = 0xcc,
//       AITALKERR_NOT_INITIALIZED = -10,
//       AITALKERR_NOT_LOADED = -11,
//       AITALKERR_PARTIALLY_REGISTERED = 0x15,
//       AITALKERR_PATH_NOT_FOUND = -1002,
//       AITALKERR_READ_FAULT = -1003,
//       AITALKERR_SUCCESS = 0,
//       AITALKERR_TOO_MANY_JOBS = -201,
//       AITALKERR_UNSUPPORTED = -2,
//       AITALKERR_USERDIC_LOCKED = -1011,
//       AITALKERR_USERDIC_NOENTRY = -1012,
//       AITALKERR_WAIT_TIMEOUT = -4
//     }
//   }
enum AITalkResultCode : int
{
  AITALKERR_SUCCESS = 0

  , AITALKERR_ALREADY_INITIALIZED = 10
  , AITALKERR_ALREADY_LOADED = 11
  , AITALKERR_PARTIALLY_REGISTERED = 21
  , AITALKERR_NOMORE_DATA = 204

  , AITALKERR_INTERNAL_ERROR = -1
  , AITALKERR_UNSUPPORTED = -2
  , AITALKERR_INVALID_ARGUMENT = -3
  , AITALKERR_WAIT_TIMEOUT = -4
  , AITALKERR_NOT_INITIALIZED = -10
  , AITALKERR_NOT_LOADED = -11
  , AITALKERR_INSUFFICIENT = -20
  , AITALKERR_TOO_MANY_JOBS = -201
  , AITALKERR_INVALID_JOBID = -202
  , AITALKERR_LICENSE_ABSENT = -100
  , AITALKERR_LICENSE_EXPIRED = -101
  , AITALKERR_LICENSE_REJECTED = -102
  , AITALKERR_FILE_NOT_FOUND = -1001
  , AITALKERR_PATH_NOT_FOUND = -1002
  , AITALKERR_READ_FAULT = -1003
  , AITALKERR_USERDIC_LOCKED = -1011
  , AITALKERR_USERDIC_NOENTRY = -1012
};

/* Structures */

// C#:
//   namespace AITalk
//   {
//     using System;
//     using System.Runtime.InteropServices;
//
//     [StructLayout(LayoutKind.Sequential)]
//     public struct AITalk_TConfig
//     {
//       public int hzVoiceDB;
//       public string dirVoiceDBS;
//       public uint msecTimeout;
//       public string pathLicense;
//       public string codeAuthSeed;
//       public uint lenAuthSeed;
//     }
//   }
struct AITalk_TConfig
{
  int hzVoiceDB;
  const char *dirVoiceDBS;
  unsigned int msecTimeout;
  const char *pathLicense;
  const char *codeAuthSeed;
  unsigned int lenAuthSeed;
};

// C#:
//   namespace AITalk
//   {
//     using System;
//     using System.Runtime.InteropServices;
//
//     [StructLayout(LayoutKind.Sequential)]
//     public struct AITalk_TJobParam
//     {
//       public AITalkJobInOut modeInOut;
//       public IntPtr userData;
//     }
//   }
struct AITalk_TJobParam
{
  AITalkJobInOut modeInOut;
  int *userData;
};

} // namespace AITalk
