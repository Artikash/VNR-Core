// main.cc
// 10/1/2014 jichi
// http://msdn.microsoft.com/en-us/library/windows/desktop/dd563023%28v=vs.85%29.aspx
// http://stackoverflow.com/questions/21699887/iaxwinhostwindow-createcontrol-returns-e-nointerface-when-trying-to-host-wmp-wit
// http://forums.codeguru.com/showthread.php?471953-IWMPPlayer%284%29-COM-issues
// http://blog.firefly-vj.net/blog/2008/05/15/directshow-windowsmobile-play-mp3-windows-media-player/
//#include "winmp/winmp.h"
#include <windows.h>
//#include <wmp.h>
#include <QtCore>
#include "wmp/wmp.h"

//#import "wmp.dll" no_namespace named_guids high_method_prefix( "I" )
//#import "wmp.dll"
// http://social.msdn.microsoft.com/Forums/vstudio/ko-KR/db3007bd-0410-45a0-bab0-6b0a20723f14/windows-media-playermfcatl?forum=visualcpluszhchs
//#import "wmp.dll" raw_interfaces_only raw_native_types no_namespace named_guids

int main()
{
#define URL L"http://translate.google.com/translate_tts?tl=ja&q=hello"
//#define URL L"http://tts.baidu.com/text2audio?lan=jp&pid=101&ie=UTF-8&text=hello"
//#define URL L"Z:\\Users\\jichi\\tmp\\test.mp3"

  // Require CoInitializeEX MTA
  // http://stackoverflow.com/questions/21699887/iaxwinhostwindow-createcontrol-returns-e-nointerface-when-trying-to-host-wmp-wit
  //::CoInitialize(NULL); // This won't work
  ::CoInitializeEx(NULL, COINIT_MULTITHREADED);

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
      long vol;
      qDebug() << settings->get_volume(&vol);
      qDebug() << vol;
      qDebug() << settings->put_autoStart(VARIANT_TRUE);
      //qDebug() << player->put_uiMode(L"full");
      //qDebug() << player->put_uiMode(L"invisible");
      //qDebug() << player->put_uiMode(L"none");
      qDebug() << player->put_URL(URL);
      //qDebug() << ctrl->play();
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
