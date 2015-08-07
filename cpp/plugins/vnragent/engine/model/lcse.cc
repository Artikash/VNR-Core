// lcse.cc
// 8/6/2015 jichi
#include "engine/model/lcse.h"
#include "engine/enginecontroller.h"
#include "engine/enginedef.h"
#include "engine/enginehash.h"
#include "engine/engineutil.h"
#include "hijack/hijackmanager.h"
#include "util/textutil.h"
#include "winhook/hookcode.h"
#include "memdbg/memsearch.h"
#include <qt_windows.h>
#include <cstdint>

#define DEBUG "model/lcse"
#include "sakurakit/skdebug.h"

namespace { // unnamed
namespace ScenarioHook {
namespace Private {

  // Skip trailing 0203
  int rtrim(LPCSTR text, int size)
  {
    if (size >= 2 && text[size - 1] == 0x3 && text[size - 2] == 0x2)
      size -= 2;
    return size;
  }

  /**
   *  Sample game: 春恋＊乙女～乙女の園でごきげんよう。～
   *
   *  067C73FA  8F CD 90 6D 01 81 75 96 7B 93 96 82 C9 82 B1 82  章仁「本当にこ・
   *  067C740A  F1 82 C8 82 C6 82 B1 82 EB 82 AA 82 A0 82 E9 82  ﾈところがある・
   *  067C741A  F1 82 BE 82 C8 82 9F 81 63 81 63 81 76 02 03 00  ｾなぁ……」.
   *  067C742A  38 00 00 00 01 81 40 96 DA 82 CC 91 4F 82 C9 8D  8...　目の前に・
   *  067C743A  4C 82 AA 82 E9 8C F5 8C 69 82 F0 91 4F 82 C9 81  Lがる光景を前に・
   *
   *  Name/scenario splitter: 01 ()
   *  New line splitter: 0203 ()
   */

  // 0042FBE8   A1 E8234A00      MOV EAX,DWORD PTR DS:[0x4A23E8] ; jichi: text length here
  //
  // 0042FC03   8B15 E8234A00    MOV EDX,DWORD PTR DS:[0x4A23E8]	; jichi: text length here
  // 0042FC09   8B4C24 10        MOV ECX,DWORD PTR SS:[ESP+0x10]	; jichi: count is here
  // 0042FC0D   8B76 04          MOV ESI,DWORD PTR DS:[ESI+0x4]
  // 0042FC10   894424 14        MOV DWORD PTR SS:[ESP+0x14],EAX
  // 0042FC14   8B92 44290000    MOV EDX,DWORD PTR DS:[EDX+0x2944]	; jichi: offset
  // 0042FC1A   8BF8             MOV EDI,EAX
  // 0042FC1C   8BC1             MOV EAX,ECX
  // 0042FC1E   83C4 04          ADD ESP,0x4
  // 0042FC21   8D7432 04        LEA ESI,DWORD PTR DS:[EDX+ESI+0x4]

  ulong textBaseAddress_,   // 0042FC03   8B15 E8234A00    MOV EDX,DWORD PTR DS:[0x4A23E8]
        textOffset_;        // 0042FC14   8B92 44290000    MOV EDX,DWORD PTR DS:[EDX+0x2944]

  QByteArray data_;

