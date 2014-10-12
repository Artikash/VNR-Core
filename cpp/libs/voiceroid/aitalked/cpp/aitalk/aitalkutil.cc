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
  : _api(h), _valid(false)
{
  if (!_api.IsValid())
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

  _valid = _api.Init(&config) == AITALKERR_SUCCESS;
}

AITalk::AITalkUtil::~AITalkUtil()
{
  if (_valid)
    _api.End();
}

// Speech synthesize

AITalkResultCode AITalk::AITalkUtil::SynthSync(int *jobID, const AITalk_TJobParam &jobparam, const char *text)
{
  return _api.TextToSpeech(jobID, &jobparam, text);
  //AITalk_TTtsParam param = new AITalk_TTtsParam();
  //AITalkResultCode res = this.GetParam(ref param);
  //this.OnWriteLog("[AITalkAPI_GetParam] " + res);
  //if (res != AITalkResultCode.AITALKERR_SUCCESS)
  //{
  //    this._busy = false;
  //    this._playing = false;
  //    return res;
  //}
  //param.procTextBuf = this._AITalkProcTextBuf;
  //param.procRawBuf = this._AITalkProcRawBuf;
  //param.procEventTts = this._AITalkProcEventTTS;
  //res = this.SetParam(ref param);
  //this.OnWriteLog("[AITalkAPI_SetParam] " + res);
  //if (res != AITalkResultCode.AITALKERR_SUCCESS)
  //{
  //    this._busy = false;
  //    this._playing = false;
  //    return res;
  //}
  //this.OnWriteLog("[AIAuidoAPI_ClearData] " + AIAudioAPI.ClearData());
  //res = AITalkAPI.TextToSpeech(out jobID, ref jobparam, text);
  //this.OnWriteLog("[AITalkAPI_TextToSpeech] " + res);
  //if (res != AITalkResultCode.AITALKERR_SUCCESS)
  //{
  //    this._busy = false;
  //    this._playing = false;
  //    return res;
  //}
  //new Thread(delegate {
  //    AITalkStatusCode code;
  //    do
  //    {
  //        Thread.Sleep(this.GetStatusInterval);
  //        res = this.GetStatus(jobID, out code);
  //    }
  //    while ((this._playing && (res == AITalkResultCode.AITALKERR_SUCCESS)) && (code != AITalkStatusCode.AITALKSTAT_DONE));
  //    AITalkAPI.CloseSpeech(jobID, 0);
  //    this.OnWriteLog("[AITalkAPI_CloseSpeech] " + res);
  //    this._busy = false;
  //}) { IsBackground = true }.Start();
  //return res;
}

// EOF
