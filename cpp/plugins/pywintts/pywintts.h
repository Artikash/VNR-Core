#ifndef PYWINTTS_H
#define PYWINTTS_H

// pywintts.h
// 4/7/2013 jichi

#include "sakurakit/skglobal.h"
#include <string>

class WinTts_p;
class WinTts
{
  SK_CLASS(WinTts)
  SK_DISABLE_COPY(WinTts)
  SK_DECLARE_PRIVATE(WinTts_p)

public:
  // - Construction -

  WinTts();
  ~WinTts();

  bool isValid() const;

  // Example registry: L"HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Speech\\Voices\\Tokens\\VW Misaki"
  bool setVoice(const std::wstring& registry);
  std::wstring voice() const;
  bool hasVoice() const;

  // - Actions -

  bool speak(const std::wstring& text, bool async = false) const;
  bool purge(bool async = false) const; // clear previous speak
};

#endif // PYWINTTS
