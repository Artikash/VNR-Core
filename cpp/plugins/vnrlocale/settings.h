#pragma once

// settings.h
// 5/15/2014 jichi

struct Settings
{
  unsigned int codePage; // target code page

  bool overridesGetSystemDefaultLangID; // needed by BGI, see: http://i.watashi.me/archives/1.html

  Settings()
    : codePage(0)
    , overridesGetSystemDefaultLangID(true)
  {}
};

// EOF
