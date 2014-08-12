// main.cc
// 6/15/2014 jichi
#include "modiocr/modiocr.h"
#include <qt_windows.h>
#include <QtCore/QDebug>
#include <QtCore/QString>

int main()
{
  qDebug() << "enter";
  const wchar_t *path = L"test.tiff";

  modiocr_flags langs = modiocr_lang_ja|modiocr_lang_zhs|modiocr_lang_zht|modiocr_lang_en;

  CoInitialize(nullptr);
  if (modiocr_available()) {
    modiocr_lang lang = modiocr_from_file(path, langs, [](const wchar_t *text) {
      qDebug() << QString::fromWCharArray(text);
    });

    qDebug() << (int)lang;
  }

  CoUninitialize();
  qDebug() << "leave";
  return 0;
}

// EOF
