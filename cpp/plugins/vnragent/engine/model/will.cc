// will.cc
// 7/3/2015 jichi
// See: http://capita.tistory.com/m/post/251
#include "engine/model/will.h"
#include "engine/enginecontroller.h"
#include "engine/enginehash.h"
#include "engine/enginedef.h"
#include "engine/engineutil.h"
#include "hijack/hijackmanager.h"
#include "memdbg/memsearch.h"
#include "winhook/hookcode.h"
#include <qt_windows.h>
#include <cstdint>

#define DEBUG "model/will"
#include "sakurakit/skdebug.h"

/** Private data */

namespace { // unnamed

// Sample prefix: %LF
// Sample suffix: %L%P%W
QString trimW(const QString &text, QString *prefix, QString *suffix)
{
  QString ret = text;
  if (text.startsWith('%')) {
    int pos = 0;
    while (pos < text.size() - 1 && text[pos].unicode() == '%' && ::isupper(text[pos+1].unicode())) {
      pos += 2;
      while (::isupper(text[pos].unicode()))
        pos++;
    }
    if (prefix)
      *prefix = ret.left(pos);
    ret = ret.mid(pos);
  }
  if (ret.contains('%')) {
    int pos = ret.size();
    for (int i = ret.size() - 1; i >= 0; i--) {
      wchar_t w = ret[i].unicode();
      if (::isupper(w))
        ;
      else if (w == '%')
        pos = i;
      else
        break;
    }
    if (pos != ret.size()) {
      if (suffix)
        *suffix = ret.mid(pos);
      ret = ret.left(pos);
    }
  }
  return ret;
}

class TextHookW
{
  typedef TextHookW Self;

  QString text_;
  int stackIndex_;
  int role_;

  bool hookBefore(winhook::hook_stack *s)
  {
    auto text = (LPCWSTR)s->stack[stackIndex_];
    if (!text || !*text)
      return true;
    QString oldText = QString::fromWCharArray(text),
            prefix,
            suffix;
    oldText = trimW(oldText, &prefix, &suffix);
    if (oldText.isEmpty())
      return true;
    auto sig = Engine::hashThreadSignature(role_);
    QString newText = EngineController::instance()->dispatchTextW(oldText, sig, role_);
    if (newText == oldText)
      return true;
    if (!prefix.isEmpty())
      newText.prepend(prefix);
    if (!suffix.isEmpty())
      newText.append(suffix);
    text_ = newText;
    s->stack[stackIndex_] = (ulong)text_.utf16();
    return true;
  }

public:
  explicit TextHookW(int hookStackIndex, int role = Engine::UnknownRole) : stackIndex_(hookStackIndex), role_(role) {}

