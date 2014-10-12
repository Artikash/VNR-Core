// aitalkutil.cc
// 10/11/2014 jichi
#include "aitalk/aitalkutil.h"
#include "aitalk/aitalkapi.h"
#include "aitalk/aitalkconf.h"
#include "cpputil/cpppath.h"
#include <windows.h>
#include <cstring>

using namespace AITalk;

// Construction

AITalk::AITalkUtil::AITalkUtil(HMODULE h)
  : api_(h), valid_(false)
{
  if (!api_.IsValid())
    return;

  char dllpath[MAX_PATH];
  if (!::GetModuleFileNameA(h, dllpath, MAX_PATH))
    return;

  size_t dirlen = ::cpp_wdirlen(dllpath);
  if (!dirlen)
    return;

  char dirpath[MAX_PATH]; // path to zunko
  ::memcpy(dirpath, dllpath, dirlen - 1);
  dirpath[dirlen] = 0;

  char licpath[MAX_PATH]; // path to aitalk.lic
  ::memcpy(licpath, dllpath, dirlen);
  ::strcpy(licpath + dirlen, AITALK_CONFIG_LICENSE);

  AITalk_TConfig config;
  config.hzVoiceDB = AITALK_CONFIG_FREQ;
  config.msecTimeout = AITALK_CONFIG_TIMEOUT;
  config.dirVoiceDBS = dirpath; // r'C:\Program Files\AHS\VOICEROID+\zunko'
  config.pathLicense = licpath; // r'C:\Program Files\AHS\VOICEROID+\zunko\aitalk.lic'
  config.codeAuthSeed = AITALK_CONFIG_CODEAUTHSEED;
  config.lenAuthSeed = AITALK_CONFIG_LENAUTHSEED;

  valid_ = 0 == api_.Init(&config);
}

AITalk::AITalkUtil::~AITalkUtil()
{
  if (valid_)
    api_.End();
}

// EOF
