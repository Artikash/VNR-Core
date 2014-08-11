// main.cc
// 6/15/2014 jichi
#include "comm/wintts.h"
#include <qt_windows.h>
#include <QtCore/QString>

int main()
{
  //const wchar_t *voice = L"HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Speech\\Voices\\Tokens\\VW Misaki";
  const wchar_t *voice = L"HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Speech\\Voices\\Tokens\\CeVIO-\u3055\u3068\u3046\u3055\u3055\u3089";

  CoInitialize(nullptr);
  wintts_t *tts = wintts_create();
  Q_ASSERT(tts);
  wintts_set_voice(tts, voice);
  wintts_speak(tts, text);
  wintts_destroy(tts);

  CoUninitialize();
  return 0;
}

// EOF
