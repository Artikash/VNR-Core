// rgss.cc
// 6/7/2015 jichi
#include "engine/model/rgss.h"
#include "engine/enginecontroller.h"
#include "engine/enginedef.h"
#include "engine/enginehash.h"
#include "engine/engineutil.h"
#include "hijack/hijackmanager.h"
#include "memdbg/memsearch.h"
#include "winhook/hookcode.h"
#include <qt_windows.h>
#include <boost/foreach.hpp>

#define DEBUG "rgss"
#include "sakurakit/skdebug.h"

//#pragma intrinsic(_ReturnAddress)

namespace { // unnamed

namespace RGSS3Hook {
namespace Private {

  QString getDllModuleName()
  {
    foreach (const QString &dll, Engine::glob("System/RGSS3*.dll"))
      if (::GetModuleHandleW((LPCWSTR)dll.utf16()))
        return dll;
    return QString();
  }

  struct HookArgument
  {
    LPDWORD unknown1,
            unknown2;
    size_t size;    // 0x8
    LPSTR text;     // 0xc
  };

  HookArgument *arg_;

  enum { MaxTextSize = 0x1000 };
  //char oldText_[MaxTextSize + 1]; // 1 extra 0 that is always 0
  //size_t oldSize_;

  /**
   * Skip ascii characters in the beginning
   *  @param  text
   *  @param  prefix
   *  @return
   */
  QString ltrim(const QString &text, QString *prefix = nullptr)
  {
    if (text.isEmpty() || text[0].unicode() > 127)
      return text;
    int pos = 1;
    for(; pos < text.size() && text[pos].unicode() <= 127; pos++);
    if (prefix)
      *prefix = text.left(pos);
    return text.mid(pos);
  }

  bool hookBefore(winhook::hook_stack *s)
  {
    auto arg = (HookArgument *)s->stack[1]; // arg1
    if (arg->size && arg->size < MaxTextSize && arg->text && Engine::isAddressWritable(arg->text, arg->size) && *arg->text) {
      QString oldText = QString::fromUtf8(arg->text),
              prefix,
              trimmedText = ltrim(oldText, &prefix);
      if (!trimmedText.isEmpty()) { // skip text beginning with ascii character
        enum { role = Engine::ScenarioRole, sig = Engine::ScenarioThreadSignature };
        //ulong split = arg->unknown2[0]; // always 2
        //ulong split = s->stack[0]; // return address
        //auto sig = Engine::hashThreadSignature(role, split);

        QString newText = EngineController::instance()->dispatchTextW(trimmedText, sig, role);
        if (newText != trimmedText) {
          if (!prefix.isEmpty())
            newText.prepend(prefix);
          QByteArray data = newText.toUtf8();
          //if (Engine::isAddressWritable(arg->text, data.size() + 1))

          //arg_ = arg;
          //oldSize_ = arg->size;
          //::memcpy(oldText_, arg->text, min(arg->size + 1, MaxTextSize));

          arg->size = min(data.size(), MaxTextSize - 1);
          ::memcpy(arg->text, data.constData(), arg->size + 1);
        }
      }
    }
    return true;
  }

