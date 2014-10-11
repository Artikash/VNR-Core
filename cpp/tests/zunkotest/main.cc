// main.cc
// 10/11/2014 jichi
#include <windows.h>
#include <iostream>
#include <string>

int main()
{
  std::cerr << "enter" << std::endl;
  HMODULE h = ::LoadLibraryA("aitalked.dll");
  std::cerr << h << std::endl;
  //if (h) {
  //  Init init = (Init)::GetProcAddress(h, "_AITalkAPI_Init@4");
  //  TextToSpeech tts = (TextToSpeech)::GetProcAddress(h, "_AITalkAPI_TextToSpeech@12");

  //  AITalk_TConfig config;
  //  config.hzVoiceDB = 0x5622;
  //  config.dirVoiceDBS = "C:\\Program Files\\AHS\\VOICEROID+\\zunko";
  //  config.msecTimeout = 1000;
  //  config.pathLicense = "C:\\Program Files\\AHS\\VOICEROID+\\zunko\\aitalk.lic";
  //  config.codeAuthSeed = "NqKN148elpuO2tmdCMCU";
  //  config.lenAuthSeed = 0x0;
  //  int r = init(&config);
  //  std::cerr << "init: " << r << std::endl;

  //  AITalk_TJobParam param;
  //  param.modeInOut = AITALKIOMODE_PLAIN_TO_WAVE;
  //  param.userData = 0x0;
  //  int jobid = -1;
  //  //const char *t = "hello";
  //  const char *t = "\x83\x6E\x21\x83\x8D\x81\x5B";
  //  r = tts(&jobid, &param, t);
  //  std::cerr << "tts: " << r << std::endl;

  //  system("pause");
  //}
  std::cerr << "leave" << std::endl;
  return 0;
}

// EOF

//struct AITalk_TConfig
//{
//  int hzVoiceDB;
//  const char *dirVoiceDBS;
//  unsigned int msecTimeout;
//  const char *pathLicense;
//  const char *codeAuthSeed;
//  unsigned int lenAuthSeed;
//};
//
//enum AITalkJobInOut
//{
//  AITALKIOMODE_AIKANA_TO_JEITA = 0x17,
//  AITALKIOMODE_AIKANA_TO_WAVE = 12,
//  AITALKIOMODE_JEITA_TO_WAVE = 13,
//  AITALKIOMODE_PLAIN_TO_AIKANA = 0x15,
//  AITALKIOMODE_PLAIN_TO_WAVE = 11
//};
//
//struct AITalk_TJobParam
//{
//  int modeInOut; // AITalkJobInOut
//  void *userData;
//};
//
//struct AITalk_TTtsParam
//{
//  const int MAX_VOICENAME_ = 80;
//  uint size;
//  AITalkProcTextBuf procTextBuf;
//  AITalkProcRawBuf procRawBuf;
//  AITalkProcEventTTS procEventTts;
//  uint lenTextBufBytes;
//  uint lenRawBufBytes;
//  float volume;
//  int pauseBegin;
//  int pauseTerm;
//  string voiceName;
//  TJeitaParam Jeita;
//  uint numSpeakers;
//  int __reserved__;
//  TSpeakerParam[] Speaker;
//
//  // Nested Types
//  struct TJeitaParam
//  {
//    string femaleName;
//    string maleName;
//    int pauseMiddle;
//    int pauseLong;
//    int pauseSentence;
//    string control;
//  };
//
//  struct TSpeakerParam
//  {
//    string voiceName;
//    float volume;
//    float speed;
//    float pitch;
//    float range;
//    int pauseMiddle;
//    int pauseLong;
//    int pauseSentence;
//  };
//};
//
//typedef int (__stdcall *Init)(const AITalk_TConfig *config);
//typedef int (__stdcall *TextToSpeech)(int *jobID, AITalk_TJobParam *param, const char *sjistext);