  bool hook1(winhook::hook_stack *s)
  {
    data_.clear();

    int size = s->eax - 1;
    if (size <= 0)
      return true;

    // 0042FC03   8B15 E8234A00    MOV EDX,DWORD PTR DS:[0x4A23E8]	; jichi: text here
    // 0042FC09   8B4C24 10        MOV ECX,DWORD PTR SS:[ESP+0x10]	; jichi: count is here
    // 0042FC0D   8B76 04          MOV ESI,DWORD PTR DS:[ESI+0x4]   ; jichi: [arg1+4]
    // 0042FC10   894424 14        MOV DWORD PTR SS:[ESP+0x14],EAX
    // 0042FC14   8B92 44290000    MOV EDX,DWORD PTR DS:[EDX+0x2944] ; jichi: base addr, [[0x4A23E8] + 0x2944]
    // 0042FC1A   8BF8             MOV EDI,EAX
    // 0042FC1C   8BC1             MOV EAX,ECX
    // 0042FC1E   83C4 04          ADD ESP,0x4
    //
    // 0042FC21   8D7432 04        LEA ESI,DWORD PTR DS:[EDX+ESI+0x4]  ; jichi: hook2, text in esi

    ulong edx, esi;
    {
      edx = *(DWORD *)textBaseAddress_;     // 0042FC03   8B15 E8234A00    MOV EDX,DWORD PTR DS:[0x4A23E8]
      edx = *(DWORD *)(edx + textOffset_);  // 0042FC14   8B92 44290000    MOV EDX,DWORD PTR DS:[EDX+0x2944]
      esi = *(DWORD *)(s->esi + 0x4);       // 0042FC0D   8B76 04          MOV ESI,DWORD PTR DS:[ESI+0x4]
      esi = edx + esi + 0x4;                // 0042FC21   8D7432 04        LEA ESI,DWORD PTR DS:[EDX+ESI+0x4]
    }

    auto text = (LPCSTR)esi;
    if (!*text || ::strlen(text) != size || Util::allAscii(text))
      return true;

    int trimmedSize = rtrim(text, size);
    if (trimmedSize <= 0)
      return true;

    //auto size = s->ecx * 4;
    //auto dst = (LPSTR)s->edi;
    enum { role = Engine::ScenarioRole, sig = 0 };
    QByteArray oldData(text, trimmedSize),
               newData = EngineController::instance()->dispatchTextA(oldData, role, sig);
    if (newData.isEmpty() || newData == oldData)
      return true;
    if (trimmedSize != size)
      newData.append(text + trimmedSize, size - trimmedSize);
    data_ = newData;
    s->eax = data_.size() + 1;
    return true;
  }

