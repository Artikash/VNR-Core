// main.cc
// 10/11/2014 jichi
#include "aitalk/aitalkutil.h"
#include <windows.h>
#include <iostream>
#include <string>

int main()
{
  //const char *text = "hello";
  const char *text = "\x83\x6E\x21\x83\x8D\x81\x5B";
  //
  std::cerr << "enter" << std::endl;
  HMODULE h = ::LoadLibraryA("aitalked.dll");
  std::cerr << h << std::endl;
  if (h) {
    std::cerr << "create ai" << std::endl;
    AITalk::AITalkUtil ai(h);
    bool ok = ai.IsValid();
    std::cerr << "init: " << ok << std::endl;

    int jobID;
    auto r = ai.TextToSpeech(&jobID, text);
    std::cerr << "tts: " << (int)r << ":" << jobID << std::endl;

    system("pause");
    ::Sleep(10000);
    r = ai.CloseSpeech(jobID);
    std::cerr << "close: " << (int)r << ":" << jobID << std::endl;
  }
  std::cerr << "leave" << std::endl;
  return 0;
}

// EOF
