// main.cc
// 6/15/2014 jichi
#include "modiocr/modiocr.h"
#include <qt_windows.h>
#include <QtCore/QDebug>
#include <QtCore/QString>

int main()
{
  const wchar_t *path = L"test.tiff";

  CoInitialize(nullptr);
  if (modiocr_available())
    modiocr_from_file(path, [](const wchar_t *text) {
      qDebug() << QString::fromWCharArray(text);
    });

  CoUninitialize();
  return 0;
}

// EOF