  bool hook2(winhook::hook_stack *s)
  {
    if (!data_.isEmpty())
      s->esi = (ulong)data_.constData();
    return true;
  }
} // namespace Private

/**
 *  Sample game: 春恋＊乙女～乙女の園でごきげんよう。～
 *
 *  0042FB1E   CC               INT3
 *  0042FB1F   CC               INT3
 *  0042FB20   6A FF            PUSH -0x1
 *  0042FB22   68 18094800      PUSH lcsebody.00480918
 *  0042FB27   64:A1 00000000   MOV EAX,DWORD PTR FS:[0]
 *  0042FB2D   50               PUSH EAX
 *  0042FB2E   64:8925 00000000 MOV DWORD PTR FS:[0],ESP
 *  0042FB35   83EC 08          SUB ESP,0x8
 *  0042FB38   53               PUSH EBX
 *  0042FB39   33DB             XOR EBX,EBX
 *  0042FB3B   56               PUSH ESI
 *  0042FB3C   57               PUSH EDI
 *  0042FB3D   895C24 0C        MOV DWORD PTR SS:[ESP+0xC],EBX
 *  0042FB41   895C24 10        MOV DWORD PTR SS:[ESP+0x10],EBX
 *  0042FB45   8B7424 24        MOV ESI,DWORD PTR SS:[ESP+0x24] ; jichi; arg1
 *  0042FB49   895C24 1C        MOV DWORD PTR SS:[ESP+0x1C],EBX
 *  0042FB4D   8B06             MOV EAX,DWORD PTR DS:[ESI]
 *  0042FB4F   83F8 05          CMP EAX,0x5
 *  0042FB52   75 2F            JNZ SHORT lcsebody.0042FB83
 *  0042FB54   8B76 04          MOV ESI,DWORD PTR DS:[ESI+0x4]
 *  0042FB57   8B3D E8234A00    MOV EDI,DWORD PTR DS:[0x4A23E8]
 *  0042FB5D   3BF3             CMP ESI,EBX
 *  0042FB5F   7C 08            JL SHORT lcsebody.0042FB69
 *  0042FB61   39B7 54290000    CMP DWORD PTR DS:[EDI+0x2954],ESI
 *  0042FB67   7F 12            JG SHORT lcsebody.0042FB7B
 *  0042FB69   53               PUSH EBX
 *  0042FB6A   68 20F54800      PUSH lcsebody.0048F520                   ; ASCII "err"
 *  0042FB6F   68 F4F44800      PUSH lcsebody.0048F4F4
 *  0042FB74   53               PUSH EBX
 *  0042FB75   FF15 EC874A00    CALL DWORD PTR DS:[<&USER32.MessageBoxA>>; user32.MessageBoxA
 *  0042FB7B   8B87 74290000    MOV EAX,DWORD PTR DS:[EDI+0x2974]
 *  0042FB81   EB 32            JMP SHORT lcsebody.0042FBB5
 *  0042FB83   83F8 08          CMP EAX,0x8 ; jichi: esi=arg1 jumped here
 *  0042FB86   75 57            JNZ SHORT lcsebody.0042FBDF
 *  0042FB88   8B76 04          MOV ESI,DWORD PTR DS:[ESI+0x4]
 *  0042FB8B   8B3D E8234A00    MOV EDI,DWORD PTR DS:[0x4A23E8]
 *  0042FB91   3BF3             CMP ESI,EBX
 *  0042FB93   7C 08            JL SHORT lcsebody.0042FB9D
 *  0042FB95   39B7 60290000    CMP DWORD PTR DS:[EDI+0x2960],ESI
 *  0042FB9B   7F 12            JG SHORT lcsebody.0042FBAF
 *  0042FB9D   53               PUSH EBX
 *  0042FB9E   68 20F54800      PUSH lcsebody.0048F520                   ; ASCII "err"
 *  0042FBA3   68 F4F44800      PUSH lcsebody.0048F4F4
 *  0042FBA8   53               PUSH EBX
 *  0042FBA9   FF15 EC874A00    CALL DWORD PTR DS:[<&USER32.MessageBoxA>>; user32.MessageBoxA
 *  0042FBAF   8B87 80290000    MOV EAX,DWORD PTR DS:[EDI+0x2980]
 *  0042FBB5   8D34F0           LEA ESI,DWORD PTR DS:[EAX+ESI*8]
 *  0042FBB8   8B06             MOV EAX,DWORD PTR DS:[ESI]
 *  0042FBBA   50               PUSH EAX
 *  0042FBBB   894424 10        MOV DWORD PTR SS:[ESP+0x10],EAX
 *  0042FBBF   E8 5E840000      CALL lcsebody.00438022
 *  0042FBC4   8B4C24 10        MOV ECX,DWORD PTR SS:[ESP+0x10]
 *  0042FBC8   83C4 04          ADD ESP,0x4
 *  0042FBCB   8BD1             MOV EDX,ECX
 *  0042FBCD   894424 10        MOV DWORD PTR SS:[ESP+0x10],EAX
 *  0042FBD1   8B76 04          MOV ESI,DWORD PTR DS:[ESI+0x4]
 *  0042FBD4   8BF8             MOV EDI,EAX
 *  0042FBD6   C1E9 02          SHR ECX,0x2
 *  0042FBD9   F3:A5            REP MOVS DWORD PTR ES:[EDI],DWORD PTR DS>
 *  0042FBDB   8BCA             MOV ECX,EDX
 *  0042FBDD   EB 4D            JMP SHORT lcsebody.0042FC2C
 *  0042FBDF   83F8 02          CMP EAX,0x2  ; jichi: esi=arg1 jumped here
 *  0042FBE2   0F85 A2000000    JNZ lcsebody.0042FC8A
 *  0042FBE8   A1 E8234A00      MOV EAX,DWORD PTR DS:[0x4A23E8] ; jichi: text length here
 *  0042FBED   8B56 04          MOV EDX,DWORD PTR DS:[ESI+0x4]
 *  0042FBF0   8B88 44290000    MOV ECX,DWORD PTR DS:[EAX+0x2944]
 *  0042FBF6   8B0411           MOV EAX,DWORD PTR DS:[ECX+EDX]
 *
 *  0042FBF9   50               PUSH EAX    ; jichi: hook1, text length pushed, new function
 *  0042FBFA   894424 10        MOV DWORD PTR SS:[ESP+0x10],EAX ; jichi: text length, is this the memory allocation
 *  0042FBFE   E8 1F840000      CALL lcsebody.00438022
 *
 *  0042FC03   8B15 E8234A00    MOV EDX,DWORD PTR DS:[0x4A23E8]	; jichi: text here
 *  0042FC09   8B4C24 10        MOV ECX,DWORD PTR SS:[ESP+0x10]	; jichi: count is here
 *  0042FC0D   8B76 04          MOV ESI,DWORD PTR DS:[ESI+0x4]  ; jichi: [arg1+4]
 *  0042FC10   894424 14        MOV DWORD PTR SS:[ESP+0x14],EAX
 *  0042FC14   8B92 44290000    MOV EDX,DWORD PTR DS:[EDX+0x2944] ; jichi: base addr, [[0x4A23E8] + 0x2944]
 *  0042FC1A   8BF8             MOV EDI,EAX
 *  0042FC1C   8BC1             MOV EAX,ECX
 *  0042FC1E   83C4 04          ADD ESP,0x4
 *
 *  0042FC21   8D7432 04        LEA ESI,DWORD PTR DS:[EDX+ESI+0x4]  ; jichi: hook2, text in esi
 *  0042FC25   C1E9 02          SHR ECX,0x2	; jichi: ecx is now the count, here, the rep function is blocked by 4 for performance
 *  0042FC28   F3:A5            REP MOVS DWORD PTR ES:[EDI],DWORD PTR DS[ESI]	; jichi: text accessed here from esi to edi
 *
 *  0042FC2A   8BC8             MOV ECX,EAX
 *  0042FC2C   8B5424 28        MOV EDX,DWORD PTR SS:[ESP+0x28]
 *  0042FC30   83E1 03          AND ECX,0x3
 *  0042FC33   F3:A4            REP MOVS BYTE PTR ES:[EDI],BYTE PTR DS:[ESI]
 *  0042FC35   8B4C24 2C        MOV ECX,DWORD PTR SS:[ESP+0x2C]
 *  0042FC39   8D4424 0C        LEA EAX,DWORD PTR SS:[ESP+0xC]
 *  0042FC3D   51               PUSH ECX
 *  0042FC3E   52               PUSH EDX
 *  0042FC3F   50               PUSH EAX
 *  0042FC40   E8 AB14FDFF      CALL lcsebody.004010F0
 *  0042FC45   83C4 0C          ADD ESP,0xC
 *  0042FC48   C74424 1C FFFFFF>MOV DWORD PTR SS:[ESP+0x1C],-0x1
 *  0042FC50   84C0             TEST AL,AL
 *  0042FC52   8B4424 10        MOV EAX,DWORD PTR SS:[ESP+0x10]
 *  0042FC56   895C24 0C        MOV DWORD PTR SS:[ESP+0xC],EBX
 *  0042FC5A   74 21            JE SHORT lcsebody.0042FC7D
 *  0042FC5C   3BC3             CMP EAX,EBX
 *  0042FC5E   74 09            JE SHORT lcsebody.0042FC69
 *  0042FC60   50               PUSH EAX
 *  0042FC61   E8 467E0000      CALL lcsebody.00437AAC
 *  0042FC66   83C4 04          ADD ESP,0x4
 *  0042FC69   5F               POP EDI
 *  0042FC6A   5E               POP ESI
 *  0042FC6B   B0 01            MOV AL,0x1
 *  0042FC6D   5B               POP EBX
 *  0042FC6E   8B4C24 08        MOV ECX,DWORD PTR SS:[ESP+0x8]
 *  0042FC72   64:890D 00000000 MOV DWORD PTR FS:[0],ECX
 *  0042FC79   83C4 14          ADD ESP,0x14
 *  0042FC7C   C3               RETN
 *  0042FC7D   3BC3             CMP EAX,EBX
 *  0042FC7F   74 09            JE SHORT lcsebody.0042FC8A
 *  0042FC81   50               PUSH EAX
 *  0042FC82   E8 257E0000      CALL lcsebody.00437AAC
 *  0042FC87   83C4 04          ADD ESP,0x4
 *  0042FC8A   8B4C24 14        MOV ECX,DWORD PTR SS:[ESP+0x14]
 *  0042FC8E   5F               POP EDI
 *  0042FC8F   5E               POP ESI
 *  0042FC90   32C0             XOR AL,AL
 *  0042FC92   5B               POP EBX
 *  0042FC93   64:890D 00000000 MOV DWORD PTR FS:[0],ECX
 *  0042FC9A   83C4 14          ADD ESP,0x14
 *  0042FC9D   C3               RETN
 *  0042FC9E   90               NOP
 *  0042FC9F   90               NOP
 *  0042FCA0   CC               INT3
 *  0042FCA1   CC               INT3
 *  0042FCA2   CC               INT3
 *  0042FCA3   CC               INT3
 *  0042FCA4   CC               INT3
 *  0042FCA5   CC               INT3
 *  0042FCA6   CC               INT3
 *
 *  Sample game: 姦獄学園
 *
 *  00430CAB   CC               INT3
 *  00430CAC   CC               INT3
 *  00430CAD   CC               INT3
 *  00430CAE   CC               INT3
 *  00430CAF   CC               INT3
 *  00430CB0   6A FF            PUSH -0x1
 *  00430CB2   68 08204800      PUSH .00482008
 *  00430CB7   64:A1 00000000   MOV EAX,DWORD PTR FS:[0]
 *  00430CBD   50               PUSH EAX
 *  00430CBE   64:8925 00000000 MOV DWORD PTR FS:[0],ESP
 *  00430CC5   83EC 08          SUB ESP,0x8
 *  00430CC8   53               PUSH EBX
 *  00430CC9   33DB             XOR EBX,EBX
 *  00430CCB   56               PUSH ESI
 *  00430CCC   57               PUSH EDI
 *  00430CCD   895C24 0C        MOV DWORD PTR SS:[ESP+0xC],EBX
 *  00430CD1   895C24 10        MOV DWORD PTR SS:[ESP+0x10],EBX
 *  00430CD5   8B7424 24        MOV ESI,DWORD PTR SS:[ESP+0x24]
 *  00430CD9   895C24 1C        MOV DWORD PTR SS:[ESP+0x1C],EBX
 *  00430CDD   8B06             MOV EAX,DWORD PTR DS:[ESI]
 *  00430CDF   83F8 05          CMP EAX,0x5
 *  00430CE2   75 2F            JNZ SHORT .00430D13
 *  00430CE4   8B76 04          MOV ESI,DWORD PTR DS:[ESI+0x4]
 *  00430CE7   8B3D 9C4E4A00    MOV EDI,DWORD PTR DS:[0x4A4E9C]
 *  00430CED   3BF3             CMP ESI,EBX
 *  00430CEF   7C 08            JL SHORT .00430CF9
 *  00430CF1   39B7 54310000    CMP DWORD PTR DS:[EDI+0x3154],ESI
 *  00430CF7   7F 12            JG SHORT .00430D0B
 *  00430CF9   53               PUSH EBX
 *  00430CFA   68 98154900      PUSH .00491598                           ; ASCII "err"
 *  00430CFF   68 D8254900      PUSH .004925D8
 *  00430D04   53               PUSH EBX
 *  00430D05   FF15 2CC84A00    CALL DWORD PTR DS:[0x4AC82C]             ; user32.MessageBoxA
 *  00430D0B   8B87 74310000    MOV EAX,DWORD PTR DS:[EDI+0x3174]
 *  00430D11   EB 32            JMP SHORT .00430D45
 *  00430D13   83F8 08          CMP EAX,0x8
 *  00430D16   75 57            JNZ SHORT .00430D6F
 *  00430D18   8B76 04          MOV ESI,DWORD PTR DS:[ESI+0x4]
 *  00430D1B   8B3D 9C4E4A00    MOV EDI,DWORD PTR DS:[0x4A4E9C]
 *  00430D21   3BF3             CMP ESI,EBX
 *  00430D23   7C 08            JL SHORT .00430D2D
 *  00430D25   39B7 60310000    CMP DWORD PTR DS:[EDI+0x3160],ESI
 *  00430D2B   7F 12            JG SHORT .00430D3F
 *  00430D2D   53               PUSH EBX
 *  00430D2E   68 98154900      PUSH .00491598                           ; ASCII "err"
 *  00430D33   68 AC254900      PUSH .004925AC
 *  00430D38   53               PUSH EBX
 *  00430D39   FF15 2CC84A00    CALL DWORD PTR DS:[0x4AC82C]             ; user32.MessageBoxA
 *  00430D3F   8B87 80310000    MOV EAX,DWORD PTR DS:[EDI+0x3180]
 *  00430D45   8D34F0           LEA ESI,DWORD PTR DS:[EAX+ESI*8]
 *  00430D48   8B06             MOV EAX,DWORD PTR DS:[ESI]
 *  00430D4A   50               PUSH EAX
 *  00430D4B   894424 10        MOV DWORD PTR SS:[ESP+0x10],EAX
 *  00430D4F   E8 BE890000      CALL .00439712
 *  00430D54   8B4C24 10        MOV ECX,DWORD PTR SS:[ESP+0x10]
 *  00430D58   83C4 04          ADD ESP,0x4
 *  00430D5B   8BD1             MOV EDX,ECX
 *  00430D5D   894424 10        MOV DWORD PTR SS:[ESP+0x10],EAX
 *  00430D61   8B76 04          MOV ESI,DWORD PTR DS:[ESI+0x4]
 *  00430D64   8BF8             MOV EDI,EAX
 *  00430D66   C1E9 02          SHR ECX,0x2
 *  00430D69   F3:A5            REP MOVS DWORD PTR ES:[EDI],DWORD PTR DS>
 *  00430D6B   8BCA             MOV ECX,EDX
 *  00430D6D   EB 4D            JMP SHORT .00430DBC
 *  00430D6F   83F8 02          CMP EAX,0x2
 *  00430D72   0F85 A2000000    JNZ .00430E1A
 *  00430D78   A1 9C4E4A00      MOV EAX,DWORD PTR DS:[0x4A4E9C]
 *  00430D7D   8B56 04          MOV EDX,DWORD PTR DS:[ESI+0x4]
 *  00430D80   8B88 44310000    MOV ECX,DWORD PTR DS:[EAX+0x3144]
 *  00430D86   8B0411           MOV EAX,DWORD PTR DS:[ECX+EDX]
 *  00430D89   50               PUSH EAX
 *  00430D8A   894424 10        MOV DWORD PTR SS:[ESP+0x10],EAX
 *  00430D8E   E8 7F890000      CALL .00439712
 *  00430D93   8B15 9C4E4A00    MOV EDX,DWORD PTR DS:[0x4A4E9C]
 *  00430D99   8B4C24 10        MOV ECX,DWORD PTR SS:[ESP+0x10]
 *  00430D9D   8B76 04          MOV ESI,DWORD PTR DS:[ESI+0x4]
 *  00430DA0   894424 14        MOV DWORD PTR SS:[ESP+0x14],EAX
 *  00430DA4   8B92 44310000    MOV EDX,DWORD PTR DS:[EDX+0x3144]
 *  00430DAA   8BF8             MOV EDI,EAX
 *  00430DAC   8BC1             MOV EAX,ECX
 *  00430DAE   83C4 04          ADD ESP,0x4
 *  00430DB1   8D7432 04        LEA ESI,DWORD PTR DS:[EDX+ESI+0x4]	; jichi: the other game's access point
 *  00430DB5   C1E9 02          SHR ECX,0x2
 *  00430DB8   F3:A5            REP MOVS DWORD PTR ES:[EDI],DWORD PTR DS:[ESI]
 *  00430DBA   8BC8             MOV ECX,EAX
 *  00430DBC   8B5424 28        MOV EDX,DWORD PTR SS:[ESP+0x28]
 *  00430DC0   83E1 03          AND ECX,0x3
 *  00430DC3   F3:A4            REP MOVS BYTE PTR ES:[EDI],BYTE PTR DS:[ESI]
 *  00430DC5   8B4C24 2C        MOV ECX,DWORD PTR SS:[ESP+0x2C]
 *  00430DC9   8D4424 0C        LEA EAX,DWORD PTR SS:[ESP+0xC]
 *  00430DCD   51               PUSH ECX
 *  00430DCE   52               PUSH EDX
 *  00430DCF   50               PUSH EAX
 *  00430DD0   E8 2503FDFF      CALL .004010FA
 *  00430DD5   83C4 0C          ADD ESP,0xC
 *  00430DD8   C74424 1C FFFFFF>MOV DWORD PTR SS:[ESP+0x1C],-0x1
 *  00430DE0   84C0             TEST AL,AL
 *  00430DE2   8B4424 10        MOV EAX,DWORD PTR SS:[ESP+0x10]
 *  00430DE6   895C24 0C        MOV DWORD PTR SS:[ESP+0xC],EBX
 *  00430DEA   74 21            JE SHORT .00430E0D
 *  00430DEC   3BC3             CMP EAX,EBX
 *  00430DEE   74 09            JE SHORT .00430DF9
 *  00430DF0   50               PUSH EAX
 *  00430DF1   E8 A6830000      CALL .0043919C
 *  00430DF6   83C4 04          ADD ESP,0x4
 *  00430DF9   5F               POP EDI
 *  00430DFA   5E               POP ESI
 *  00430DFB   B0 01            MOV AL,0x1
 *  00430DFD   5B               POP EBX
 *  00430DFE   8B4C24 08        MOV ECX,DWORD PTR SS:[ESP+0x8]
 *  00430E02   64:890D 00000000 MOV DWORD PTR FS:[0],ECX
 *  00430E09   83C4 14          ADD ESP,0x14
 *  00430E0C   C3               RETN
 *  00430E0D   3BC3             CMP EAX,EBX
 *  00430E0F   74 09            JE SHORT .00430E1A
 *  00430E11   50               PUSH EAX
 *  00430E12   E8 85830000      CALL .0043919C
 *  00430E17   83C4 04          ADD ESP,0x4
 *  00430E1A   8B4C24 14        MOV ECX,DWORD PTR SS:[ESP+0x14]
 *  00430E1E   5F               POP EDI
 *  00430E1F   5E               POP ESI
 *  00430E20   32C0             XOR AL,AL
 *  00430E22   5B               POP EBX
 *  00430E23   64:890D 00000000 MOV DWORD PTR FS:[0],ECX
 *  00430E2A   83C4 14          ADD ESP,0x14
 *  00430E2D   C3               RETN
 *  00430E2E   90               NOP
 *  00430E2F   90               NOP
 *  00430E30   CC               INT3
 *  00430E31   CC               INT3
 *  00430E32   CC               INT3
 *  00430E33   CC               INT3
 *  00430E34   CC               INT3
 */
bool attach(ulong startAddress, ulong stopAddress)
{
  const uint8_t bytes[] = {
    0x8d,0x74,0x32, 0x04,   // 0042fc21   8d7432 04        lea esi,dword ptr ds:[edx+esi+0x4]
    0xc1,0xe9, 0x02,        // 0042fc25   c1e9 02          shr ecx,0x2
    0xf3,0xa5               // 0042fc28   f3:a5            rep movs dword ptr es:[edi],dword ptr ds[esi]	; jichi: text accessed here from esi to edi
  };
  ulong addr2 = MemDbg::findBytes(bytes, sizeof(bytes), startAddress, stopAddress);
  if (!addr2)
    return false;

  // 0042FBF9   50               PUSH EAX    ; jichi: hook1, text length pushed, new function
  // 0042FBFA   894424 10        MOV DWORD PTR SS:[ESP+0x10],EAX ; jichi: text length, is this the memory allocation?
  // 0042FBFE   E8 1F840000      CALL lcsebody.00438022
  // 0042FC03   8B15 E8234A00    MOV EDX,DWORD PTR DS:[0x4A23E8]	; jichi: text here
  // 0042FC09   8B4C24 10        MOV ECX,DWORD PTR SS:[ESP+0x10]	; jichi: count is here
  // 0042FC0D   8B76 04          MOV ESI,DWORD PTR DS:[ESI+0x4]  ; jichi: [arg1+4]
  // 0042FC10   894424 14        MOV DWORD PTR SS:[ESP+0x14],EAX
  // 0042FC14   8B92 44290000    MOV EDX,DWORD PTR DS:[EDX+0x2944]	; jichi: base addr, [[0x4A23E8] + 0x2944]
  // 0042FC1A   8BF8             MOV EDI,EAX
  // 0042FC1C   8BC1             MOV EAX,ECX
  // 0042FC1E   83C4 04          ADD ESP,0x4
  //
  // 0042FC21   8D7432 04        LEA ESI,DWORD PTR DS:[EDX+ESI+0x4] ; jichi: hook2, text in esi
  // 0042FC25   C1E9 02          SHR ECX,0x2	; jichi: ecx is now the count, here, the rep function is blocked by 4 for performance
  // 0042FC28   F3:A5            REP MOVS DWORD PTR ES:[EDI],DWORD PTR DS[ESI]	; jichi: text accessed here from esi to edi
  ulong addr1 = addr2 + 0x0042fbf9 - 0x0042fc21;
  if (*(BYTE *)addr1 != 0x50) // push_eax
    return false;

  // 0042FC03   8B15 E8234A00    MOV EDX,DWORD PTR DS:[0x4A23E8]	; jichi: text here
  // 0042FC09   8B4C24 10        MOV ECX,DWORD PTR SS:[ESP+0x10]	; jichi: count is here
  // 0042FC0D   8B76 04          MOV ESI,DWORD PTR DS:[ESI+0x4]  ; jichi: [arg1+4]
  // 0042FC10   894424 14        MOV DWORD PTR SS:[ESP+0x14],EAX
  // 0042FC14   8B92 44290000    MOV EDX,DWORD PTR DS:[EDX+0x2944]	; jichi: offset addr, [[0x4A23E8] + 0x2944]
  {
    ulong addr = addr2 + 0x0042fc03 - 0x0042fc21;
    if (*(WORD *)addr != 0x158b) // 0042FC03   8B15 E8234A00    MOV EDX,DWORD PTR DS:[0x4A23E8]
      return false;
    addr += 2;
    Private::textBaseAddress_ = *(DWORD *)addr;
  }
  {
    ulong addr = addr2 + 0x0042fc14 - 0x0042fc21;
    if (*(WORD *)addr != 0x928b) // 0042FC14   8B92 44290000    MOV EDX,DWORD PTR DS:[EDX+0x2944]
      return false;
    addr += 2;
    Private::textOffset_ = *(DWORD *)addr;
  }

  return winhook::hook_before(addr1, Private::hook1)
      && winhook::hook_after(addr2, Private::hook2);
}
} // namespace ScenarioHook
} // unnamed namespace

bool LCScriptEngine::attach()
{
  ulong startAddress, stopAddress;
  if (!Engine::getProcessMemoryRange(&startAddress, &stopAddress))
    return false;
  if (!ScenarioHook::attach(startAddress, stopAddress))
    return false;
  HijackManager::instance()->attachFunction((ulong)::GetGlyphOutlineA);
  return true;
}

// EOF
