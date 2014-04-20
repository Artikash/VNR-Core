#pragma once
// myd3d.h
// 4/19/2014 jichi

// EOF

/*
 * TODO
// - Test -

#include <d3d9.h>
#include <d3dx9.h>
#include <detours.h>

typedef LPDIRECT3D9 (WINAPI *D3DCreate9) (UINT SDKVersion);
D3DCreate9 pD3DCreate9 = (D3DCreate9)::GetProcAddress(::LoadLibraryA("d3d9.dll"), "Direct3DCreate9");
IDirect3D9 *d3d;
LPDIRECT3D9 WINAPI nD3DCreate9(UINT SDKVersion) {
  d3d = pD3DCreate9(SDKVersion);
  ::MessageBoxA(nullptr, "d3d", "Error", MB_OK);
  return d3d;
}

DWORD d3dhook(LPVOID lpThreadParameter)
{
  Q_UNUSED(lpThreadParameter);
  //pD3DCreate9 = (D3DCreate9)DetourFunction((BYTE*)GetProcAddress(LoadLibraryA("d3d9.dll"), "Direct3DCreate9"), (BYTE*)nD3DCreate9);

  ::DetourRestoreAfterWith();
  ::DetourTransactionBegin();
  ::DetourUpdateThread(::GetCurrentThread());
  ::DetourAttach((PVOID *)&pD3DCreate9, nD3DCreate9);
  ::DetourTransactionCommit();
  return 0;
}
*/
