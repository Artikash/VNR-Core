// wmpcli.cc
// 10/5/2014 jichi

#include "wmpcli/wmpcli.h"
#include <windows.h>
#include "wmp/wmp.h"

// Construction
int wmp_player_release(wmp_player_t *p) { return p->Release(); }
int wmp_controls_release(wmp_controls_t *c) { return c->Release(); }
int wmp_settings_release(wmp_settings_t *s) { return s->Release(); }
int wmp_media_release(wmp_media_t *m) { return m->Release(); }

wmp_player_t *wmp_player_create()
{
  IWMPPlayer *p = nullptr;
  return SUCCEEDED(::CoCreateInstance(CLSID_WindowsMediaPlayer, nullptr, CLSCTX_INPROC_SERVER, IID_IWMPPlayer,
      reinterpret_cast<LPVOID *>(&p))) ? p : nullptr;
}

wmp_controls_t *wmp_player_get_controls(wmp_player_t *p)
{
  IWMPControls *c;
  return SUCCEEDED(p->get_controls(&c)) ? c : nullptr;
}

wmp_settings_t *wmp_player_get_settings(wmp_player_t *p)
{
  IWMPSettings *s;
  return SUCCEEDED(p->get_settings(&s)) ? s : nullptr;
}

// Player
// http://msdn.microsoft.com/en-us/library/windows/desktop/dd563514%28v=vs.85%29.aspx

bool wmp_player_close(wmp_player_t *p) { return SUCCEEDED(p->close()); }

bool wmp_player_set_enabled(wmp_player_t *p, bool t)
{ return SUCCEEDED(p->put_enabled(t)); }

bool wmp_player_get_enabled(wmp_player_t *p)
{
  VARIANT_BOOL t;
  return SUCCEEDED(p->get_enabled(&t)) && t;
}

bool wmp_player_set_fullscreen(wmp_player_t *p, bool t)
{ return SUCCEEDED(p->put_fullScreen(t)); }

bool wmp_player_get_fullscreen(wmp_player_t *p)
{
  VARIANT_BOOL t;
  return SUCCEEDED(p->get_fullScreen(&t)) && t;
}

bool wmp_player_set_contextmenuenabled(wmp_player_t *p, bool t)
{ return SUCCEEDED(p->put_enableContextMenu(t)); }

bool wmp_player_get_contextmenuenabled(wmp_player_t *p)
{
  VARIANT_BOOL t;
  return SUCCEEDED(p->get_enableContextMenu(&t)) && t;
}

bool wmp_player_set_url(wmp_player_t *p, const wchar_t *val)
{ return SUCCEEDED(p->put_URL(const_cast<BSTR>(val))); }

const wchar_t *wmp_player_get_url(wmp_player_t *p)
{
  BSTR val;
  return SUCCEEDED(p->get_URL(&val)) ? val : nullptr;
}

bool wmp_player_set_uimode(wmp_player_t *p, const wchar_t *val)
{ return SUCCEEDED(p->put_uiMode(const_cast<BSTR>(val))); }

const wchar_t *wmp_player_get_uimode(wmp_player_t *p)
{
  BSTR val;
  return SUCCEEDED(p->get_uiMode(&val)) ? val : nullptr;
}

// Control

bool wmp_controls_play(wmp_controls_t *c) { return SUCCEEDED(c->play()); }
bool wmp_controls_stop(wmp_controls_t *c) { return SUCCEEDED(c->stop()); }
bool wmp_controls_pause(wmp_controls_t *c) { return SUCCEEDED(c->pause()); }

bool wmp_controls_previous(wmp_controls_t *c) { return SUCCEEDED(c->previous()); }
bool wmp_controls_next(wmp_controls_t *c) { return SUCCEEDED(c->next()); }

bool wmp_controls_forward(wmp_controls_t *c) { return SUCCEEDED(c->fastForward()); }
bool wmp_controls_backward(wmp_controls_t *c) { return SUCCEEDED(c->fastReverse()); }

bool wmp_controls_set_pos(wmp_controls_t *c, double val)
{ return SUCCEEDED(c->put_currentPosition(val)); }

double wmp_controls_get_pos(wmp_controls_t *c)
{
  double val;
  return SUCCEEDED(c->get_currentPosition(&val)) ? val : -1;
}

bool wmp_controls_set_media(wmp_controls_t *c, wmp_media_t *m)
{ return SUCCEEDED(c->put_currentItem(m)); }

wmp_media_t *wmp_controls_get_media(wmp_controls_t *c)
{
  IWMPMedia *m;
  return SUCCEEDED(c->get_currentItem(&m)) ? m : nullptr;
}

// Settings

bool wmp_settings_set_errordialogsenabled(wmp_settings_t *s, bool t)
{ return SUCCEEDED(s->put_enableErrorDialogs(t)); }

