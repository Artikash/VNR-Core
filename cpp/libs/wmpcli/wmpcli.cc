// wmpcli.cc
// 10/5/2014 jichi

#include "wincli/wincli.h"
#include <windows.h>
#include "wmp/wmp.h"
#include "cc/ccmacro.h"

// Construction
void wmp_player_release(wmp_player_t *p) { p->Release(); }
void wmp_control_release(wmp_control_t *c) { c->Release(); }
void wmp_settings_release(wmp_settings_t *s) { s->Release(); }
void wmp_media_release(wmp_media_t *m) { m->Release(); }

wmp_player_t *wmp_player_create()
{
  IWMPlayer *p = nullptr;
  return SUCCEEDED(::CoCreateInstance(CLSID_WindowsMediaPlayer, nullptr, CLSCTX_INPROC_SERVER, IID_IWMPPlayer,
      reinterpret_cast<LPVOID *>(&p))) ? p : nullptr;
}

wmp_control_t *wmp_player_get_control(wmp_player_t *p);
{
  IWMPControls *c;
  return SUCCEEDED(player->get_controls(&c)) ? c : nullptr;
}

wmp_settings_t *wmp_player_get_settings(wmp_player_t *p);
{
  IWMPSettings *s;
  return SUCCEEDED(player->get_settings(&s)) ? s : nullptr;
}

// Player
// http://msdn.microsoft.com/en-us/library/windows/desktop/dd563514%28v=vs.85%29.aspx

// Player is disabled on the startup
bool wmp_player_set_enabled(wmp_player_t *p, bool t)
{ return SUCCEEDED(player->put_enabled(p, t); }

bool wmp_player_get_enabled(wmp_player_t *p)
{
  VARIANT_BOOL t;
  return SUCCEEDED(player->get_enabled(p, &t)) && t;
}

bool wmp_player_set_fullscreen(wmp_player_t *p, bool t)
{ return SUCCEEDED(player->put_fullScreen(p, t); }

bool wmp_player_get_fullscreen(wmp_player_t *p)
{
  VARIANT_BOOL t;
  return SUCCEEDED(player->get_fullScreen(p, &t)) && t;
}

bool wmp_player_set_contextmenuenabled(wmp_player_t *p, bool t)
{ return SUCCEEDED(player->put_enableContextMenu(p, t); }

bool wmp_player_get_contextmenuenabled(wmp_player_t *p)
{
  VARIANT_BOOL t;
  return SUCCEEDED(player->get_enableContextMenu(p, &t)) && t;
}

bool wmp_player_set_url(wmp_player_t *p, const wchar_t *val);
{ return SUCCEEDED(player->put_URL(p, val); }

const wchar_t *wmp_player_get_url(wmp_player_t *p);
{
  BSTR val;
  return SUCCEEDED(player->get_URL(p, &val)) ? val : nullptr;
}

bool wmp_player_set_uimode(wmp_player_t *p, const wchar_t *mode);
{ return SUCCEEDED(player->put_uiMode(p, val); }

const wchar_t *wmp_player_get_uimode(wmp_player_t *p)
{
  BSTR val;
  return SUCCEEDED(player->get_uiMode(p, &val)) ? val : nullptr;
}

// Control

bool wmp_control_play(wmp_control_t *c) { return SUCCEEDED(c->play()); }
bool wmp_control_stop(wmp_control_t *c) { return SUCCEEDED(c->stop()); }
bool wmp_control_pause(wmp_control_t *c) { return SUCCEEDED(c->pause()); }

bool wmp_control_previous(wmp_control_t *c) { return SUCCEEDED(c->previous()); }
bool wmp_control_next(wmp_control_t *c) { return SUCCEEDED(c->next()); }

bool wmp_control_forward(wmp_control_t *c) { return SUCCEEDED(c->fastForward()); }
bool wmp_control_backward(wmp_control_t *c) { return SUCCEEDED(c->fastReverse()); }

bool wmp_control_set_pos(wmp_control_t *c, double val)
{ return SUCCEEDED(c->put_currentPosition(val)); }

double wmp_control_get_pos(wmp_control_t *c);
{
  double val;
  return SUCCEEDED(c->get_currentPosition(&val)) ? val : -1;
}

bool wmp_control_set_media(wmp_control_t *c, wmp_media_t *m);
{ return SUCCEEDED(c->put_currentItem(m)); }

wmp_media_t *wmp_control_get_media(wmp_control_t *c)
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

bool wmp_settings_set_baseurl(wmp_settings_t *s, const wchar_t *val);
{ return SUCCEEDED(s->put_baseUrl(t, val)); }

const wchar_t *wmp_settings_get_baseurl(wmp_settings_t *s)
{
  BSTR val;
  return SUCCEEDED(s->get_baseUrl(&val)) ? val : nullptr;
}

bool wmp_settings_set_volume(wmp_settings_t *s, int val)
{ return SUCCEEDED(s->put_volume(t, val)); }

int wmp_settings_get_volume(wmp_settings_t *s)
{
  long val;
  return SUCCEEDED(s->get_volume(&val)) ? val : -1;
}

bool wmp_settings_set_balance(wmp_settings_t *s, int val)
{ return SUCCEEDED(s->put_balance(t, val)); }

int wmp_settings_get_balance(wmp_settings_t *s)
{
  long val;
  return SUCCEEDED(s->get_balance(&val)) ? val : 0;
}

bool wmp_settings_set_mute(wmp_settings_t *s, bool t)
{ return SUCCEEDED(s->put_mute(t, t)); }

bool wmp_settings_get_mute(wmp_settings_t *s)
{
  VARIANT_BOOL t;
  return SUCCEEDED(s->get_mute(&t)) && t;
}

bool wmp_settings_set_rate(wmp_settings_t *s, double val)
{ return SUCCEEDED(s->put_rate(t, val)); }

double wmp_settings_get_rate(wmp_settings_t *s)
{
  double val;
  return SUCCEEDED(s->get_rate(&val)) ? val : -1;
}

bool wmp_settings_set_playcount(wmp_settings_t *s, int val);
{ return SUCCEEDED(s->put_playCount(t, val)); }

int wmp_settings_get_playcount(wmp_settings_t *s);
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
      || x && y && SUCCEEDED(s->get_isIdentical(y, &t)) && t;
}

double wmp_media_get_duration(wmp_media_t *m)
{
  double val;
  return SUCCEEDED(m->get_duration(&val)) ? val : -1;
}

bool wmp_media_set_name(wmp_media_t *m, const wchar_t *val)
{ return SUCCEEDED(m->put_name(&val)); }

const wchar_t *wmp_media_get_name(wmp_media_t *m);
{
  const wchar_t *val;
  return SUCCEEDED(m->get_name(&val)) ? val : nullptr;
}

int wmp_media_get_imagewidth(wmp_media_t *m)
{
  long val;
  return SUCCEEDED(m->get_imageSourceWidth(&val)) ? val : -1;
}

int wmp_media_get_imageheight(wmp_media_t *m);
{
  long val;
  return SUCCEEDED(m->get_imageSourceHeight(&val)) ? val : -1;
}

const wchar_t *wmp_media_get_url(wmp_media_t *m);
{
  const wchar_t *val;
  return SUCCEEDED(m->get_sourceUrl(&val)) ? val : nullptr;
}

// EOF
