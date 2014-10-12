// aitalked.cc
// 10/11/2014 jichi
#include "aitalked/aitalked.h"
#include "aitalked/aitalkapi.h"
#include "aitalked/aitalkconfig.h"
#include "cpputil/cpppath.h"
#include <windows.h>
#include <cstring>

using namespace AITalk;

// Helper functions

namespace { // unnamed

bool _InitFunctions(HMODULE h)
{
#define get(h, fun, entrypoint) \
  (fun = (_##fun)::GetProcAddress(h, entrypoint))

  bool ok = true;
  ok = ok && get(h, AITalkAPI_Init, "_AITalkAPI_Init@4");
  ok = ok && get(h, AITalkAPI_End, "_AITalkAPI_End@0");
  ok = ok && get(h, AITalkAPI_TextToSpeech, "_AITalkAPI_TextToSpeech@12");

  return ok;
#undef get
}

void _DestroyFunctions()
{
  AITalkAPI_End = nullptr; // do this first because the way I implement destroy
  AITalkAPI_Init = nullptr;
  AITalkAPI_TextToSpeech = nullptr;
}

bool _InitAPI(const char *dllpath)
{
  static char dirpath_[MAX_PATH]; // path to zunko
  static char licpath_[MAX_PATH]; // path to aitalk.lic

  size_t dirlen = ::cpp_wdirlen(dllpath);
  if (!dirlen)
    return false;

  ::strncpy(dirpath_, dllpath, dirlen - 1);

  ::strncpy(licpath_, dllpath, dirlen);
  ::strcpy(licpath_ + dirlen, AITALK_CONFIG_LICENSE);

  AITalk_TConfig config;
  config.hzVoiceDB = AITALK_CONFIG_FREQ;
  config.msecTimeout = AITALK_CONFIG_TIMEOUT;
  config.dirVoiceDBS = dirpath_; // r'C:\Program Files\AHS\VOICEROID+\zunko'
  config.pathLicense = licpath_; // r'C:\Program Files\AHS\VOICEROID+\zunko\aitalk.lic'
  config.codeAuthSeed = AITALK_CONFIG_CODEAUTHSEED;
  config.lenAuthSeed = AITALK_CONFIG_LENAUTHSEED;
  return 0 == AITalkAPI_Init(&config);
}

} // unnamed namespace

// API

bool AITalk::Init(void *h)
{
  char path[MAX_PATH];
  return ::GetModuleFileNameA((HMODULE)h, path, MAX_PATH)
      && _InitFunctions((HMODULE)h)
      && _InitAPI(path);
}

void AITalk::Destroy()
{
  if (AITalkAPI_End)
    AITalkAPI_End();
  _DestroyFunctions();
}

// EOF
