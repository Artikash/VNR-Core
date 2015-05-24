#pragma once

// hijacksettings.h
// 5/23/2015 jichi
#include <QString>

class HijackSettings
{
public:
  QString fontFamily; // font face
  quint8 fontCharSet; // font character set
  bool fontCharSetEnabled; // whether modify font char set

  HijackSettings() : fontCharSet(0) {}
};

// EOF
