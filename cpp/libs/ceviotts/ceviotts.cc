// ceviotts.cc
// 6/15/2014 jichi
// http://guide2.project-cevio.com/interface/com

#include "ceviotts/ceviotts.h"
#include "cevio/cevio.h"
#include "cc/ccmacro.h"

//#import "libid:D3AEA482-B527-4818-8CEA-810AFFCB24B6" named_guids rename_namespace("CeVIO")

ceviotts_t *ceviotts_create()
{
  CeVIO::ITalker *ret = nullptr;
  ::CoCreateInstance(CeVIO::CLSID_ITalker, nullptr, CLSCTX_INPROC_SERVER, CeVIO::IID_ITalker,
                     reinterpret_cast<LPVOID *>(&ret));
  return ret;
}

void ceviotts_destroy(ceviotts_t *talker)
{
  // FIXME: voice's token is not destroyed?
  if (CC_LIKELY(talker))
    talker->Release();
}

bool ceviotts_speak(ceviotts_t *talker, const wchar_t *sentence, unsigned long flags)
{
  //return talker && SUCCEEDED(voice->Speak(sentence, flags, nullptr));
  return false;
}

// EOF

//bool wintts_set_voice(wintts_t *voice, const wchar_t *reg)
//{
//  if (CC_UNLIKELY(!voice || !reg))
//    return false;
//
//  enum { fCreateIfNotExist = FALSE };
//  ISpObjectToken *token;
//  if (SUCCEEDED(::CoCreateInstance(CLSID_SpObjectToken, nullptr, CLSCTX_ALL, IID_ISpObjectToken,
//          reinterpret_cast<LPVOID *>(&token)))) {
//    if (SUCCEEDED(token->SetId(nullptr, reg, fCreateIfNotExist))) {
//      voice->SetVoice(token);
//      return true;
//    }
//    token->Release();
//  }
//  return false;
//}

/*
// タイプライブラリインポート
// （タイプライブラリの登録は、【CeVIO Creative Studio】インストール時に行われます。）
#import "libid:D3AEA482-B527-4818-8CEA-810AFFCB24B6" named_guids rename_namespace("CeVIO")

int _tmain(int argc, _TCHAR* argv[])
{

  // COM初期化
  ::CoInitialize(NULL);

  // Talkerインスタンス生成
  CeVIO::ITalker *pTalker;
  HRESULT result = ::CoCreateInstance(CeVIO::CLSID_Talker,
    NULL,
    CLSCTX_INPROC_SERVER,
    CeVIO::IID_ITalker,
    reinterpret_cast<LPVOID *>(&pTalker));
  if (FAILED(result)) {
    // 失敗
    ::CoUninitialize();
    return 0;
  }

  // キャスト設定
  pTalker->Cast = "さとうささら";

  // （例）音量設定
  pTalker->Volume = 100;

  // （例）再生
  CeVIO::ISpeakingStatePtr pState = pTalker->Speak("こんにちは");

  // （例）再生終了を待つ
  pState->Wait();

  // （例）音素データ取得
  CeVIO::IPhonemeDataArrayPtr pPhonemes = pTalker->GetPhonemes("はじめまして");

  // Talker解放
  pTalker->Release();

  // COM使用終了
  ::CoUninitialize();

  return 0;
}
*/
