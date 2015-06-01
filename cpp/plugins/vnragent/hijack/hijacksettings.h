#pragma once

// hijacksettings.h
// 5/23/2015 jichi
#include <QString>

class HijackSettings
{
public:
  QString fontFamily;       // font face
  quint8 fontCharSet;       // font character set
  bool fontCharSetEnabled;  // whether modify font char set
  qreal fontScaleFactor;    // zoom font width and height
  int fontWeight;           // fw font weight

  HijackSettings() : fontCharSet(0), fontZoomFactor(0), fontWeight(0) {}

  // true if fontScaleFactor is not zero
  bool isFontScaled() const { return !qFuzzyCompare(1, 1 + fontScaleFactor); }

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
