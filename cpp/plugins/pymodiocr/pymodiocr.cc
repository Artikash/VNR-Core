// pymodiocr.cc
// 8/13/2014

#include "pymodiocr/pymodiocr.h"
#include "modiocr/modiocr.h"

bool ModiOcr::isValid()
{ return ::modiocr_available(); }

QStringList ModiOcr::readFile(const QString &path, int language)
{
  QStringList ret;
  ::modiocr_readfile((const wchar_t *)path.utf16(), language,
    [&ret](const wchar_t *text) {
      ret.append(QString::fromWCharArray(text));
    }
  );
  return ret;
}

// EOF
