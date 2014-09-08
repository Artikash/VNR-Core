// pymodiocr.cc
// 8/13/2014

#include "pymodiocr/pymodiocr.h"
#include "modiocr/modiocr.h"
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>

static QMutex _mutex; // MODI OCR would crash if invoked simutaneously

bool ModiOcr::isValid()
{ return ::modiocr_available(); }

QString ModiOcr::readText(const QString &path, int language)
{
  QMutexLocker sync(&::_mutex);
  QString ret;
  ::modiocr_readfile((const wchar_t *)path.utf16(), language,
    [&ret](const wchar_t *text) {
      ret.append(QString::fromWCharArray(text));
    }
  );
  return ret;
}

QStringList ModiOcr::readTextList(const QString &path, int language)
{
  QMutexLocker sync(&::_mutex);
  QStringList ret;
  ::modiocr_readfile((const wchar_t *)path.utf16(), language,
    [&ret](const wchar_t *text) {
      ret.append(QString::fromWCharArray(text));
    }
  );
  return ret;
}

// EOF
