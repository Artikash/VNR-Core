// leaf.cc
// 6/24/2015 jichi
// See: http://bbs.sumisora.org/read.php?tid=11044256
// See also ATCode: http://capita.tistory.com/m/post/255
#include "engine/model/leaf.h"
#include "engine/enginecontroller.h"
#include "engine/enginedef.h"
#include "engine/enginehash.h"
#include "engine/engineutil.h"
//#include "hijack/hijackmanager.h"
#include "util/textutil.h"
#include "memdbg/memsearch.h"
#include "winhook/hookcode.h"
#include "winhook/hookcall.h"
#include <qt_windows.h>

#define DEBUG "leaf"
#include "sakurakit/skdebug.h"

namespace { // unnamed
namespace ScenarioHook {
namespace Private {

  QByteArray data_;

  bool hook1(winhook::hook_stack *s)
  {
    auto text = (LPCSTR)s->eax;
    if (Util::allAscii(text))
      return true;
    auto split = s->stack[0];
    auto role = split == 2 ? Engine::ScenarioRole :
                split == 0 ? Engine::NameRole :
                Engine::OtherRole; // split == 1
    auto sig = Engine::hashThreadSignature(role, split);
    QByteArray oldData = text,
               newData = EngineController::instance()->dispatchTextA(oldData, sig, role);
    if (newData.isEmpty() || newData == oldData) {
      data_.clear();
      return true;
    }
    data_ = newData;
    s->eax = (ulong)data_.constData();
    return true;
  }

