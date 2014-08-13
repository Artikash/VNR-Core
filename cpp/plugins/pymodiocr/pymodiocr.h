#ifndef PYMODIOCR_H
#define PYMODIOCR_H

// pymodiocr.h
// 8/13/2014 jichi

#include <QtCore/QStringList>

// This class is supposed to be a singleton.
// All methods here are stateless, and supposed to be static.
class ModiOcr
{
public:
  ///  Return whether MODI is available
  static bool isValid();

  /**  Read image at path with MODI language and return list of string
   *  @param  path  image path, could be either TIFF or JPEG
   *  @param  language  language enum defined in modiocr.h
   */
  static QStringList readFile(const QString& path, int language);
};

#endif // PYMODIOCR_H
