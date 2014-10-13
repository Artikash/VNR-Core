// aitalkutil.cc
// 10/11/2014 jichi
#include "aitalk/aitalkutil.h"
#include "aitalk/aitalkapi.h"
#include "aitalk/aitalkmarshal.h"
#include "cpputil/cpppath.h"
#include "cc/ccmacro.h"
#include <windows.h>
#include <cstring>

using namespace AITalk;

// Static variables
AITalkUtil *AITalk::AITalkUtil::_instance;

// Construction

AITalk::AITalkUtil::AITalkUtil(HMODULE h)
  : _talk(h), _audio(h), _valid(false)
{
  if (!_talk.IsValid() || !_audio.IsValid())
    return;

  // Initialize talk API

  char dllpath[MAX_PATH];
  if (!::GetModuleFileNameA(h, dllpath, MAX_PATH))
    return;

  size_t dirlen = ::cpp_wdirlen(dllpath);
  if (dirlen < 1)
    return;

  ::memcpy(_dirpath, dllpath, dirlen - 1);
  _dirpath[dirlen - 1] = 0;

  ::memcpy(_licpath, dllpath, dirlen);
  ::strcpy(_licpath + dirlen, AITALK_CONFIG_LICENSE);

  {
    AITalk_TConfig config;
    config.hzVoiceDB = AITALK_CONFIG_FREQUENCY;
    config.msecTimeout = AITALK_CONFIG_TIMEOUT;
    //config.dirVoiceDBS = _dirpath; // r'C:\Program Files\AHS\VOICEROID+\zunko'
    config.dirVoiceDBS = "C:\\Program Files\\AHS\\VOICEROID+\\zunko\\voice";
    //config.pathLicense = _licpath; // r'C:\Program Files\AHS\VOICEROID+\zunko\aitalk.lic'
    config.pathLicense = "C:\\Program Files\\AHS\\VOICEROID+\\zunko\\aitalk.lic";
    config.codeAuthSeed = AITALK_CONFIG_CODEAUTHSEED;
    config.lenAuthSeed = AITALK_CONFIG_LENAUTHSEED;

    if (_talk.Init(&config) != AITALKERR_SUCCESS)
      return;
  }

  AITalkResultCode code;
  static char *langDir =  "C:\\Program Files\\AHS\\VOICEROID+\\zunko\\lang";
  code = _talk.LangLoad(langDir);
  if (code != AITALKERR_SUCCESS)
    return;

  static char *voiceName = "zunko_22";
  code = _talk.VoiceLoad(voiceName);
  if (code != AITALKERR_SUCCESS)
    return;

  // Initialize audio API
  {
    AIAudio_TConfig config;
    config.msecLatency = AIAUDIO_CONFIG_BUFFERLATENCY;
    config.lenBufferBytes = AITALK_CONFIG_FREQUENCY * 2 * AIAUDIO_CONFIG_BUFFERLENGTH;
    config.hzSamplesPerSec = AITALK_CONFIG_FREQUENCY;
    config.formatTag = AIAUDIOTYPE_PCM_16;
    config.__reserved__ = 0;
    if (_audio.Open(&config) != AIAUDIOERR_SUCCESS)
      return;
  }

  _valid = true;
  _instance = this;
}

AITalk::AITalkUtil::~AITalkUtil()
{
  _instance = nullptr;
  if (_valid) {
    _audio.Close();
    _talk.End();
  }
}

// APIs wrappers

AIAudioResultCode AITalk::AITalkUtil::PushData(short wave[], unsigned int size, int stop)
{
  //if (!this._playing)
  //  return AIAudioResultCode.AIAUDIOERR_NO_PLAYING;
  if (!wave)
    return AIAUDIOERR_INVALID_ARGUMENT;
  size_t dstlen = size * 2;
  char *dst = new char[dstlen];
  if (dstlen)
    ::memcpy(dst, wave, dstlen);
  AIAudioResultCode code = _audio.PushData(dst, dstlen, stop);
  //if (code != AIAudioResultCode.AIAUDIOERR_SUCCESS)
  //  this._playing = false;
  return code;
}

