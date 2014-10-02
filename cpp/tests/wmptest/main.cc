// main.cc
// 10/1/2014 jichi
// http://forums.codeguru.com/showthread.php?471953-IWMPPlayer%284%29-COM-issues
// http://blog.firefly-vj.net/blog/2008/05/15/directshow-windowsmobile-play-mp3-windows-media-player/
//#include "winmp/winmp.h"
#include <windows.h>
//#include <wmp.h>
#include <QtCore>

//#import "wmp.dll" no_namespace named_guids high_method_prefix( "I" )
//#import "wmp.dll"
// http://social.msdn.microsoft.com/Forums/vstudio/ko-KR/db3007bd-0410-45a0-bab0-6b0a20723f14/windows-media-playermfcatl?forum=visualcpluszhchs
#import "wmp.dll" raw_interfaces_only raw_native_types no_namespace named_guids

int main(int argc, char *argv[])
{
//#define URL L"http://translate.google.com/translate_tts?tl=ja&q=hello"
#define URL L"http://tts.baidu.com/text2audio?lan=jp&pid=101&ie=UTF-8&text=hello"
  ::CoInitialize(NULL);
  IWMPPlayer *player;
  HRESULT ok = ::CoCreateInstance(CLSID_WindowsMediaPlayer, NULL, CLSCTX_INPROC_SERVER, IID_IWMPPlayer, (void**)&player);
  if (SUCCEEDED(ok)) {
    player->put_enabled(VARIANT_TRUE);
    IWMPControls *ctrl;
    ok = player->get_controls(&ctrl);
    qDebug() << ok;
    if (SUCCEEDED(ok)) {
      IWMPSettings *settings;
      player->get_settings(&settings);
      qDebug() << settings->put_volume(100);
      qDebug() << settings->put_autoStart(VARIANT_TRUE);
      BSTR url = ::SysAllocString(URL);
      qDebug() << player->put_URL(url);
      ::SysFreeString(url);
      qDebug() << ctrl->play();
      qDebug() << ::GetLastError();
      //Sleep(2000);
      system("pause");
      //QCoreApplication a(argc, argv);
      //a.exec();
      settings->Release();
    }
    ctrl->Release();
  }
  player->close();
  player->Release();
  ::CoUninitialize();
  return 0;
}

// EOF
