// main.cc
// 6/15/2014 jichi
// See: http://guide2.project-cevio.com/interface/com
#include "ceviotts/ceviotts.h"
#include <qt_windows.h>
#include <QDebug>

int main()
{
  CoInitialize(nullptr);
  ceviotts_t *tts = ceviotts_create();
  ceviotts_destroy(tts);
  CoUninitialize();
  return 0;
}

// EOF