bool wmp_settings_get_errordialogsenabled(wmp_settings_t *s)
{
  VARIANT_BOOL t;
  return SUCCEEDED(s->get_enableErrorDialogs(&t)) && t;
}

bool wmp_settings_set_autostart(wmp_settings_t *s, bool t)
{ return SUCCEEDED(s->put_autoStart(t)); }

bool wmp_settings_get_autostart(wmp_settings_t *s)
{
  VARIANT_BOOL t;
  return SUCCEEDED(s->get_autoStart(&t)) && t;
}

bool wmp_settings_set_baseurl(wmp_settings_t *s, const wchar_t *val)
{ return SUCCEEDED(s->put_baseURL(const_cast<BSTR>(val))); }

const wchar_t *wmp_settings_get_baseurl(wmp_settings_t *s)
{
  BSTR val;
  return SUCCEEDED(s->get_baseURL(&val)) ? val : nullptr;
}

bool wmp_settings_set_volume(wmp_settings_t *s, int val)
{ return SUCCEEDED(s->put_volume(val)); }

int wmp_settings_get_volume(wmp_settings_t *s)
{
  long val;
  return SUCCEEDED(s->get_volume(&val)) ? val : -1;
}

bool wmp_settings_set_balance(wmp_settings_t *s, int val)
{ return SUCCEEDED(s->put_balance(val)); }

int wmp_settings_get_balance(wmp_settings_t *s)
{
  long val;
  return SUCCEEDED(s->get_balance(&val)) ? val : 0;
}

bool wmp_settings_set_mute(wmp_settings_t *s, bool t)
{ return SUCCEEDED(s->put_mute(t)); }

bool wmp_settings_get_mute(wmp_settings_t *s)
{
  VARIANT_BOOL t;
  return SUCCEEDED(s->get_mute(&t)) && t;
}

bool wmp_settings_set_rate(wmp_settings_t *s, double val)
{ return SUCCEEDED(s->put_rate( val)); }

double wmp_settings_get_rate(wmp_settings_t *s)
{
  double val;
  return SUCCEEDED(s->get_rate(&val)) ? val : -1;
}

bool wmp_settings_set_playcount(wmp_settings_t *s, int val)
{ return SUCCEEDED(s->put_playCount(val)); }

int wmp_settings_get_playcount(wmp_settings_t *s)
{
  long val;
  return SUCCEEDED(s->get_playCount(&val)) ? val : 0;
}

// Shuffle the order
bool wmp_settings_set_shuffle(wmp_settings_t *s, bool t)
{ return SUCCEEDED(s->setMode(L"shuffle", t)); }

bool wmp_settings_get_shuffle(wmp_settings_t *s)
{
  VARIANT_BOOL t;
  return SUCCEEDED(s->getMode(L"shuffle", &t)) && t;
}

// Repeat the tracks
bool wmp_settings_set_repeat(wmp_settings_t *s, bool t)
{ return SUCCEEDED(s->setMode(L"loop", t)); }

bool wmp_settings_get_repeat(wmp_settings_t *s)
{
  VARIANT_BOOL t;
  return SUCCEEDED(s->getMode(L"loop", &t)) && t;
}

bool wmp_settings_set_autorewind(wmp_settings_t *s, bool t)
{ return SUCCEEDED(s->setMode(L"autoRewind", t)); }

bool wmp_settings_get_autorewind(wmp_settings_t *s)
{
  VARIANT_BOOL t;
  return SUCCEEDED(s->getMode(L"autoRewind", &t)) && t;
}

// Media

bool wmp_media_equal(wmp_media_t *x, wmp_media_t *y)
{
  VARIANT_BOOL t;
  return x == y
      || x && y && SUCCEEDED(x->get_isIdentical(y, &t)) && t;
}

double wmp_media_get_duration(wmp_media_t *m)
{
  double val;
  return SUCCEEDED(m->get_duration(&val)) ? val : -1;
}

bool wmp_media_set_name(wmp_media_t *m, const wchar_t *val)
{ return SUCCEEDED(m->put_name(const_cast<BSTR>(val))); }

const wchar_t *wmp_media_get_name(wmp_media_t *m)
{
  BSTR val;
  return SUCCEEDED(m->get_name(&val)) ? val : nullptr;
}

int wmp_media_get_imagewidth(wmp_media_t *m)
{
  long val;
  return SUCCEEDED(m->get_imageSourceWidth(&val)) ? val : -1;
}

int wmp_media_get_imageheight(wmp_media_t *m)
{
  long val;
  return SUCCEEDED(m->get_imageSourceHeight(&val)) ? val : -1;
}

const wchar_t *wmp_media_get_url(wmp_media_t *m)
{
  BSTR val;
  return SUCCEEDED(m->get_sourceURL(&val)) ? val : nullptr;
}

// EOF
