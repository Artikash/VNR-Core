// aitalkapi.cc
// 10/11/2014 jichi
#include "aitalk/aitalkapi.h"
#include <windows.h>

/* Global variables */

#define ctor(var) \
  var((AITalkAPI_##var)::GetProcAddress(h, _AITalkAPI_##var))

AITalk::AITalkAPI::AITalkAPI(HMODULE h)
  : ctor(Init)
  , ctor(BLoadWordDic)
  , ctor(CloseKana)
  , ctor(CloseSpeech)
  , ctor(End)
  , ctor(GetData)
  , ctor(GetJeitaControl)
  , ctor(GetParam)
  , ctor(GetStatus)
  , ctor(LangClear)
  , ctor(LangLoad)
  , ctor(ReloadPhraseDic)
  , ctor(ReloadSymbolDic)
  , ctor(ReloadWordDic)
  , ctor(SetParam)
  , ctor(TextToKana)
  , ctor(TextToSpeech)
  , ctor(VersionInfo)
  , ctor(VoiceClear)
  , ctor(VoiceLoad)
{}
#undef ctor

AITalk::AITalkAPI::~AITalkAPI() {}

// EOF
