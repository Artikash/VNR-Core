#pragma once

// mousehook.h
// 11/26/2011
#include <boost/function.hpp>

// Global instance

void mousehook_start();
void mousehook_stop();
bool mousehook_active();

///  Return true if eat the event
typedef boost::function<bool (long x, long y, void *hwnd)> mousehook_fun_t;

void mousehook_onmove(mousehook_fun_t callback);
void mousehook_onlbuttonup(mousehook_fun_t callback);
void mousehook_onlbuttondown(mousehook_fun_t callback);

// EOF
