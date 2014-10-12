// aiaudioapi.cc
// 10/12/2014 jichi
#include "aitalk/aiaudioapi.h"
#include <windows.h>

/* Global variables */

#define ctor(var) \
  var((AIAudioAPI_##var)::GetProcAddress(h, _AIAudioAPI_##var))

AITalk::AIAudioAPI::AIAudioAPI(HMODULE h)
  : ctor(Open)
  , ctor(Close)
{}
#undef ctor

AITalk::AIAudioAPI::~AIAudioAPI() {}

// EOF