  //bool hookAfter(winhook::hook_stack *)
  //{
  //  if (arg_) {
  //    arg_->size = oldSize_;
  //    ::strcpy(arg_->text, oldText_);
  //    arg_ = nullptr;
  //  }
  //  return true;
  //}

} // namespace Private

/**
 *  Sample game: Mogeko Castle with RGSS 3.01
 *
 *  1004149D   CC               INT3
 *  1004149E   CC               INT3
 *  1004149F   CC               INT3
 *  100414A0   8B4C24 08        MOV ECX,DWORD PTR SS:[ESP+0x8]
 *  100414A4   8BC1             MOV EAX,ECX
 *  100414A6   E8 75030500      CALL RGSS301.10091820
 *  100414AB   83F8 05          CMP EAX,0x5
 *  100414AE   74 19            JE SHORT RGSS301.100414C9
 *  100414B0   68 649D1A10      PUSH RGSS301.101A9D64                    ; ASCII "to_str"
 *  100414B5   68 74931A10      PUSH RGSS301.101A9374                    ; ASCII "String"
 *  100414BA   6A 05            PUSH 0x5
 *  100414BC   51               PUSH ECX
 *  100414BD   E8 AE2FFFFF      CALL RGSS301.10034470
 *  100414C2   83C4 10          ADD ESP,0x10
 *  100414C5   894424 08        MOV DWORD PTR SS:[ESP+0x8],EAX
 *  100414C9   53               PUSH EBX
 *  100414CA   55               PUSH EBP
 *  100414CB   56               PUSH ESI
 *  100414CC   8B7424 10        MOV ESI,DWORD PTR SS:[ESP+0x10]
 *  100414D0   57               PUSH EDI
 *  100414D1   8B7C24 18        MOV EDI,DWORD PTR SS:[ESP+0x18]
 *  100414D5   57               PUSH EDI
 *  100414D6   56               PUSH ESI
 *  100414D7   E8 B4490100      CALL RGSS301.10055E90
 *  100414DC   8BE8             MOV EBP,EAX
 *  100414DE   8B06             MOV EAX,DWORD PTR DS:[ESI]
 *  100414E0   83C4 08          ADD ESP,0x8
 *  100414E3   A9 00200000      TEST EAX,0x2000
 *  100414E8   75 08            JNZ SHORT RGSS301.100414F2
 *  100414EA   C1E8 0E          SHR EAX,0xE
 *  100414ED   83E0 1F          AND EAX,0x1F
 *  100414F0   EB 03            JMP SHORT RGSS301.100414F5
 *  100414F2   8B46 08          MOV EAX,DWORD PTR DS:[ESI+0x8]
 *  100414F5   8B0F             MOV ECX,DWORD PTR DS:[EDI]
 *  100414F7   F7C1 00200000    TEST ECX,0x2000
 *  100414FD   75 08            JNZ SHORT RGSS301.10041507
 *  100414FF   C1E9 0E          SHR ECX,0xE
 *  10041502   83E1 1F          AND ECX,0x1F
 *  10041505   EB 03            JMP SHORT RGSS301.1004150A
 *  10041507   8B4F 08          MOV ECX,DWORD PTR DS:[EDI+0x8]
 *  1004150A   8D3401           LEA ESI,DWORD PTR DS:[ECX+EAX]
 *  1004150D   A1 70C02A10      MOV EAX,DWORD PTR DS:[0x102AC070]
 *  10041512   50               PUSH EAX
 *  10041513   33FF             XOR EDI,EDI
 *  10041515   E8 B64EFFFF      CALL RGSS301.100363D0
 *  1004151A   8B5424 18        MOV EDX,DWORD PTR SS:[ESP+0x18] ; jichi: edx becomes arg1 on the stack
 *  1004151E   8BD8             MOV EBX,EAX
 *  10041520   8B02             MOV EAX,DWORD PTR DS:[EDX]
 *  10041522   8BC8             MOV ECX,EAX
 *  10041524   83C4 04          ADD ESP,0x4
 *  10041527   81E1 00200000    AND ECX,0x2000
 *  1004152D   75 08            JNZ SHORT RGSS301.10041537
 *  1004152F   C1E8 0E          SHR EAX,0xE
 *  10041532   83E0 1F          AND EAX,0x1F
 *  10041535   EB 03            JMP SHORT RGSS301.1004153A
 *  10041537   8B42 08          MOV EAX,DWORD PTR DS:[EDX+0x8]
 *  1004153A   85C9             TEST ECX,ECX
 *  1004153C   75 05            JNZ SHORT RGSS301.10041543
 *  1004153E   83C2 08          ADD EDX,0x8
 *  10041541   EB 03            JMP SHORT RGSS301.10041546
 *  10041543   8B52 0C          MOV EDX,DWORD PTR DS:[EDX+0xC] ; jichi: edx + 0xc could be the text address
 *  10041546   F703 00200000    TEST DWORD PTR DS:[EBX],0x2000
 *  1004154C   8D4B 08          LEA ECX,DWORD PTR DS:[EBX+0x8]
 *  1004154F   74 03            JE SHORT RGSS301.10041554
 *  10041551   8B4B 0C          MOV ECX,DWORD PTR DS:[EBX+0xC]
 *  10041554   50               PUSH EAX
 *  10041555   52               PUSH EDX
 *  10041556   51               PUSH ECX
 *  10041557   E8 E4F21300      CALL RGSS301.10180840   ; jichi: text is in edx
 *  1004155C   8B5424 24        MOV EDX,DWORD PTR SS:[ESP+0x24]
 *  10041560   8B02             MOV EAX,DWORD PTR DS:[EDX]
 *  10041562   8BC8             MOV ECX,EAX
 *  10041564   83C4 0C          ADD ESP,0xC
 *  10041567   81E1 00200000    AND ECX,0x2000
 *  1004156D   75 08            JNZ SHORT RGSS301.10041577
 *
 *  Stack:
 *  00828EB4   1002E5E6  RETURN to RGSS301.1002E5E6 from RGSS301.100414A0
 *  00828EB8   03F13B20
 *  00828EBC   069F42CC
 *  00828EC0   00000000
 *  00828EC4   01699298
 *  00828EC8   01699298
 *  00828ECC   03EB41B8
 *  00828ED0   01692A00
 *  00828ED4   06A34548
 *  00828ED8   00000000
 *  00828EDC   00000168
 *  00828EE0   00000280
 *  00828EE4   000001E0
 *  00828EE8   1019150F  RETURN to RGSS301.1019150F from RGSS301.1018DF45
 */

bool attach() // attach scenario
{
  QString module = Private::getDllModuleName();
  if (module.isEmpty())
    return false;

  DOUT("dll =" << module);

  ulong startAddress, stopAddress;
  if (!Engine::getMemoryRange((LPCWSTR)module.utf16(), &startAddress, &stopAddress))
    return false;

  const quint8 bytes[] = {
    0x8b,0x54,0x24, 0x24,           // 1004155c   8b5424 24        mov edx,dword ptr ss:[esp+0x24]
    0x8b,0x02,                      // 10041560   8b02             mov eax,dword ptr ds:[edx]
    0x8b,0xc8,                      // 10041562   8bc8             mov ecx,eax
    0x83,0xc4, 0x0c,                // 10041564   83c4 0c          add esp,0xc
    0x81,0xe1, 0x00,0x20,0x00,0x00  // 10041567   81e1 00200000    and ecx,0x2000
  };
  ulong addr = MemDbg::findBytes(bytes, sizeof(bytes), startAddress, stopAddress);
  if (!addr)
    return false;
  addr = MemDbg::findEnclosingAlignedFunction(addr);
  if (!addr)
    return false;
  //addr = MemDbg::findPushAddress(addr, startAddress, stopAddress);
  //addr = 0x10041557;
  //addr = 0x100414a0;
  return winhook::hook_before(addr, Private::hookBefore); //, Private::hookAfter);
}

} // namespace AgsHookW

} // unnamed namespace

