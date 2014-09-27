// pywintts.cc
// 4/7/2013

#include "pywintts/pywintts.h"
#include "wintts/wintts.h"

/** Private class */

class WinTts_p
{
public:
  wintts_t *tts;
  std::wstring voice; // voice registry path

  WinTts_p(): tts(::wintts_create()) {}
  ~WinTts_p() { ::wintts_destroy(tts); }
};

/** Public class */

// - Construction -

WinTts::WinTts() : d_(new D) {}
WinTts::~WinTts() { delete d_; }

bool WinTts::isValid() const { return d_->tts; }

bool WinTts::setVoice(const std::wstring &registry)
{
  bool ret = false;
  if (d_->voice != registry)
    if (ret = ::wintts_set_voice(d_->tts, registry.c_str()))
      d_->voice = ret;
  return ret;
}

std::wstring WinTts::voice() const { return d_->voice; }
bool WinTts::hasVoice() const { return !d_->voice.empty(); }

// - Actions -

bool WinTts::purge(bool async) const
{ return ::wintts_speak(d_->tts, nullptr, WINTTS_PURGE | (async ? WINTTS_ASYNC : 0)); }

bool WinTts::speak(const std::wstring &text, bool async) const
{ return ::wintts_speak(d_->tts, text.c_str(), async ? WINTTS_ASYNC : WINTTS_DEFAULT); }

// EOF