  bool hook2(winhook::hook_stack *s)
  {
    if (!data_.isEmpty())
      s->ecx = (ulong)data_.constData();
    return true;
  }

} // namespace Private

/**
 *  Sample game: [141224] [AQUAPLUS] WHITE ALBUM2 ミニアフターストーリー
 *  Debug method: hardware break found text
 *  The text address is fixed.
 *  There are three matched functions.
 *  It can find both character name and scenario.
 *
 *  The scenario text contains "\n" or "\k".
 *
 *  0045145C   CC               INT3
 *  0045145D   CC               INT3
 *  0045145E   CC               INT3
 *  0045145F   CC               INT3
 *  00451460   D9EE             FLDZ
 *  00451462   56               PUSH ESI
 *  00451463   8B7424 08        MOV ESI,DWORD PTR SS:[ESP+0x8]
 *  00451467   D95E 0C          FSTP DWORD PTR DS:[ESI+0xC]
 *  0045146A   57               PUSH EDI
 *  0045146B   8BF9             MOV EDI,ECX
 *  0045146D   8B97 B0A00000    MOV EDX,DWORD PTR DS:[EDI+0xA0B0]
 *  00451473   33C0             XOR EAX,EAX
 *  00451475   3BD0             CMP EDX,EAX
 *  00451477   C706 05000000    MOV DWORD PTR DS:[ESI],0x5
 *  0045147D   C746 04 03000000 MOV DWORD PTR DS:[ESI+0x4],0x3
 *  00451484   8946 10          MOV DWORD PTR DS:[ESI+0x10],EAX
 *  00451487   8946 08          MOV DWORD PTR DS:[ESI+0x8],EAX
 *  0045148A   7F 0D            JG SHORT .00451499
 *  0045148C   8987 B0A00000    MOV DWORD PTR DS:[EDI+0xA0B0],EAX
 *  00451492   5F               POP EDI
 *  00451493   8BC6             MOV EAX,ESI
 *  00451495   5E               POP ESI
 *  00451496   C2 0400          RETN 0x4
 *  00451499   8D0492           LEA EAX,DWORD PTR DS:[EDX+EDX*4]
 *  0045149C   53               PUSH EBX
 *  0045149D   8B9C87 B08C0000  MOV EBX,DWORD PTR DS:[EDI+EAX*4+0x8CB0]
 *  004514A4   8D0487           LEA EAX,DWORD PTR DS:[EDI+EAX*4]
 *  004514A7   55               PUSH EBP
 *  004514A8   8D6B FF          LEA EBP,DWORD PTR DS:[EBX-0x1]
 *  004514AB   B9 04000000      MOV ECX,0x4
 *  004514B0   3BE9             CMP EBP,ECX
 *  004514B2   0F87 10020000    JA .004516C8
 *  004514B8   FF24AD E8164500  JMP DWORD PTR DS:[EBP*4+0x4516E8]
 *  004514BF   8B80 C08C0000    MOV EAX,DWORD PTR DS:[EAX+0x8CC0]
 *  004514C5   8D0480           LEA EAX,DWORD PTR DS:[EAX+EAX*4]
 *  004514C8   03C0             ADD EAX,EAX
 *  004514CA   0FBE9400 6416BC0>MOVSX EDX,BYTE PTR DS:[EAX+EAX+0xBC1664]
 *  004514D2   03C0             ADD EAX,EAX
 *  004514D4   8D5A FF          LEA EBX,DWORD PTR DS:[EDX-0x1]
 *  004514D7   3BD9             CMP EBX,ECX
 *  004514D9   0F87 B9000000    JA .00451598
 *  004514DF   FF249D FC164500  JMP DWORD PTR DS:[EBX*4+0x4516FC]
 *  004514E6   0FB688 6516BC00  MOVZX ECX,BYTE PTR DS:[EAX+0xBC1665]
 *  004514ED   FF8F B0A00000    DEC DWORD PTR DS:[EDI+0xA0B0]
 *  004514F3   5D               POP EBP
 *  004514F4   5B               POP EBX
 *  004514F5   5F               POP EDI
 *  004514F6   894E 10          MOV DWORD PTR DS:[ESI+0x10],ECX
 *  004514F9   8BC6             MOV EAX,ESI
 *  004514FB   5E               POP ESI
 *  004514FC   C2 0400          RETN 0x4
 *  004514FF   0FBF90 6616BC00  MOVSX EDX,WORD PTR DS:[EAX+0xBC1666]
 *  00451506   FF8F B0A00000    DEC DWORD PTR DS:[EDI+0xA0B0]
 *  0045150C   5D               POP EBP
 *  0045150D   5B               POP EBX
 *  0045150E   5F               POP EDI
 *  0045150F   8956 10          MOV DWORD PTR DS:[ESI+0x10],EDX
 *  00451512   8BC6             MOV EAX,ESI
 *  00451514   5E               POP ESI
 *  00451515   C2 0400          RETN 0x4
 *  00451518   8B80 6816BC00    MOV EAX,DWORD PTR DS:[EAX+0xBC1668]
 *  0045151E   FF8F B0A00000    DEC DWORD PTR DS:[EDI+0xA0B0]
 *  00451524   5D               POP EBP
 *  00451525   5B               POP EBX
 *  00451526   8946 10          MOV DWORD PTR DS:[ESI+0x10],EAX
 *  00451529   5F               POP EDI
 *  0045152A   8BC6             MOV EAX,ESI
 *  0045152C   5E               POP ESI
 *  0045152D   C2 0400          RETN 0x4
 *  00451530   D980 6C16BC00    FLD DWORD PTR DS:[EAX+0xBC166C]
 *  00451536   FF8F B0A00000    DEC DWORD PTR DS:[EDI+0xA0B0]
 *  0045153C   5D               POP EBP
 *  0045153D   D95E 0C          FSTP DWORD PTR DS:[ESI+0xC]
 *  00451540   5B               POP EBX
 *  00451541   5F               POP EDI
 *  00451542   894E 04          MOV DWORD PTR DS:[ESI+0x4],ECX
 *  00451545   8BC6             MOV EAX,ESI
 *  00451547   5E               POP ESI
 *  00451548   C2 0400          RETN 0x4
 *  0045154B   8B80 7016BC00    MOV EAX,DWORD PTR DS:[EAX+0xBC1670]
 *  00451551   8D58 01          LEA EBX,DWORD PTR DS:[EAX+0x1]
 *  00451554   8A10             MOV DL,BYTE PTR DS:[EAX]
 *  00451556   40               INC EAX
 *  00451557   84D2             TEST DL,DL
 *  00451559  ^75 F9            JNZ SHORT .00451554
 *  0045155B   2BC3             SUB EAX,EBX
 *  0045155D   8D58 01          LEA EBX,DWORD PTR DS:[EAX+0x1]
 *  00451560   53               PUSH EBX
 *  00451561   6A 00            PUSH 0x0
 *  00451563   53               PUSH EBX
 *  00451564   6A 00            PUSH 0x0
 *  00451566   FF15 74104A00    CALL DWORD PTR DS:[0x4A1074]             ; kernel32.GetProcessHeap
 *  0045156C   50               PUSH EAX
 *  0045156D   FF15 B4104A00    CALL DWORD PTR DS:[0x4A10B4]             ; ntdll.RtlAllocateHeap
 *  00451573   50               PUSH EAX
 *  00451574   E8 373F0200      CALL .004754B0
 *  00451579   8B8F B0A00000    MOV ECX,DWORD PTR DS:[EDI+0xA0B0]
 *  0045157F   8D0C89           LEA ECX,DWORD PTR DS:[ECX+ECX*4]
 *  00451582   8B8C8F C08C0000  MOV ECX,DWORD PTR DS:[EDI+ECX*4+0x8CC0]
 *  00451589   8D1489           LEA EDX,DWORD PTR DS:[ECX+ECX*4]
 *  0045158C   8B0C95 7016BC00  MOV ECX,DWORD PTR DS:[EDX*4+0xBC1670]
 *  00451593   E9 0C010000      JMP .004516A4
 *  00451598   52               PUSH EDX
 *  00451599   68 A8644A00      PUSH .004A64A8
 *  0045159E   E9 2B010000      JMP .004516CE
 *  004515A3   8D9492 2D230000  LEA EDX,DWORD PTR DS:[EDX+EDX*4+0x232D]
 *  004515AA   8B1C97           MOV EBX,DWORD PTR DS:[EDI+EDX*4]
 *  004515AD   85DB             TEST EBX,EBX
 *  004515AF   0F8C 23010000    JL .004516D8
 *  004515B5   8B80 C08C0000    MOV EAX,DWORD PTR DS:[EAX+0x8CC0]
 *  004515BB   99               CDQ
 *  004515BC   BD 1A000000      MOV EBP,0x1A
 *  004515C1   F7FD             IDIV EBP
 *  004515C3   C1E2 04          SHL EDX,0x4
 *  004515C6   03D3             ADD EDX,EBX
 *  004515C8   85C0             TEST EAX,EAX
 *  004515CA   74 1C            JE SHORT .004515E8
 *  004515CC   D98497 34A70000  FLD DWORD PTR DS:[EDI+EDX*4+0xA734]
 *  004515D3   FF8F B0A00000    DEC DWORD PTR DS:[EDI+0xA0B0]
 *  004515D9   5D               POP EBP
 *  004515DA   D95E 0C          FSTP DWORD PTR DS:[ESI+0xC]
 *  004515DD   5B               POP EBX
 *  004515DE   5F               POP EDI
 *  004515DF   894E 04          MOV DWORD PTR DS:[ESI+0x4],ECX
 *  004515E2   8BC6             MOV EAX,ESI
 *  004515E4   5E               POP ESI
 *  004515E5   C2 0400          RETN 0x4
 *  004515E8   8B8497 B4A00000  MOV EAX,DWORD PTR DS:[EDI+EDX*4+0xA0B4]
 *  004515EF   FF8F B0A00000    DEC DWORD PTR DS:[EDI+0xA0B0]
 *  004515F5   5D               POP EBP
 *  004515F6   5B               POP EBX
 *  004515F7   8946 10          MOV DWORD PTR DS:[ESI+0x10],EAX
 *  004515FA   5F               POP EDI
 *  004515FB   8BC6             MOV EAX,ESI
 *  004515FD   5E               POP ESI
 *  004515FE   C2 0400          RETN 0x4
 *  00451601   8B88 C08C0000    MOV ECX,DWORD PTR DS:[EAX+0x8CC0]
 *  00451607   D980 BC8C0000    FLD DWORD PTR DS:[EAX+0x8CBC]
 *  0045160D   894E 10          MOV DWORD PTR DS:[ESI+0x10],ECX
 *  00451610   D95E 0C          FSTP DWORD PTR DS:[ESI+0xC]
 *  00451613   8B88 B88C0000    MOV ECX,DWORD PTR DS:[EAX+0x8CB8]
 *  00451619   894E 08          MOV DWORD PTR DS:[ESI+0x8],ECX
 *  0045161C   8D9492 2D230000  LEA EDX,DWORD PTR DS:[EDX+EDX*4+0x232D]
 *  00451623   8B0C97           MOV ECX,DWORD PTR DS:[EDI+EDX*4]
 *  00451626   894E 04          MOV DWORD PTR DS:[ESI+0x4],ECX
 *  00451629   33C9             XOR ECX,ECX
 *  0045162B   8988 B08C0000    MOV DWORD PTR DS:[EAX+0x8CB0],ECX
 *  00451631   8988 B48C0000    MOV DWORD PTR DS:[EAX+0x8CB4],ECX
 *  00451637   8988 B88C0000    MOV DWORD PTR DS:[EAX+0x8CB8],ECX
 *  0045163D   5D               POP EBP
 *  0045163E   8988 BC8C0000    MOV DWORD PTR DS:[EAX+0x8CBC],ECX
 *  00451644   8988 C08C0000    MOV DWORD PTR DS:[EAX+0x8CC0],ECX
 *  0045164A   FF8F B0A00000    DEC DWORD PTR DS:[EDI+0xA0B0]
 *  00451650   5B               POP EBX
 *  00451651   5F               POP EDI
 *  00451652   8BC6             MOV EAX,ESI
 *  00451654   5E               POP ESI
 *  00451655   C2 0400          RETN 0x4
 *  00451658   8B90 C08C0000    MOV EDX,DWORD PTR DS:[EAX+0x8CC0]
 *  0045165E   8B8497 14080000  MOV EAX,DWORD PTR DS:[EDI+EDX*4+0x814]  ; jichi: text in eax, hook1 hook after here to replace eax
 *  00451665   8D58 01          LEA EBX,DWORD PTR DS:[EAX+0x1]  ; jichi: hook here would crash
 *  00451668   8A10             MOV DL,BYTE PTR DS:[EAX]        ; jichi: text accessed here in eax
 *  0045166A   40               INC EAX
 *  0045166B   84D2             TEST DL,DL
 *  0045166D  ^75 F9            JNZ SHORT .00451668
 *  0045166F   2BC3             SUB EAX,EBX     ; jichi: hook here, text in ebx-1
 *  00451671   8D58 01          LEA EBX,DWORD PTR DS:[EAX+0X1]
 *  00451674   53               PUSH EBX
 *  00451675   6A 00            PUSH 0x0
 *  00451677   53               PUSH EBX
 *  00451678   6A 00            PUSH 0x0
 *  0045167A   FF15 74104A00    CALL DWORD PTR DS:[0x4A1074]             ; kernel32.GetProcessHeap
 *  00451680   50               PUSH EAX
 *  00451681   FF15 B4104A00    CALL DWORD PTR DS:[0x4A10B4]             ; ntdll.RtlAllocateHeap
 *  00451687   50               PUSH EAX
 *  00451688   E8 233E0200      CALL .004754B0
 *  0045168D   8B8F B0A00000    MOV ECX,DWORD PTR DS:[EDI+0xA0B0]
 *  00451693   8D0C89           LEA ECX,DWORD PTR DS:[ECX+ECX*4]
 *  00451696   8B948F C08C0000  MOV EDX,DWORD PTR DS:[EDI+ECX*4+0x8CC0]
 *  0045169D   8B8C97 14080000  MOV ECX,DWORD PTR DS:[EDI+EDX*4+0x814]  ; jichi: text in ecx, hook2 hook after here to replace ecx
 *  004516A4   53               PUSH EBX
 *  004516A5   51               PUSH ECX
 *  004516A6   50               PUSH EAX
 *  004516A7   8946 08          MOV DWORD PTR DS:[ESI+0x8],EAX
 *  004516AA   E8 31410200      CALL .004757E0
 *  004516AF   83C4 18          ADD ESP,0x18
 *  004516B2   FF8F B0A00000    DEC DWORD PTR DS:[EDI+0xA0B0]
 *  004516B8   5D               POP EBP
 *  004516B9   5B               POP EBX
 *  004516BA   5F               POP EDI
 *  004516BB   C746 04 05000000 MOV DWORD PTR DS:[ESI+0x4],0x5
 *  004516C2   8BC6             MOV EAX,ESI
 *  004516C4   5E               POP ESI
 *  004516C5   C2 0400          RETN 0x4
 *  004516C8   53               PUSH EBX
 *  004516C9   68 8C644A00      PUSH .004A648C
 *  004516CE   6A 00            PUSH 0x0
 *  004516D0   E8 6BABFFFF      CALL .0044C240
 *  004516D5   83C4 0C          ADD ESP,0xC
 *  004516D8   FF8F B0A00000    DEC DWORD PTR DS:[EDI+0xA0B0]
 *  004516DE   5D               POP EBP
 *  004516DF   5B               POP EBX
 *  004516E0   5F               POP EDI
 *  004516E1   8BC6             MOV EAX,ESI
 *  004516E3   5E               POP ESI
 *  004516E4   C2 0400          RETN 0x4
 *  004516E7   90               NOP
 *  004516E8   BF 144500A3      MOV EDI,0xA3004514
 *  004516ED   15 45005816      ADC EAX,0x16580045
 *  004516F2   45               INC EBP
 *  004516F3   00C8             ADD AL,CL
 *  004516F5   16               PUSH SS
 *  004516F6   45               INC EBP
 *  004516F7   0001             ADD BYTE PTR DS:[ECX],AL
 *  004516F9   16               PUSH SS
 *  004516FA   45               INC EBP
 *  004516FB   00E6             ADD DH,AH
 *  004516FD   14 45            ADC AL,0x45
 *  004516FF   00FF             ADD BH,BH
 *  00451701   14 45            ADC AL,0x45
 *  00451703   0018             ADD BYTE PTR DS:[EAX],BL
 *  00451705   15 45003015      ADC EAX,0x15300045
 *  0045170A   45               INC EBP
 *  0045170B   004B 15          ADD BYTE PTR DS:[EBX+0x15],CL
 *  0045170E   45               INC EBP
 *  0045170F   0083 7C240800    ADD BYTE PTR DS:[EBX+0x8247C],AL
 *  00451715   56               PUSH ESI
 *  00451716   8BF1             MOV ESI,ECX
 *  00451718   74 29            JE SHORT .00451743
 *  0045171A   8B86 B0A00000    MOV EAX,DWORD PTR DS:[ESI+0xA0B0]
 *  00451720   3D FF000000      CMP EAX,0xFF
 *  00451725   7C 15            JL SHORT .0045173C
 *  00451727   68 74644A00      PUSH .004A6474
 *  0045172C   6A 00            PUSH 0x0
 *  0045172E   E8 0DABFFFF      CALL .0044C240
 *  00451733   83C4 08          ADD ESP,0x8
 *  00451736   33C0             XOR EAX,EAX
 *  00451738   5E               POP ESI
 *  00451739   C2 0800          RETN 0x8
 *  0045173C   40               INC EAX
 *  0045173D   8986 B0A00000    MOV DWORD PTR DS:[ESI+0xA0B0],EAX
 *  00451743   8B86 B0A00000    MOV EAX,DWORD PTR DS:[ESI+0xA0B0]
 *  00451749   8D0C80           LEA ECX,DWORD PTR DS:[EAX+EAX*4]
 *  0045174C   8D0C8E           LEA ECX,DWORD PTR DS:[ESI+ECX*4]
 *  0045174F   57               PUSH EDI
 *  00451750   8BB9 B08C0000    MOV EDI,DWORD PTR DS:[ECX+0x8CB0]
 *  00451756   8BD7             MOV EDX,EDI
 *  00451758   83EA 01          SUB EDX,0x1
 *  0045175B   74 70            JE SHORT .004517CD
 *  0045175D   83EA 01          SUB EDX,0x1
 *  00451760   74 1A            JE SHORT .0045177C
 *  00451762   57               PUSH EDI
 *  00451763   68 CC644A00      PUSH .004A64CC
 *  00451768   6A 00            PUSH 0x0
 *  0045176A   E8 D1AAFFFF      CALL .0044C240
 *  0045176F   83C4 0C          ADD ESP,0xC
 *  00451772   5F               POP EDI
 *  00451773   B8 01000000      MOV EAX,0x1
 *  00451778   5E               POP ESI
 *  00451779   C2 0800          RETN 0x8
 *  0045177C   8D9480 2D230000  LEA EDX,DWORD PTR DS:[EAX+EAX*4+0x232D]
 *  00451783   8B3C96           MOV EDI,DWORD PTR DS:[ESI+EDX*4]
 *  00451786   85FF             TEST EDI,EDI
 *  00451788   0F8C C8000000    JL .00451856
 *  0045178E   8B81 C08C0000    MOV EAX,DWORD PTR DS:[ECX+0x8CC0]
 *  00451794   99               CDQ
 *  00451795   B9 1A000000      MOV ECX,0x1A
 *  0045179A   F7F9             IDIV ECX
 *  0045179C   C1E2 04          SHL EDX,0x4
 *  0045179F   03D7             ADD EDX,EDI
 *  004517A1   85C0             TEST EAX,EAX
 *  004517A3   74 13            JE SHORT .004517B8
 *  004517A5   DB4424 0C        FILD DWORD PTR SS:[ESP+0xC]
 *  004517A9   5F               POP EDI
 *  004517AA   8D41 E7          LEA EAX,DWORD PTR DS:[ECX-0x19]
 *  004517AD   D99C96 34A70000  FSTP DWORD PTR DS:[ESI+EDX*4+0xA734]
 *  004517B4   5E               POP ESI
 *  004517B5   C2 0800          RETN 0x8
 *  004517B8   8B4424 0C        MOV EAX,DWORD PTR SS:[ESP+0xC]
 *  004517BC   898496 B4A00000  MOV DWORD PTR DS:[ESI+EDX*4+0xA0B4],EAX
 *  004517C3   5F               POP EDI
 *  004517C4   B8 01000000      MOV EAX,0x1
 *  004517C9   5E               POP ESI
 *  004517CA   C2 0800          RETN 0x8
 *  004517CD   8B89 C08C0000    MOV ECX,DWORD PTR DS:[ECX+0x8CC0]
 *  004517D3   8D0489           LEA EAX,DWORD PTR DS:[ECX+ECX*4]
 *  004517D6   03C0             ADD EAX,EAX
 *  004517D8   0FBE9400 6416BC0>MOVSX EDX,BYTE PTR DS:[EAX+EAX+0xBC1664]
 *  004517E0   03C0             ADD EAX,EAX
 *  004517E2   8D7A FF          LEA EDI,DWORD PTR DS:[EDX-0x1]
 *  004517E5   83FF 04          CMP EDI,0x4
 *  004517E8   77 41            JA SHORT .0045182B
 *  004517EA   FF24BD 60184500  JMP DWORD PTR DS:[EDI*4+0x451860]
 *  004517F1   8A4C24 0C        MOV CL,BYTE PTR SS:[ESP+0xC]
 *  004517F5   8888 6516BC00    MOV BYTE PTR DS:[EAX+0xBC1665],CL
 *  004517FB   EB 3E            JMP SHORT .0045183B
 *  004517FD   66:8B5424 0C     MOV DX,WORD PTR SS:[ESP+0xC]
 *  00451802   66:8990 6616BC00 MOV WORD PTR DS:[EAX+0xBC1666],DX
 *  00451809   EB 30            JMP SHORT .0045183B
 *  0045180B   8B4C24 0C        MOV ECX,DWORD PTR SS:[ESP+0xC]
 *  0045180F   8988 6816BC00    MOV DWORD PTR DS:[EAX+0xBC1668],ECX
 *  00451815   EB 24            JMP SHORT .0045183B
 *  00451817   DB4424 0C        FILD DWORD PTR SS:[ESP+0xC]
 *  0045181B   D998 6C16BC00    FSTP DWORD PTR DS:[EAX+0xBC166C]
 *  00451821   EB 18            JMP SHORT .0045183B
 *  00451823   51               PUSH ECX
 *  00451824   68 BC644A00      PUSH .004A64BC
 *  00451829   EB 06            JMP SHORT .00451831
 *  0045182B   52               PUSH EDX
 *  0045182C   68 A8644A00      PUSH .004A64A8
 *  00451831   6A 00            PUSH 0x0
 *  00451833   E8 08AAFFFF      CALL .0044C240
 *  00451838   83C4 0C          ADD ESP,0xC
 *  0045183B   8B86 B0A00000    MOV EAX,DWORD PTR DS:[ESI+0xA0B0]
 *  00451841   8D1480           LEA EDX,DWORD PTR DS:[EAX+EAX*4]
 *  00451844   8B8496 C08C0000  MOV EAX,DWORD PTR DS:[ESI+EDX*4+0x8CC0]
 *  0045184B   6A 00            PUSH 0x0
 *  0045184D   50               PUSH EAX
 *  0045184E   E8 FDF0FFFF      CALL .00450950
 *  00451853   83C4 08          ADD ESP,0x8
 *  00451856   5F               POP EDI
 *  00451857   B8 01000000      MOV EAX,0x1
 *  0045185C   5E               POP ESI
 *  0045185D   C2 0800          RETN 0x8
 *  00451860   F1               INT1
 *  00451861   17               POP SS                                   ; Modification of segment register
 *  00451862   45               INC EBP
 *  00451863   00FD             ADD CH,BH
 *  00451865   17               POP SS                                   ; Modification of segment register
 *  00451866   45               INC EBP
 *  00451867   000B             ADD BYTE PTR DS:[EBX],CL
 *  00451869   1845 00          SBB BYTE PTR SS:[EBP],AL
 *  0045186C   17               POP SS                                   ; Modification of segment register
 *  0045186D   1845 00          SBB BYTE PTR SS:[EBP],AL
 *  00451870   2318             AND EBX,DWORD PTR DS:[EAX]
 *  00451872   45               INC EBP
 *  00451873   00CC             ADD AH,CL
 *  00451875   CC               INT3
 *  00451876   CC               INT3
 *  00451877   CC               INT3
 *  00451878   CC               INT3
 *  00451879   CC               INT3
 *  0045187A   CC               INT3
 *  0045187B   CC               INT3
 *  0045187C   CC               INT3
 *  0045187D   CC               INT3
 *
 *  EAX 00000038
 *  ECX 00000004 ; jichi: fixed
 *  EDX 00000000 ; jichi: fixed
 *  EBX 00321221
 *  ESP 0012FD98
 *  EBP 00000002
 *  ESI 0012FDC4
 *  EDI 079047E0
 *  EIP 00451671 .00451671
 *
 *  Name thread:
 *  EAX 0019EE28
 *  ECX 00000004
 *  EDX 00000004
 *  EBX 00000003
 *  ESP 0012FD98
 *  EBP 00000002
 *  ESI 0012FDC4
 *  EDI 078C31D8
 *  EIP 00451665 .00451665
 *
 *  0012FD98   00000000 ; jichi: used as split
 *  0012FD9C   00000000
 *  0012FDA0   00000000
 *  0012FDA4   078C31D8
 *  0012FDA8   00453F12  RETURN to .00453F12 from .00451460
 *  0012FDAC   0012FDC4
 *  0012FDB0   708626E0  winmm.timeGetTime
 *  0012FDB4   078C31D8
 *  0012FDB8   00000003
 *
 *  Scenario thread:
 *  EAX 078C31EC
 *  ECX 00000004
 *  EDX 00000005
 *  EBX 00000003
 *  ESP 0012FD98
 *  EBP 00000002
 *  ESI 0012FDC4
 *  EDI 078C31D8
 *  EIP 0045165E .0045165E
 *
 *  0012FD98   00000002 ; jichi: used as split
 *  0012FD9C   00000000
 *  0012FDA0   00000000
 *  0012FDA4   078C31D8
 *  0012FDA8   00453F12  RETURN to .00453F12 from .00451460
 *  0012FDAC   0012FDC4
 *  0012FDB0   00000003
 *  0012FDB4   078C31D8
 *  0012FDB8   00000003
 *  0012FDBC   00000000
 *  0012FDC0   00000002
 */
bool attach() // attach scenario
{
  ulong startAddress, stopAddress;
  if (!Engine::getProcessMemoryRange(&startAddress, &stopAddress))
    return false;

  // 0045165E   8B8497 14080000  MOV EAX,DWORD PTR DS:[EDI+EDX*4+0x814]  ; jichi: text in eax, hook1 hook after here to replace eax
  // 0045169D   8B8C97 14080000  MOV ECX,DWORD PTR DS:[EDI+EDX*4+0x814]  ; jichi: text in ecx, hook2 hook after here to replace ecx
  const BYTE bytes1[] = { 0x8b,0x84,0x97, 0x14,0x08,0x00,0x00 },
             bytes2[] = { 0x8b,0x8c,0x97, 0x14,0x08,0x00,0x00 };

  ulong addr1 = MemDbg::findBytes(bytes1, sizeof(bytes1), startAddress, stopAddress),
        addr2 = MemDbg::findBytes(bytes2, sizeof(bytes2), startAddress, stopAddress);

  if (!addr1 || !addr2)
    return false;

  return winhook::hook_after(addr1, Private::hook1)
      && winhook::hook_after(addr2, Private::hook2);
}

} // namespace ScenarioHook

} // unnamed namespace

/** Public class */

bool LeafEngine::attach()
{
  if (!ScenarioHook::attach())
    return false;
  //HijackManager::instance()->attachFunction((ulong)::MultiByteToWideChar);
  return true;
}

QString LeafEngine::textFilter(const QString &text, int role)
{
  if (role != Engine::ScenarioRole || !text.contains('\\'))
    return text;
  return QString(text)
      .remove("\\k")
      .replace("\\n", "\n");
}

QString LeafEngine::translationFilter(const QString &text, int role)
{
  if (role != Engine::ScenarioRole || !text.contains('\n'))
    return text;
  return QString(text).replace("\n", "\\n");
}

// EOF
