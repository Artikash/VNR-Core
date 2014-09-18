// main.cc
// 9/17/2014 jichi
#include <windows.h>
#include <iostream>
#include <string>

extern "C" {
typedef DWORD (__stdcall *StartSession)(wchar_t *path, void *bufferStart, void *bufferStop, const wchar_t *app);
typedef DWORD (__stdcall *OpenEngine)(int bufferSize);
typedef DWORD (__stdcall *SimpleTransSentM)(int, const wchar_t *fr, wchar_t *t, int, int);
}

int main()
{
  wchar_t path[] = L"C:\\tmp\\FASTAIT_PERSONAL\\GTS\\JapaneseSChinese\\DCT";
  std::cerr << "enter" << std::endl;
  HMODULE h = ::LoadLibraryA("JPNSCHSDK.dll");
  std::cerr << h << std::endl;
  enum { key = 0x4f4 };
  if (h) {
    StartSession startSession = (StartSession)::GetProcAddress(h, "StartSession");
    OpenEngine openEngine = (OpenEngine)::GetProcAddress(h, "OpenEngine");
    SimpleTransSentM simpleTransSentM = (SimpleTransSentM)::GetProcAddress(h, "SimpleTransSentM");

    enum { bufferSize = key };
    char buffer[bufferSize];

    int ret = startSession(path, buffer, buffer + bufferSize, L"DCT");

    ret = openEngine(key);

    wchar_t fr[] = L"こんにちは";
    wchar_t to[0x400] = {};
    ret = simpleTransSentM(key, fr, to, 0x28, 0x4);

    std::cerr << ret << std::endl;
    std::cerr << fr[0] << ":" << wcslen(fr) << std::endl;
    std::cerr << to[0] << ":" << wcslen(to) << std::endl;
  }
  std::cerr << "leave" << std::endl;
  return 0;
}

// EOF