  bool attach(const uint8_t *pattern, size_t patternSize, ulong startAddress, ulong stopAddress)
  {
    ulong addr = MemDbg::findBytes(pattern, patternSize, startAddress, stopAddress);
    return addr && winhook::hook_before(addr,
        std::bind(&Self::hookBefore, this, std::placeholders::_1));
  }
};

/**
 *  Sample game: なついろレシピ
 *
 *  Scenario:
 *  00452A8F   77 05            JA SHORT .00452A96
 *  00452A91   E8 A25B0B00      CALL .00508638                           ; JMP to msvcr90._invalid_parameter_noinfo
 *  00452A96   8B43 0C          MOV EAX,DWORD PTR DS:[EBX+0xC]
 *  00452A99   8B48 18          MOV ECX,DWORD PTR DS:[EAX+0x18]
 *  00452A9C   83C0 10          ADD EAX,0x10
 *  00452A9F   33D2             XOR EDX,EDX
 *  00452AA1   8BC1             MOV EAX,ECX
 *  00452AA3   C78424 C4010000 >MOV DWORD PTR SS:[ESP+0x1C4],0x7
 *  00452AAE   C78424 C0010000 >MOV DWORD PTR SS:[ESP+0x1C0],0x0
 *  00452AB9   66:899424 B00100>MOV WORD PTR SS:[ESP+0x1B0],DX
 *  00452AC1   8D70 02          LEA ESI,DWORD PTR DS:[EAX+0x2]
 *  00452AC4   66:8B10          MOV DX,WORD PTR DS:[EAX]
 *  00452AC7   83C0 02          ADD EAX,0x2
 *  00452ACA   66:85D2          TEST DX,DX
 *  00452ACD  ^75 F5            JNZ SHORT .00452AC4
 *  00452ACF   2BC6             SUB EAX,ESI
 *  00452AD1   D1F8             SAR EAX,1
 *  00452AD3   50               PUSH EAX
 *  00452AD4   51               PUSH ECX
 *  00452AD5   8DB424 B4010000  LEA ESI,DWORD PTR SS:[ESP+0x1B4]
 *  00452ADC   E8 DF4DFBFF      CALL .004078C0
 *  00452AE1   C68424 B8020000 >MOV BYTE PTR SS:[ESP+0x2B8],0x8
 *  00452AE9   8B43 10          MOV EAX,DWORD PTR DS:[EBX+0x10]
 *  00452AEC   2B43 0C          SUB EAX,DWORD PTR DS:[EBX+0xC]
 *  00452AEF   C1F8 04          SAR EAX,0x4
 *  00452AF2   83F8 02          CMP EAX,0x2
 *  00452AF5   77 05            JA SHORT .00452AFC
 *  00452AF7   E8 3C5B0B00      CALL .00508638                           ; JMP to msvcr90._invalid_parameter_noinfo
 *  00452AFC   8B43 0C          MOV EAX,DWORD PTR DS:[EBX+0xC]
 *  00452AFF   8B48 28          MOV ECX,DWORD PTR DS:[EAX+0x28]
 *  00452B02   83C0 20          ADD EAX,0x20    ; jichi: hook before here, text in ecx
 *  00452B05   33D2             XOR EDX,EDX
 *  00452B07   8BC1             MOV EAX,ECX
 *  00452B09   C78424 E0010000 07000000        MOV DWORD PTR SS:[ESP+0x1E0],0x7
 *  00452B14   C78424 DC010000 00000000        MOV DWORD PTR SS:[ESP+0x1DC],0x0
 *  00452B27   8D70 02          LEA ESI,DWORD PTR DS:[EAX+0x2]
 *  00452B2A   33DB             XOR EBX,EBX
 *  00452B2C   8D6424 00        LEA ESP,DWORD PTR SS:[ESP]
 *  00452B30   66:8B10          MOV DX,WORD PTR DS:[EAX]
 *  00452B33   83C0 02          ADD EAX,0x2
 *  00452B36   66:3BD3          CMP DX,BX
 *  00452B39  ^75 F5            JNZ SHORT .00452B30
 *  00452B3B   2BC6             SUB EAX,ESI
 *  00452B3D   D1F8             SAR EAX,1
 *  00452B3F   50               PUSH EAX
 *  00452B40   51               PUSH ECX
 *  00452B41   8DB424 D0010000  LEA ESI,DWORD PTR SS:[ESP+0x1D0]
 *  00452B48   E8 734DFBFF      CALL .004078C0
 *  00452B4D   C68424 B8020000 >MOV BYTE PTR SS:[ESP+0x2B8],0x9
 *  00452B55   895C24 1C        MOV DWORD PTR SS:[ESP+0x1C],EBX
 *  00452B59   395C24 14        CMP DWORD PTR SS:[ESP+0x14],EBX
 *  00452B5D   0F84 77080000    JE .004533DA
 *  00452B63   BE 07000000      MOV ESI,0x7
 *  00452B68   33C0             XOR EAX,EAX
 *  00452B6A   895C24 20        MOV DWORD PTR SS:[ESP+0x20],EBX
 *  00452B6E   89B424 FC010000  MOV DWORD PTR SS:[ESP+0x1FC],ESI
 *  00452B75   899C24 F8010000  MOV DWORD PTR SS:[ESP+0x1F8],EBX
 *  00452B7C   66:898424 E80100>MOV WORD PTR SS:[ESP+0x1E8],AX
 *  00452B84   8D4C24 3C        LEA ECX,DWORD PTR SS:[ESP+0x3C]
 *  00452B88   51               PUSH ECX
 *  00452B89   C68424 BC020000 >MOV BYTE PTR SS:[ESP+0x2BC],0xA
 *  00452B91   E8 7AACFCFF      CALL .0041D810
 *  00452B96   C68424 B8020000 >MOV BYTE PTR SS:[ESP+0x2B8],0xB
 *  00452B9E   399C24 C0010000  CMP DWORD PTR SS:[ESP+0x1C0],EBX
 *  00452BA5   0F84 BB020000    JE .00452E66
 *  00452BAB   81C7 14010000    ADD EDI,0x114
 */
bool attachScenarioHookW(ulong startAddress, ulong stopAddress)
{
  // ECX PTR: 83 C0 20 33 D2 8B C1 C7 84 24 E0 01 00 00 07 00 00 00
  const uint8_t bytes[] = {
    0x83,0xc0, 0x20,     // 00452b02   83c0 20     add eax,0x20    ; jichi: hook before here, text in ecx
    0x33,0xd2,           // 00452b05   33d2        xor edx,edx
    0x8b,0xc1,           // 00452b07   8bc1        mov eax,ecx
    0xc7,0x84,0x24, 0xe0,0x01,0x00,0x00, 0x07,0x00,0x00,0x00  // 00452b09   c78424 e0010000 07000000      mov dword ptr ss:[esp+0x1e0],0x7
                                                              // 00452b14   c78424 dc010000 00000000      mov dword ptr ss:[esp+0x1dc],0x0
  };
  int ecx = winhook_stack_indexof(ecx);
  static auto h = new TextHookW(ecx, Engine::ScenarioRole); // never deleted
  return h->attach(bytes, sizeof(bytes), startAddress, stopAddress);
}

/**
 *  Sample game: なついろレシピ
 *
 *  Name:
 *
 *  004534FA   64:A3 00000000                  MOV DWORD PTR FS:[0],EAX
 *  00453500   8B75 14                         MOV ESI,DWORD PTR SS:[EBP+0x14]
 *  00453503   8B46 10                         MOV EAX,DWORD PTR DS:[ESI+0x10]
 *  00453506   2B46 0C                         SUB EAX,DWORD PTR DS:[ESI+0xC]
 *  00453509   8BF9                            MOV EDI,ECX
 *  0045350B   C1F8 04                         SAR EAX,0x4
 *  0045350E   897C24 14                       MOV DWORD PTR SS:[ESP+0x14],EDI
 *  00453512   85C0                            TEST EAX,EAX
 *  00453514   77 05                           JA SHORT .0045351B
 *  00453516   E8 1D510B00                     CALL .00508638                           ; JMP to msvcr90._invalid_parameter_noinfo
 *  0045351B   8B76 0C                         MOV ESI,DWORD PTR DS:[ESI+0xC]
 *  0045351E   8B4E 08                         MOV ECX,DWORD PTR DS:[ESI+0x8]
 *  00453521   33DB                            XOR EBX,EBX  ; jichi: hook here, text in ecx
 *  00453523   33D2                            XOR EDX,EDX
 *  00453525   8BC1                            MOV EAX,ECX
 *  00453527   C78424 88000000 07000000        MOV DWORD PTR SS:[ESP+0x88],0x7
 *  00453532   899C24 84000000                 MOV DWORD PTR SS:[ESP+0x84],EBX
 *  00453539   66:895424 74                    MOV WORD PTR SS:[ESP+0x74],DX
 *  0045353E   8D70 02                         LEA ESI,DWORD PTR DS:[EAX+0x2]
 *  00453541   66:8B10                         MOV DX,WORD PTR DS:[EAX]
 *  00453544   83C0 02                         ADD EAX,0x2
 *  00453547   66:3BD3                         CMP DX,BX
 *  0045354A  ^75 F5                           JNZ SHORT .00453541
 *  0045354C   2BC6                            SUB EAX,ESI
 *  0045354E   D1F8                            SAR EAX,1
 *  00453550   50                              PUSH EAX
 *  00453551   51                              PUSH ECX
 *  00453552   8D7424 78                       LEA ESI,DWORD PTR SS:[ESP+0x78]
 *  00453556   E8 6543FBFF                     CALL .004078C0
 *  0045355B   899C24 70010000                 MOV DWORD PTR SS:[ESP+0x170],EBX
 *  00453562   A1 DCAA5500                     MOV EAX,DWORD PTR DS:[0x55AADC]
 *  00453567   894424 1C                       MOV DWORD PTR SS:[ESP+0x1C],EAX
 *  0045356B   B8 0F000000                     MOV EAX,0xF
 *  00453570   894424 6C                       MOV DWORD PTR SS:[ESP+0x6C],EAX
 *  00453574   895C24 68                       MOV DWORD PTR SS:[ESP+0x68],EBX
 *  00453578   885C24 58                       MOV BYTE PTR SS:[ESP+0x58],BL
 *  0045357C   894424 50                       MOV DWORD PTR SS:[ESP+0x50],EAX
 *  00453580   895C24 4C                       MOV DWORD PTR SS:[ESP+0x4C],EBX
 *  00453584   885C24 3C                       MOV BYTE PTR SS:[ESP+0x3C],BL
 *  00453588   C68424 70010000 02              MOV BYTE PTR SS:[ESP+0x170],0x2
 *  00453590   8B8424 84000000                 MOV EAX,DWORD PTR SS:[ESP+0x84]
 *  00453597   8BF0                            MOV ESI,EAX
 *  00453599   3BC3                            CMP EAX,EBX
 *  0045359B   74 3D                           JE SHORT .004535DA
 *  0045359D   83BC24 88000000 08              CMP DWORD PTR SS:[ESP+0x88],0x8
 *  004535A5   8B5424 74                       MOV EDX,DWORD PTR SS:[ESP+0x74]
 *  004535A9   73 04                           JNB SHORT .004535AF
 *  004535AB   8D5424 74                       LEA EDX,DWORD PTR SS:[ESP+0x74]
 */
bool attachNameHookW(ulong startAddress, ulong stopAddress)
{
  // ECX PTR: 33 DB 33 D2 8B C1 C7 84 24 88 00 00 00 07 00 00 00
  const uint8_t bytes[] = {
    0x33,0xdb,   // 00453521   33db                            xor ebx,ebx  ; jichi: hook here, text in ecx
    0x33,0xd2,   // 00453523   33d2                            xor edx,edx
    0x8b,0xc1,   // 00453525   8bc1                            mov eax,ecx
    0xc7,0x84,0x24, 0x88,0x00,0x00,0x00, 0x07,0x00,0x00,0x00 // 00453527   c78424 88000000 07000000        mov dword ptr ss:[esp+0x88],0x7
                                                             // 00453532   899c24 84000000                 mov dword ptr ss:[esp+0x84],ebx
  };
  int ecx = winhook_stack_indexof(ecx);
  static auto h = new TextHookW(ecx, Engine::NameRole); // never deleted
  return h->attach(bytes, sizeof(bytes), startAddress, stopAddress);
}

/**
 *  Sample game: なついろレシピ
 *
 *  Choice:
 *  00470D95   72 05                           JB SHORT .00470D9C
 *  00470D97   E8 9C780900                     CALL .00508638                           ; JMP to msvcr90._invalid_parameter_noinfo
 *  00470D9C   8BB5 EC020000                   MOV ESI,DWORD PTR SS:[EBP+0x2EC]
 *  00470DA2   037424 14                       ADD ESI,DWORD PTR SS:[ESP+0x14]
 *  00470DA6   8B4E 10                         MOV ECX,DWORD PTR DS:[ESI+0x10]
 *  00470DA9   2B4E 0C                         SUB ECX,DWORD PTR DS:[ESI+0xC]
 *  00470DAC   C1F9 04                         SAR ECX,0x4
 *  00470DAF   83F9 01                         CMP ECX,0x1
 *  00470DB2   77 05                           JA SHORT .00470DB9
 *  00470DB4   E8 7F780900                     CALL .00508638                           ; JMP to msvcr90._invalid_parameter_noinfo
 *  00470DB9   8B46 0C                         MOV EAX,DWORD PTR DS:[ESI+0xC]
 *  00470DBC   8B50 18                         MOV EDX,DWORD PTR DS:[EAX+0x18]
 *  00470DBF   83C0 10                         ADD EAX,0x10     ; jichi: text in edx
 *  00470DC2   52                              PUSH EDX
 *  00470DC3   8D8C24 7C040000                 LEA ECX,DWORD PTR SS:[ESP+0x47C]
 *  00470DCA   8D7424 4C                       LEA ESI,DWORD PTR SS:[ESP+0x4C]
 *  00470DCE   E8 EDA3F9FF                     CALL .0040B1C0
 *  00470DD3   83C4 04                         ADD ESP,0x4
 *  00470DD6   6A FF                           PUSH -0x1
 *  00470DD8   53                              PUSH EBX
 *  00470DD9   50                              PUSH EAX
 *  00470DDA   8D8424 84040000                 LEA EAX,DWORD PTR SS:[ESP+0x484]
 *  00470DE1   C68424 B0040000 07              MOV BYTE PTR SS:[ESP+0x4B0],0x7
 *  00470DE9   E8 1251F9FF                     CALL .00405F00
 *  00470DEE   BE 08000000                     MOV ESI,0x8
 *  00470DF3   C68424 A4040000 06              MOV BYTE PTR SS:[ESP+0x4A4],0x6
 *  00470DFB   397424 60                       CMP DWORD PTR SS:[ESP+0x60],ESI
 *  00470DFF   72 0D                           JB SHORT .00470E0E
 *  00470E01   8B4424 4C                       MOV EAX,DWORD PTR SS:[ESP+0x4C]
 *  00470E05   50                              PUSH EAX
 *  00470E06   E8 65770900                     CALL .00508570                           ; JMP to msvcr90.??3@YAXPAX@Z
 *  00470E0B   83C4 04                         ADD ESP,0x4
 *  00470E0E   8B9424 7C040000                 MOV EDX,DWORD PTR SS:[ESP+0x47C]
 *  00470E15   33C9                            XOR ECX,ECX
 *  00470E17   C74424 60 07000000              MOV DWORD PTR SS:[ESP+0x60],0x7
 *  00470E1F   895C24 5C                       MOV DWORD PTR SS:[ESP+0x5C],EBX
 *  00470E23   66:894C24 4C                    MOV WORD PTR SS:[ESP+0x4C],CX
 *  00470E28   39B424 90040000                 CMP DWORD PTR SS:[ESP+0x490],ESI
 *  00470E2F   73 07                           JNB SHORT .00470E38
 *  00470E31   8D9424 7C040000                 LEA EDX,DWORD PTR SS:[ESP+0x47C]
 *  00470E38   8B8424 44040000                 MOV EAX,DWORD PTR SS:[ESP+0x444]
 *  00470E3F   B9 10000000                     MOV ECX,0x10
 *  00470E44   398C24 58040000                 CMP DWORD PTR SS:[ESP+0x458],ECX
 *  00470E4B   73 07                           JNB SHORT .00470E54
 *  00470E4D   8D8424 44040000                 LEA EAX,DWORD PTR SS:[ESP+0x444]
 *  00470E54   398C24 74040000                 CMP DWORD PTR SS:[ESP+0x474],ECX
 *  00470E5B   8B8C24 60040000                 MOV ECX,DWORD PTR SS:[ESP+0x460]
 */
bool attachOtherHookW(ulong startAddress, ulong stopAddress)
{
  // EDX PTR : 83 C0 10 52 8D 8C 24 7C 04 00 00 8D 74 24 4C
  const uint8_t bytes[] = {
    0x83,0xc0, 0x10,                        // 00470dbf   83c0 10                         add eax,0x10     ; jichi: text in edx
    0x52,                                   // 00470dc2   52                              push edx
    0x8d,0x8c,0x24, 0x7c,0x04,0x00,0x00,    // 00470dc3   8d8c24 7c040000                 lea ecx,dword ptr ss:[esp+0x47c]
    0x8d,0x74,0x24, 0x4c                    // 00470dca   8d7424 4c                       lea esi,dword ptr ss:[esp+0x4c]
  };
  int edx = winhook_stack_indexof(edx);
  static auto h = new TextHookW(edx, Engine::OtherRole); // never deleted
  return h->attach(bytes, sizeof(bytes), startAddress, stopAddress);
}

} // unnamed namespace

/** Public class */

bool WillPlusEngine::attach()
{
  ulong startAddress, stopAddress;
  if (!Engine::getProcessMemoryRange(&startAddress, &stopAddress))
    return false;
  if (!::attachScenarioHookW(startAddress, stopAddress))
    return false;

  if (::attachNameHookW(startAddress, stopAddress))
    DOUT("name text found");
  else
    DOUT("name text NOT FOUND");

  if (::attachOtherHookW(startAddress, stopAddress))
    DOUT("other text found");
  else
    DOUT("other text NOT FOUND");

  //HijackManager::instance()->attachFunction((ulong)::GetGlyphOutlineW);
  return true;
}

// EOF
