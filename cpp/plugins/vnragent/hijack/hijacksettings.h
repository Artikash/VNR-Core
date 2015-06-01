#pragma once

// hijacksettings.h
// 5/23/2015 jichi
#include <QString>

class HijackSettings
{
public:
  QString fontFamily;       // font face
  float fontScale;          // zoom font width and height
  int fontWeight;           // fw font weight
  quint8 fontCharSet;       // font character set
  bool fontCharSetEnabled;  // whether modify font char set

  HijackSettings()
    : fontZoomFactor(0)
    , fontWeight(0)
    , fontCharSet(0),
    , fontCharSetEnabled(true)
  {}

  // true if fontScale is not zero
  bool isFontScaled() const { return !qFuzzyCompare(1, 1 + fontScale); }

  bool isFontCustomized() const
  {
    return fontCharSetEnabled
        || fontWeight
        || !fontFamily.isEmpty()
        || isFontScaled()
        ;
  }
};

// EOF
