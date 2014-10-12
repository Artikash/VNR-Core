// aitalkapi.cc
// 10/11/2014 jichi
#include "aitalk/aitalkapi.h"
#include <windows.h>

/* Global variables */

#define ctor(var) \
  var((AITalkAPI_##var)::GetProcAddress(h, _AITalkAPI_##var))

AITalk::AITalkAPI::AITalkAPI(HMODULE h)
  : ctor(Init)
  , ctor(CloseSpeech)
  , ctor(End)
  , ctor(GetStatus)
  , ctor(TextToSpeech)
{}
#undef ctor

AITalk::AITalkAPI::~AITalkAPI() {}

// EOF
