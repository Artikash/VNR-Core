#pragma once

// wmpcli.h
// 10/1/2014 jichi

#ifdef _MSC_VER
# include <cstddef> // for wchar_t
#endif // _MSC_VER

// - Types -

struct IWMPControls;
struct IWMPMedia;
struct IWMPPlayer;
struct IWMPSettings;

typedef IWMPControls wmp_controls_t;
typedef IWMPMedia wmp_media_t;
typedef IWMPPlayer wmp_player_t;
typedef IWMPSettings wmp_settings_t;

// - Functions -

// Construction
wmp_player_t *wmp_player_create();
int wmp_player_release(wmp_player_t *p); // return reference count

wmp_controls_t *wmp_player_get_controls(wmp_player_t *p);
int wmp_controls_release(wmp_controls_t *c); // return reference count

wmp_settings_t *wmp_player_get_settings(wmp_player_t *p);
int wmp_settings_release(wmp_settings_t *s); // return reference count

int wmp_media_release(wmp_media_t *m); // return reference count

// Player
// http://msdn.microsoft.com/en-us/library/windows/desktop/dd563514%28v=vs.85%29.aspx

bool wmp_player_close(wmp_player_t *p);

inline void wmp_player_destroy(wmp_player_t *p)
{
  if (p) {
    wmp_player_close(p);
    wmp_player_release(p);
  }
}

bool wmp_player_set_enabled(wmp_player_t *p, bool t);
bool wmp_player_get_enabled(wmp_player_t *p);

bool wmp_player_set_fullscreen(wmp_player_t *p, bool t);
bool wmp_player_get_fullscreen(wmp_player_t *p);

bool wmp_player_set_contextmenuenabled(wmp_player_t *p, bool t);
bool wmp_player_get_contextmenuenabled(wmp_player_t *p);

// Not sure why this function is not on MSDN
bool wmp_player_set_url(wmp_player_t *p, const wchar_t *val);
const wchar_t *wmp_player_get_url(wmp_player_t *p);

// None, invisible, full, minimal
bool wmp_player_set_uimode(wmp_player_t *p, const wchar_t *val);
const wchar_t *wmp_player_get_uimode(wmp_player_t *p);

// Control
// http://msdn.microsoft.com/en-us/library/windows/desktop/dd563179%28v=vs.85%29.aspx

bool wmp_controls_play(wmp_controls_t *c);
bool wmp_controls_stop(wmp_controls_t *c);
bool wmp_controls_pause(wmp_controls_t *c);

bool wmp_controls_previous(wmp_controls_t *c);
bool wmp_controls_next(wmp_controls_t *c);

bool wmp_controls_forward(wmp_controls_t *c);
bool wmp_controls_backward(wmp_controls_t *c);

// Seconds from the beginning. Return -1 if failed.
bool wmp_controls_set_pos(wmp_controls_t *c, double val);
double wmp_controls_get_pos(wmp_controls_t *c);

bool wmp_controls_set_media(wmp_controls_t *c, wmp_media_t *val);
wmp_media_t *wmp_controls_get_media(wmp_controls_t *c);

// Settings
// http://msdn.microsoft.com/en-us/library/windows/desktop/dd563648%28v=vs.85%29.aspx

// Whether display a dialog on error
bool wmp_settings_set_errordialogsenabled(wmp_settings_t *s, bool t);
bool wmp_settings_get_errordialogsenabled(wmp_settings_t *s);

// Whether play on URL changes
bool wmp_settings_set_autostart(wmp_settings_t *s, bool t);
bool wmp_settings_get_autostart(wmp_settings_t *s);

// The prefix added to the URL
bool wmp_settings_set_baseurl(wmp_settings_t *s, const wchar_t *val);
const wchar_t *wmp_settings_get_baseurl(wmp_settings_t *s);

// [0,100] if succeed, or -1 if failed
enum { wmp_min_volume = 0, wmp_max_volume = 100 };
bool wmp_settings_set_volume(wmp_settings_t *s, int val);
int wmp_settings_get_volume(wmp_settings_t *s);

// [-100,100], default is 0
enum { wmp_min_balance = -100, wmp_max_balance = 100 };
bool wmp_settings_set_balance(wmp_settings_t *s, int val);
int wmp_settings_get_balance(wmp_settings_t *s);

// Whether has been muted
bool wmp_settings_set_mute(wmp_settings_t *s, bool t);
bool wmp_settings_get_mute(wmp_settings_t *s);

// Play speed, default is 1.0
bool wmp_settings_set_rate(wmp_settings_t *s, double val);
double wmp_settings_get_rate(wmp_settings_t *s);

// Number of times to play the same media. Default is 1. Return 0 if failed.
bool wmp_settings_set_playcount(wmp_settings_t *s, int val);
int wmp_settings_get_playcount(wmp_settings_t *s);

// Shuffle the order
bool wmp_settings_set_shuffle(wmp_settings_t *s, bool t);
bool wmp_settings_get_shuffle(wmp_settings_t *s);

// Repeat the tracks
bool wmp_settings_set_repeat(wmp_settings_t *s, bool t);
bool wmp_settings_get_repeat(wmp_settings_t *s);

// Play from the beginning when end is reached
bool wmp_settings_set_autorewind(wmp_settings_t *s, bool t);
bool wmp_settings_get_autorewind(wmp_settings_t *s);

// Media
// http://msdn.microsoft.com/en-us/library/windows/desktop/dd563397%28v=vs.85%29.aspx

// The two media are the same
bool wmp_media_equal(wmp_media_t *x, wmp_media_t *y);

// Return -1 if failed
double wmp_media_get_duration(wmp_media_t *m);

// Return nullptr if failed
const wchar_t *wmp_media_get_name(wmp_media_t *m);
bool wmp_media_set_name(wmp_media_t *m, const wchar_t *val);

// Return -1 if failed
int wmp_media_get_imagewidth(wmp_media_t *m);
int wmp_media_get_imageheight(wmp_media_t *m);

// Return nullptr if failed
const wchar_t *wmp_media_get_url(wmp_media_t *m);

// EOF