// Speech synthesize

AITalkResultCode AITalk::AITalkUtil::SynthSync(int *jobID, const AITalk_TJobParam &jobparam, const char *text)
{

  size_t num;
  AITalkResultCode code = _talk.GetParam(nullptr, &num);
  if (code != AITALKERR_INSUFFICIENT)
    return AITALKERR_INSUFFICIENT;

  char *data = new char[num];
  ((int *)data)[0] = num;
  //IntPtr ptr = Marshal.AllocCoTaskMem((int) num);
  code = _talk.GetParam(data, &num);
  if (code != AITALKERR_SUCCESS) {
    delete data;
    return code;
  }
  AITalk_TTtsParam param;
  AITalkMarshal::ReadTtsParam(&param, data);

  param.procRawBuf = this->MyAITalkProcRawBuf;
  param.procTextBuf = this->MyAITalkProcTextBuf;
  param.procEventTts = this->MyAITalkProcEventTTS;

  AITalkMarshal::WriteTtsParam(data, param);

  AITalk_TTtsParam param2;
  AITalkMarshal::ReadTtsParam(&param2, data);

  code = _talk.SetParam(data);
  if (code != AITALKERR_SUCCESS)
    return code;

  return _talk.TextToSpeech(jobID, &jobparam, text);
}

// Hooks

int AITalk::AITalkUtil::MyAITalkProcEventTTS(AITalkEventReasonCode reasonCode, int jobID, unsigned long tick, const char *name, const int *userData)
{
  CC_UNUSED(jobID);
  CC_UNUSED(name);
  CC_UNUSED(userData);
  switch (reasonCode) {
  case AITALKEVENT_BOOKMARK: _instance->PushEvent(tick, 0); break;
  case AITALKEVENT_PH_LABEL: _instance->PushEvent(tick, 1); break;
  }
  return 0;
}
int AITalk::AITalkUtil::MyAITalkProcTextBuf(AITalkEventReasonCode reasonCode, int jobID, const int *userData)
{
  CC_UNUSED(reasonCode);
  CC_UNUSED(jobID);
  CC_UNUSED(userData);
  //uint size = 0;
  //uint pos = 0;
  //string str = "";
  //if ((reasonCode == AITalkEventReasonCode.AITALKEVENT_TEXTBUF_FLUSH) || (reasonCode == AITalkEventReasonCode.AITALKEVENT_TEXTBUF_FULL))
  //{
  //  AITalkResultCode code = AITalkAPI.GetKana(jobID, this._kanaBuf, (uint) this._kanaBuf.Capacity, out size, out pos);
  //  this.OnWriteLog(string.Concat(new object[] { "[AITalkAPI_GetKana] ", code, " : ", size }));
  //  str = this._kanaBuf.ToString();
  //}
  return 0;
}

int AITalk::AITalkUtil::MyAITalkProcRawBuf(AITalkEventReasonCode reasonCode, int jobID, unsigned long tick, const int *userData)
{
  CC_UNUSED(userData);
  CC_UNUSED(jobID);
  if (!_instance)
    return 0;
  unsigned int size = 0;
  enum { wavelen = 1000 };
  short wave[wavelen];
  switch (reasonCode) {
  case AITALKEVENT_RAWBUF_FLUSH:
  case AITALKEVENT_RAWBUF_FULL:
    if (AITALKERR_SUCCESS == _instance->_talk.GetData(jobID, wave, wavelen, &size) && size > 0) {
      if (reasonCode == AITALKEVENT_RAWBUF_FLUSH)
        _instance->PushEvent(tick, 2);
      _instance->PushData(wave, size, 0);
    }
    break;
  case AITALKEVENT_RAWBUF_CLOSE:
    _instance->PushEvent(tick, 3);
    _instance->PushData(new short[0], 0, 1);
    break;
  }
  return 0;
}

// EOF