/** Public class */

bool RGSSEngine::attach()
{
  if (!RGSS3Hook::attach())
    return false;
  HijackManager::instance()->attachFunction((ulong)::GetGlyphOutlineW); // in order to customize font
  return true;
}

QString RGSSEngine::textFilter(const QString &text, int role)
{
  Q_UNUSED(role);
  QString ret = text;
  ret = ret.remove("\\.");
  return ret;
}

// EOF
/*
  //typedef char *(* hook_fun_t)(char *dst, const char *src, size_t size);
  //hook_fun_t oldHookFun;
  char *oldHookFun(char *dst, const char *src, size_t size)
  {
    ::memcpy(dst, src, size);
    if (src[size] == 0)
      dst[size] = 0;
    return dst;
  }

  // Similar to strncpy
  // Copy size of src to dst and append '\0' at the end
  // Return dst
  char *newHookFun(char *dst, const char *src, size_t size)
  {
    if (size) {
      QString oldText = QString::fromUtf8(src, size);
      if (oldText[0].unicode() >= 128) {

        enum { role = Engine::OtherRole };
        ulong split = (ulong)_ReturnAddress();
        auto sig = Engine::hashThreadSignature(role, split);

        QString newText = EngineController::instance()->dispatchTextW(oldText, sig, role);
        if (newText != oldText) {
          static QByteArray data;
          data = newText.toUtf8();
          return oldHookFun(dst, data.constData(), data.size());
        }
      }
    }
    return oldHookFun(dst, src, size);
  }
*/
