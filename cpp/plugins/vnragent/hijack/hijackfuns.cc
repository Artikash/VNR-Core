// hijackfuns.cc
// 6/3/2015 jichi
#include "hijack/hijackfuns.h"
#include "util/dyncodec.h"

#define DEBUG "hijackfuns"
#include "sakurakit/skdebug.h"

// Disable only for debugging purpose
//#define HIJACK_GDI_FONT
#define HIJACK_GDI_TEXT

#define DEFINE_FUN(_f)      Hijack::_f##_fun_t Hijack::old##_f = ::_f;
  DEFINE_FUN(GetGlyphOutlineA)
  DEFINE_FUN(GetTextExtentPoint32A)
#undef DEFINE_FUN

/** Text */

DWORD WINAPI Hijack::newGetGlyphOutlineA(HDC hdc, UINT uChar, UINT uFormat, LPGLYPHMETRICS lpgm, DWORD cbBuffer, LPVOID lpvBuffer, const MAT2 *lpmat2)
{
#ifdef HIJACK_GDI_TEXT
  if (uChar > 0xff)
    if (auto p = DynamicCodec::instance()) {
      bool dynamic;
      UINT ch = p->decodeChar(uChar, &dynamic);
      if (dynamic && ch)
        return ::GetGlyphOutlineW(hdc, ch, uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2);
    }
#endif // HIJACK_GDI_TEXT
  return oldGetGlyphOutlineA(hdc, uChar, uFormat, lpgm, cbBuffer, lpvBuffer, lpmat2);
}

BOOL WINAPI Hijack::newGetTextExtentPoint32A(HDC hdc, LPCSTR lpString, int cchString, LPSIZE lpSize)
{
#ifdef HIJACK_GDI_TEXT
  //DOUT("pass");
  if(cchString > 1)
    if (auto p = DynamicCodec::instance()) {
      bool dynamic;
      QByteArray data(lpString, cchString);
      QString text = p->decode(data, &dynamic);
      if (dynamic && !text.isEmpty())
        return ::GetTextExtentPoint32W(hdc, (LPCWSTR)text.utf16(), text.size(), lpSize);
    }
#endif // HIJACK_GDI_TEXT
  return oldGetTextExtentPoint32A(hdc, lpString, cchString, lpSize);
}

// EOF

