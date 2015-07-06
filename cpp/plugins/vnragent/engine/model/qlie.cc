// qlie.cc
// 7/5/2015 jichi
#include "engine/model/qlie.h"
#include "engine/enginecontroller.h"
#include "engine/enginedef.h"
#include "engine/enginehash.h"
#include "engine/engineutil.h"
#include "hijack/hijackmanager.h"
#include "memdbg/memsearch.h"
#include "util/textutil.h"
#include "winhook/hookcode.h"
#include <qt_windows.h>
#include <cstdint>

#define DEBUG "model/qlie"
#include "sakurakit/skdebug.h"

/** Private data */

namespace { // unnamed

namespace ScenarioHook {
namespace Private {

  LPCSTR trim(LPCSTR text, int *size)
  {
    int length = ::strlen(text);
    if (text[0] == '[') {
      if (Util::allAscii(text))
        return nullptr;
      if (text[length - 1] == ']' && ::CharPrevA(text, text + length) == text + length - 1)
        length--;
      for (int i = 1; i < length; i++)
        if ((signed char)text[i] <= 0) {
          text += i - 1;
          break;
        }
    }
    if (size)
      *size = length;
    return text;
  }

  /**
   *  Sample game: 月に寄りそう乙女の作法２
   *
   *
   *  Name:
   *
   *  019D7688  5B 66 2C 31 5D 5B 72 66 2C 31 5D 5B 73 2C 32 30  [f,1][rf,1][s,20
   *  019D7698  2C 32 30 5D 5B 63 2C 24 46 46 46 46 46 46 46 46  ,20][c,$FFFFFFFF
   *  019D76A8  5D 5B 72 63 2C 24 46 46 46 46 46 46 46 46 5D 81  ][rc,$FFFFFFFF]・
   *  019D76B8  79 8D F7 8F AC 98 48 83 41 83 67 83 8C 81 7A 00  y桜小路アトレ】.
   *
   *  0012FBCC   0055553D  RETURN to .0055553D from .00513234
   *  0012FBD0   0012FDB8  Pointer to next SEH record
   *  0012FBD4   005555A5  SE handler
   *  0012FBD8   0012FD90
   *  0012FBDC   0E9F72D0
   *  0012FBE0   0E9F72D0
   *  0012FBE4   0A24AA90
   *  0012FBE8   00000000
   *  0012FBEC   00000000
   *  0012FBF0   0C7AE0C8  ASCII "st+cc+tt"
   *  0012FBF4   00000000
   *  0012FBF8   00000000
   *  0012FBFC   00000000
   *  0012FC00   00000000
   *  0012FC04   00000000
   *  0012FC08   00000000
   *
   *  EAX 0E3885A0
   *  ECX 00000002
   *  EDX 019D7688
   *  EBX 0041D17C .0041D17C
   *  ESP 0012FBCC
   *  EBP 0012FD90
   *  ESI 0A24AA90
   *  EDI 0E9F72D0
   *  EIP 00513234 .00513234
   *
   *  Scenario:
   *
   *  058DC708  5B 66 2C 30 5D 5B 72 66 2C 30 5D 5B 73 2C 32 34  [f,0][rf,0][s,24
   *  058DC718  2C 32 34 5D 5B 63 2C 24 46 46 46 46 46 46 46 46  ,24][c,$FFFFFFFF
   *  058DC728  5D 5B 72 63 2C 24 46 46 46 46 46 46 46 46 5D 81  ][rc,$FFFFFFFF]・
   *  058DC738  75 82 CD 82 A2 81 41 82 B1 82 B1 82 CD 93 FA 96  uはい、ここは日・
   *  058DC748  7B 82 C5 82 B7 81 42 8B F3 8D 60 82 CC 90 45 88  {です。空港の職・
   *  058DC758  F5 82 E0 81 41 83 56 83 87 83 62 83 76 82 CC 93  焉Aショップの・
   *  058DC768  58 88 F5 82 E0 81 41 83 8D 83 72 81 5B 82 C9 8D  X員も、ロビーに・
   *  058DC778  C0 82 E9 90 6C 82 E0 81 41 93 FA 96 7B 90 6C 82  ﾀる人も、日本人・
   *  058DC788  E7 82 B5 82 AB 90 6C 82 CE 82 A9 82 E8 82 C5 82  轤ｵき人ばかりで・
   *  058DC798  B7 81 76 00 00 8E 8D 05 01 00 00 00 8C 00 00 00  ｷ」..詩...・..
   *  058DC7A8  81 75 8D A1 93 FA 82 CD 90 E2 8D 44 82 CC 93 DC  「今日は絶好の曇
   *  058DC7B8  82 E8 8B F3 82 BE 82 E6 81 41 82 C8 82 F1 82 C4  り空だよ、なんて
   *  058DC7C8  91 66 93 47 82 C8 96 E9 8B F3 82 BE 82 EB 82 A4  素敵な夜空だろう
   *  058DC7D8  81 49 81 40 96 6C 82 CC 8B 41 8D 91 82 C9 8D 87  ！　僕の帰国に合
   *  058DC7E8  82 ED 82 B9 82 C4 91 BE 97 7A 82 F0 89 42 82 B5  わせて太陽を隠し
   *
   *  0012FBCC   0055553D  RETURN to .0055553D from .00513234
   *  0012FBD0   0012FDB8  Pointer to next SEH record
   *  0012FBD4   005555A5  SE handler
   *  0012FBD8   0012FD90
   *  0012FBDC   0E9F7110
   *  0012FBE0   0E9F7110
   *  0012FBE4   0A24AA90
   *  0012FBE8   00000000
   *  0012FBEC   00000000
   *  0012FBF0   0EA33460  ASCII "st+cc+tt"
   *  0012FBF4   00000000
   *  0012FBF8   00000000
   *  0012FBFC   00000000
   *  0012FC00   00000000
   *
   *  EAX 0E9AD230
   *  ECX 00000002
   *  EDX 058DC708
   *  EBX 0041D17C .0041D17C
   *  ESP 0012FBCC
   *  EBP 0012FD90
   *  ESI 0A24AA90
   *  EDI 0E9F7110
   *  EIP 00513234 .00513234
   */
  bool hookBefore(winhook::hook_stack *s)
  {
    static QByteArray data_; // persistent storage, which makes this function not thread-safe
    auto text = (LPCSTR)s->edx; // text in arg1
    if (!text || !*text)
      return true;
    int trimmedSize = 0;
    LPCSTR trimmedText = trim(text, &trimmedSize);
    if (!trimmedSize || !trimmedText || !*trimmedText)
      return true;
    auto role = Engine::ScenarioRole;
    auto split = s->stack[0]; // retaddr
    auto sig = Engine::hashThreadSignature(role, split);
    QByteArray oldData(trimmedText, trimmedSize),
               newData = EngineController::instance()->dispatchTextA(oldData, sig, role);
    if (newData == oldData)
      return true;
    if (trimmedText != text)
      newData.prepend(text, trimmedText - text);
    if (trimmedText[trimmedSize])
      newData.append(trimmedText + trimmedSize, ::strlen(trimmedText) - trimmedSize);
    //s->edx = (ulong)data_.constData(); // reset arg1
    return true;
  }
} // namespace Private

/**
 *  Sample game: 月に寄りそう乙女の作法２
 *  See: http://capita.tistory.com/m/post/236
 *
 *  This function is not aligned.
 *  Text in edx.
 *
 *  00513234   55               PUSH EBP
 *  00513235   8BEC             MOV EBP,ESP
 *  00513237   6A 00            PUSH 0x0
 *  00513239   53               PUSH EBX
 *  0051323A   56               PUSH ESI
 *  0051323B   8BF2             MOV ESI,EDX
 *  0051323D   8BD8             MOV EBX,EAX
 *  0051323F   33C0             XOR EAX,EAX
 *  00513241   55               PUSH EBP
 *  00513242   68 AD325100      PUSH .005132AD
 *  00513247   64:FF30          PUSH DWORD PTR FS:[EAX]
 *  0051324A   64:8920          MOV DWORD PTR FS:[EAX],ESP
 *  0051324D   80BB 0A160000 00 CMP BYTE PTR DS:[EBX+0x160A],0x0 ; jichi: instruction used as pattern
 *  00513254   74 07            JE SHORT .0051325D
 *  00513256   8BC3             MOV EAX,EBX
 *  00513258   8B10             MOV EDX,DWORD PTR DS:[EAX]
 *  0051325A   FF52 24          CALL DWORD PTR DS:[EDX+0x24]
 *  0051325D   8BC3             MOV EAX,EBX
 *  0051325F   E8 98C1FFFF      CALL .0050F3FC
 *  00513264   84C0             TEST AL,AL
 *  00513266   74 07            JE SHORT .0051326F
 *  00513268   8BC3             MOV EAX,EBX
 *  0051326A   8B10             MOV EDX,DWORD PTR DS:[EAX]
 *  0051326C   FF52 24          CALL DWORD PTR DS:[EDX+0x24]
 *  0051326F   8D4D FC          LEA ECX,DWORD PTR SS:[EBP-0x4]
 *  00513272   8BD6             MOV EDX,ESI
 *  00513274   8BC3             MOV EAX,EBX
 *  00513276   E8 5D310000      CALL .005163D8
 *  0051327B   8B55 FC          MOV EDX,DWORD PTR SS:[EBP-0x4]
 *  0051327E   8BC3             MOV EAX,EBX
 *  00513280   E8 1B100000      CALL .005142A0
 *  00513285   8BC3             MOV EAX,EBX
 *  00513287   E8 5C300000      CALL .005162E8
 *  0051328C   85C0             TEST EAX,EAX
 *  0051328E   75 07            JNZ SHORT .00513297
 *  00513290   8BC3             MOV EAX,EBX
 *  00513292   E8 B1070000      CALL .00513A48
 *  00513297   33C0             XOR EAX,EAX
 *  00513299   5A               POP EDX
 *  0051329A   59               POP ECX
 *  0051329B   59               POP ECX
 *  0051329C   64:8910          MOV DWORD PTR FS:[EAX],EDX
 *  0051329F   68 B4325100      PUSH .005132B4
 *  005132A4   8D45 FC          LEA EAX,DWORD PTR SS:[EBP-0x4]
 *  005132A7   E8 F421EFFF      CALL .004054A0
 *  005132AC   C3               RETN
 *  005132AD  ^E9 A21AEFFF      JMP .00404D54
 *  005132B2  ^EB F0            JMP SHORT .005132A4
 *  005132B4   5E               POP ESI
 *  005132B5   5B               POP EBX
 *  005132B6   59               POP ECX
 *  005132B7   5D               POP EBP
 *  005132B8   C3               RETN
 *  005132B9   8D40 00          LEA EAX,DWORD PTR DS:[EAX]
 *  005132BC   55               PUSH EBP
 *  005132BD   8BEC             MOV EBP,ESP
 *  005132BF   8B45 08          MOV EAX,DWORD PTR SS:[EBP+0x8]
 *  005132C2   8B40 FC          MOV EAX,DWORD PTR DS:[EAX-0x4]
 *  005132C5   80B8 6F180000 00 CMP BYTE PTR DS:[EAX+0x186F],0x0
 *  005132CC   74 23            JE SHORT .005132F1
 *  005132CE   A1 C8EA5700      MOV EAX,DWORD PTR DS:[0x57EAC8]
 *  005132D3   8B80 FC020000    MOV EAX,DWORD PTR DS:[EAX+0x2FC]
 *  005132D9   8B15 C8EA5700    MOV EDX,DWORD PTR DS:[0x57EAC8]          ; .00586178
 *  005132DF   8B92 E8020000    MOV EDX,DWORD PTR DS:[EDX+0x2E8]
 *  005132E5   3BD0             CMP EDX,EAX
 *  005132E7   7C 02            JL SHORT .005132EB
 *  005132E9   8BC2             MOV EAX,EDX
 *  005132EB   0105 B8E45700    ADD DWORD PTR DS:[0x57E4B8],EAX
 *  005132F1   5D               POP EBP
 *  005132F2   C3               RETN
 *  005132F3   90               NOP
 *  005132F4   55               PUSH EBP
 *  005132F5   8BEC             MOV EBP,ESP
 *  005132F7   53               PUSH EBX
 *  005132F8   8B5D 08          MOV EBX,DWORD PTR SS:[EBP+0x8]
 *  ...
 *
 *  {00528988(E9 73 FC 04 00 90),00578600(8D 45 FC 8B 4D FC 66 81 39 81 79 74 05 90 90 90 90 90 E9 77 03 FB FF)}
 *  {00528988(E9 73 FC 04 00 90),005785FE(EB 27 8D 45 FC 8B 4D FC 66 81 39 81 79 74 0A 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 90 68 8E 89 52 00 C3)}
 *
 *  FORCEFONT(5),FONT(Gulim,-13),ENCODEKOR,HOOK(0x00513234,TRANS(EDX,LEN(-4),PTRCHEAT),RETNPOS(COPY)),HOOK(0x0057860D,TRANS(ECX,LEN(-4),PTRCHEAT),RETNPOS(SOURCE))
 *
 *  Character handled here, which is not used:
 *  00528969   74 28            JE SHORT .00528993
 *  0052896B   3C 09            CMP AL,0x9
 *  0052896D   74 24            JE SHORT .00528993
 *  0052896F   3C 2F            CMP AL,0x2F
 *  00528971   74 20            JE SHORT .00528993
 *  00528973   3C 40            CMP AL,0x40
 *  00528975   74 1C            JE SHORT .00528993
 *  00528977   8D45 E8          LEA EAX,DWORD PTR SS:[EBP-0x18]
 *  0052897A   8D93 49010000    LEA EDX,DWORD PTR DS:[EBX+0x149]
 *  00528980   E8 7FCDEDFF      CALL .00405704
 *  00528985   8B55 E8          MOV EDX,DWORD PTR SS:[EBP-0x18]
 *  00528988   8D45 FC          LEA EAX,DWORD PTR SS:[EBP-0x4]  ; jichi: 2-byte character in ecx
 *  0052898B   8B4D FC          MOV ECX,DWORD PTR SS:[EBP-0x4]
 *  0052898E   E8 25CEEDFF      CALL .004057B8
 *  00528993   8D83 4C020000    LEA EAX,DWORD PTR DS:[EBX+0x24C]
 *  00528999   8B55 FC          MOV EDX,DWORD PTR SS:[EBP-0x4]
 *  0052899C   E8 53CBEDFF      CALL .004054F4
 *  005289A1   8B83 4C020000    MOV EAX,DWORD PTR DS:[EBX+0x24C]
 *  005289A7   85C0             TEST EAX,EAX
 *  005289A9   74 05            JE SHORT .005289B0
 *  005289AB   83E8 04          SUB EAX,0x4
 *  005289AE   8B00             MOV EAX,DWORD PTR DS:[EAX]
 *  005289B0   8983 50020000    MOV DWORD PTR DS:[EBX+0x250],EAX
 *  005289B6   C645 F7 01       MOV BYTE PTR SS:[EBP-0x9],0x1
 *  005289BA   33C0             XOR EAX,EAX
 *  005289BC   5A               POP EDX
 *  005289BD   59               POP ECX
 *  005289BE   59               POP ECX
 *  005289BF   64:8910          MOV DWORD PTR FS:[EAX],EDX
 *  005289C2   68 E4895200      PUSH .005289E4
 *  005289C7   8D45 E8          LEA EAX,DWORD PTR SS:[EBP-0x18]
 *  005289CA   BA 03000000      MOV EDX,0x3
 *  005289CF   E8 F0CAEDFF      CALL .004054C4
 *  005289D4   8D45 FC          LEA EAX,DWORD PTR SS:[EBP-0x4]
 *  005289D7   E8 C4CAEDFF      CALL .004054A0
 *  005289DC   C3               RETN
 *  005289DD  ^E9 72C3EDFF      JMP .00404D54
 *  005289E2  ^EB E3            JMP SHORT .005289C7
 *  005289E4   0FB645 F7        MOVZX EAX,BYTE PTR SS:[EBP-0x9]
 *  005289E8   5F               POP EDI
 *  005289E9   5E               POP ESI
 *  005289EA   5B               POP EBX
 *  005289EB   8BE5             MOV ESP,EBP
 *  005289ED   5D               POP EBP
 *  005289EE   C3               RETN
 *  005289EF   90               NOP
 *  005289F0   55               PUSH EBP
 *  005289F1   8BEC             MOV EBP,ESP
 *  005289F3   83C4 F8          ADD ESP,-0x8
 *  005289F6   53               PUSH EBX
 */
bool attach(ulong startAddress, ulong stopAddress)
{
  const uint8_t bytes[] = {
    0x80,0xBB, 0x0A,0x16,0x00,0x00, 0x00  // 0051324D   80BB 0A160000 00 CMP BYTE PTR DS:[EBX+0x160A],0x0
  };
  ulong addr = MemDbg::findBytes(bytes, sizeof(bytes), startAddress, stopAddress);
  if (!addr)
    return false;
  // 00513234   55               PUSH EBP   ; jichi: hook here
  // 00513235   8BEC             MOV EBP,ESP
  // 00513237   6A 00            PUSH 0x0
  // 00513239   53               PUSH EBX
  // 0051323A   56               PUSH ESI
  enum : DWORD { sig = 0x6aec8b55 };
  addr = MemDbg::findEnclosingFunctionBeforeDword(sig, addr, 0x100, 1); // 25 = 0x0051324D - 0x00513234, step = 1
  if (!addr)
    return false;
  return winhook::hook_before(addr, Private::hookBefore);
}

} // namespace ScenarioHook

} // unnamed namespace

/** Public class */

bool QLiEEngine::attach()
{
  ulong startAddress, stopAddress;
  if (!Engine::getProcessMemoryRange(&startAddress, &stopAddress))
    return false;
  if (!ScenarioHook::attach(startAddress, stopAddress))
    return false;

  HijackManager::instance()->attachFunction((ulong)::GetTextExtentPoint32A);
  HijackManager::instance()->attachFunction((ulong)::ExtTextOutA);
  return true;
}

// EOF
