// eninehijack_p.cc
// 4/30/2014 jichi
#include "engine/enginehijack_p.h"

int WINAPI Engine::MyMultiByteToWideChar(
  _In_       UINT CodePage,
  _In_       DWORD dwFlags,
  _In_       LPCSTR lpMultiByteStr,
  _In_       int cbMultiByte,
  _Out_opt_  LPWSTR lpWideCharStr,
  _In_       int cchWideChar
)
{
  return ::MultiByteToWideChar(CodePage, dwFlags, lpMultiByteStr, cbMultiByte, lpWideCharStr, cchWideChar);
}

// EOF
