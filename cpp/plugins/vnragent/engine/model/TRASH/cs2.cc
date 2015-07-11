// cs2.cc
// 6/21/2015 jichi
#include "engine/model/cs2.h"
#include "engine/enginecontroller.h"
#include "engine/enginedef.h"
#include "engine/enginehash.h"
#include "engine/engineutil.h"
#include "hijack/hijackmanager.h"
#include "util/textutil.h"
#include "winhook/hookcode.h"
#include "memdbg/memsearch.h"
#include <qt_windows.h>

#define DEBUG "model/cs2"
#include "sakurakit/skdebug.h"

namespace { // unnamed

/**
 *  Example prefix to skip:
 *  03751294  81 40 5C 70 63 81 75 83 7B 83 4E 82 CC 8E AF 82  　\pc「ボクの識・
 */
template <typename strT>
strT ltrim(strT text)
{
  if (text && *text) {
    strT lastText = nullptr;
    while (lastText != text) {
      lastText = text;
      if (text[0] == 0x20)
        text++;
      if (text[0] == 0x81 && text[1] == 0x40) // skip space \u3000 (0x8140 in sjis)
        text += 2;
      if (text[0] == '\\') {
        text++;
        while (::islower(text[0]))
          text++;
      }
    }
  }
  return text;
}

namespace ScenarioHook {
namespace Private {
  bool hookBefore(winhook::hook_stack *s)
  {
    auto text = (LPSTR)s->stack[1]; // arg1
    text = ltrim(text);
    if (!text || !*text || Util::allAscii(text))
      return true;
    size_t size = ::strlen(text);
    auto role = text[size + 2] ? Engine::NameRole : Engine::ScenarioRole;
    auto split = s->stack[0]; // retaddr
    auto sig = Engine::hashThreadSignature(role, split);
    QByteArray data = EngineController::instance()->dispatchTextA(text, role, sig);
    ::strcpy(text, data.constData());
    return true;
  }
} // namespace Private

/**
 *  Sample game: イノセントガール
 *  See: http://capita.tistory.com/m/post/176
 *
 *  Scenario pattern: 56 8B 74 24 08 8B C6 57 8D 78 01 EB 03
 *
 *  0040264C   CC               INT3
 *  0040264D   CC               INT3
 *  0040264E   CC               INT3
 *  0040264F   CC               INT3
 *  00402650   56               PUSH ESI
 *  00402651   8B7424 08        MOV ESI,DWORD PTR SS:[ESP+0x8]
 *  00402655   8BC6             MOV EAX,ESI
 *  00402657   57               PUSH EDI
 *  00402658   8D78 01          LEA EDI,DWORD PTR DS:[EAX+0x1]
 *  0040265B   EB 03            JMP SHORT .00402660
 *  0040265D   8D49 00          LEA ECX,DWORD PTR DS:[ECX]
 *  00402660   8A10             MOV DL,BYTE PTR DS:[EAX]
 *  00402662   40               INC EAX
 *  00402663   84D2             TEST DL,DL
 *  00402665  ^75 F9            JNZ SHORT .00402660
 *  00402667   2BC7             SUB EAX,EDI
 *  00402669   50               PUSH EAX
 *  0040266A   56               PUSH ESI
 *  0040266B   E8 F0000000      CALL .00402760
 *  00402670   5F               POP EDI
 *  00402671   5E               POP ESI
 *  00402672   C2 0400          RETN 0x4
 *  00402675   CC               INT3
 *  00402676   CC               INT3
 *  00402677   CC               INT3
 *  00402678   CC               INT3
 *  00402679   CC               INT3
 */
bool attach(ulong startAddress, ulong stopAddress)
{
  const quint8 bytes[] = {
    0x56,                 // 00402650   56               push esi
    0x8b,0x74,0x24, 0x08, // 00402651   8b7424 08        mov esi,dword ptr ss:[esp+0x8]
    0x8b,0xc6,            // 00402655   8bc6             mov eax,esi
    0x57,                 // 00402657   57               push edi
    0x8d,0x78, 0x01,      // 00402658   8d78 01          lea edi,dword ptr ds:[eax+0x1]
    0xeb, 0x03            // 0040265b   eb 03            jmp short .00402660
  };
  ulong addr = MemDbg::findBytes(bytes, sizeof(bytes), startAddress, stopAddress);
  if (!addr)
    return false;
  return winhook::hook_before(addr, Private::hookBefore);
}

} // namespace ScenarioHook

namespace HistoryHook {
namespace Private {
  bool hookBefore(winhook::hook_stack *s)
  {
    auto text = (LPSTR)s->stack[1]; // arg1
    text = ltrim(text);
    if (!text || !*text)
      return true;
    enum { role = Engine::HistoryRole };
    auto split = s->stack[0]; // retaddr
    auto sig = Engine::hashThreadSignature(role, split);
    QByteArray data = EngineController::instance()->dispatchTextA(text, role, sig);
    ::strcpy(text, data.constData());
    return true;
  }
} // namespace Private

/**
 *  Sample game: イノセントガール
 *  See: http://capita.tistory.com/m/post/176
 *
 *  History pattern: 74 0C B8 6E 5C 00 00
 *
 *  0832D9E8   00509ADC  RETURN to .00509ADC from .00508450
 *  0832D9EC   03751294
 *  0832D9F0   0832DA2C
 *  0832D9F4   0832DA30
 *  0832D9F8   0832DA60
 *  0832D9FC   599E574D
 *  0832DA00   0450B1B0
 *  0832DA04   036962E2
 *  0832DA08   00000000
 *  0832DA0C   04562390
 *  0832DA10   FFFFFFFF
 *  0832DA14   0832DB34
 *  0832DA18   0832DC34
 *  0832DA1C   00000000
 *  0832DA20   00000002
 *  0832DA24   00000000
 *
 *  Text in arg1. Sample text:
 *
 *  03751294  81 40 5C 70 63 81 75 83 7B 83 4E 82 CC 8E AF 82  　\pc「ボクの識・
 *  037512A4  C1 82 C4 82 A2 82 E9 90 A2 8A 45 82 CD 81 40 83  ﾁている世界は　・
 *  037512B4  4C 83 7E 82 C6 93 AF 82 B6 82 BE 82 EB 82 A4 82  Lミと同じだろう・
 *  037512C4  A9 81 76 00 00 00 00 00 00 00 00 00 00 00 00 00  ｩ」.............
 *  037512D4  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
 *  037512E4  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
 *  037512F4  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
 *  03751304  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
 *  03751314  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
 *
 *  EAX 0832DA2C
 *  ECX 04562390
 *  EDX 0832DA30
 *  EBX 00000010
 *  ESP 0832D9E8
 *  EBP 00000001
 *  ESI 04562390
 *  EDI 03751294
 *  EIP 00508450 .00508450
 *
 *  0050844B   CC               INT3
 *  0050844C   CC               INT3
 *  0050844D   CC               INT3
 *  0050844E   CC               INT3
 *  0050844F   CC               INT3
 *  00508450   6A FF            PUSH -0x1
 *  00508452   68 89706500      PUSH .00657089
 *  00508457   64:A1 00000000   MOV EAX,DWORD PTR FS:[0]
 *  0050845D   50               PUSH EAX
 *  0050845E   81EC 7C050000    SUB ESP,0x57C
 *  00508464   A1 889A7D00      MOV EAX,DWORD PTR DS:[0x7D9A88]
 *  00508469   33C4             XOR EAX,ESP
 *  0050846B   898424 78050000  MOV DWORD PTR SS:[ESP+0x578],EAX
 *  00508472   53               PUSH EBX
 *  00508473   55               PUSH EBP
 *  00508474   56               PUSH ESI
 *  00508475   57               PUSH EDI
 *  00508476   A1 889A7D00      MOV EAX,DWORD PTR DS:[0x7D9A88]
 *  0050847B   33C4             XOR EAX,ESP
 *  0050847D   50               PUSH EAX
 *  0050847E   8D8424 90050000  LEA EAX,DWORD PTR SS:[ESP+0x590]
 *  00508485   64:A3 00000000   MOV DWORD PTR FS:[0],EAX
 *  0050848B   8B8424 A0050000  MOV EAX,DWORD PTR SS:[ESP+0x5A0]
 *  00508492   8B9424 A8050000  MOV EDX,DWORD PTR SS:[ESP+0x5A8]
 *  00508499   8BAC24 AC050000  MOV EBP,DWORD PTR SS:[ESP+0x5AC]
 *  005084A0   33DB             XOR EBX,EBX
 *  005084A2   8BF1             MOV ESI,ECX
 *  005084A4   8B8C24 A4050000  MOV ECX,DWORD PTR SS:[ESP+0x5A4]
 *  005084AB   894424 1C        MOV DWORD PTR SS:[ESP+0x1C],EAX
 *  005084AF   894C24 68        MOV DWORD PTR SS:[ESP+0x68],ECX
 *  005084B3   895424 5C        MOV DWORD PTR SS:[ESP+0x5C],EDX
 *  005084B7   895C24 2C        MOV DWORD PTR SS:[ESP+0x2C],EBX
 *  005084BB   895C24 64        MOV DWORD PTR SS:[ESP+0x64],EBX
 *  005084BF   C78424 88000000 >MOV DWORD PTR SS:[ESP+0x88],0xF
 *  005084CA   899C24 84000000  MOV DWORD PTR SS:[ESP+0x84],EBX
 *  005084D1   885C24 74        MOV BYTE PTR SS:[ESP+0x74],BL
 *  005084D5   8B46 10          MOV EAX,DWORD PTR DS:[ESI+0x10]
 *  005084D8   8B40 1C          MOV EAX,DWORD PTR DS:[EAX+0x1C]
 *  005084DB   8B8E F0000000    MOV ECX,DWORD PTR DS:[ESI+0xF0]
 *  005084E1   8BBE DC000000    MOV EDI,DWORD PTR DS:[ESI+0xDC]
 *  005084E7   03C8             ADD ECX,EAX
 *  005084E9   894C24 6C        MOV DWORD PTR SS:[ESP+0x6C],ECX
 *  005084ED   8D4C24 70        LEA ECX,DWORD PTR SS:[ESP+0x70]
 *  005084F1   899C24 98050000  MOV DWORD PTR SS:[ESP+0x598],EBX
 *  005084F8   895C24 24        MOV DWORD PTR SS:[ESP+0x24],EBX
 *  005084FC   895C24 60        MOV DWORD PTR SS:[ESP+0x60],EBX
 *  00508500   E8 1B60F2FF      CALL .0042E520
 *  00508505   F686 E0000000 02 TEST BYTE PTR DS:[ESI+0xE0],0x2
 *  0050850C   74 72            JE SHORT .00508580
 *  0050850E   395E 08          CMP DWORD PTR DS:[ESI+0x8],EBX
 *  00508511   75 6D            JNZ SHORT .00508580
 *  00508513   8BC5             MOV EAX,EBP
 *  00508515   C74424 58 0F0000>MOV DWORD PTR SS:[ESP+0x58],0xF
 *  0050851D   895C24 54        MOV DWORD PTR SS:[ESP+0x54],EBX
 *  00508521   885C24 44        MOV BYTE PTR SS:[ESP+0x44],BL
 *  00508525   8D50 01          LEA EDX,DWORD PTR DS:[EAX+0x1]
 *  00508528   8A08             MOV CL,BYTE PTR DS:[EAX]
 *  0050852A   40               INC EAX
 *  0050852B   3ACB             CMP CL,BL
 *  0050852D  ^75 F9            JNZ SHORT .00508528
 *  0050852F   2BC2             SUB EAX,EDX
 *  00508531   50               PUSH EAX
 *  00508532   55               PUSH EBP
 *  00508533   8D4C24 48        LEA ECX,DWORD PTR SS:[ESP+0x48]
 *  00508537   E8 24A2EFFF      CALL .00402760
 *  0050853C   6A FF            PUSH -0x1
 *  0050853E   53               PUSH EBX
 *  0050853F   8D5424 48        LEA EDX,DWORD PTR SS:[ESP+0x48]
 *  00508543   52               PUSH EDX
 *  00508544   8D4C24 7C        LEA ECX,DWORD PTR SS:[ESP+0x7C]
 *  00508548   C68424 A4050000 >MOV BYTE PTR SS:[ESP+0x5A4],0x1
 *  00508550   E8 1BA0EFFF      CALL .00402570
 *  00508555   837C24 58 10     CMP DWORD PTR SS:[ESP+0x58],0x10
 *  0050855A   889C24 98050000  MOV BYTE PTR SS:[ESP+0x598],BL
 *  00508561   72 0D            JB SHORT .00508570
 *  00508563   8B4424 44        MOV EAX,DWORD PTR SS:[ESP+0x44]
 *  00508567   50               PUSH EAX
 *  00508568   E8 05620A00      CALL .005AE772
 *  0050856D   83C4 04          ADD ESP,0x4
 *  00508570   8BC5             MOV EAX,EBP
 *  00508572   8D50 01          LEA EDX,DWORD PTR DS:[EAX+0x1]
 *  00508575   8A08             MOV CL,BYTE PTR DS:[EAX]
 *  00508577   40               INC EAX
 *  00508578   3ACB             CMP CL,BL
 *  0050857A  ^75 F9            JNZ SHORT .00508575
 *  0050857C   2BC2             SUB EAX,EDX
 *  0050857E   2BF8             SUB EDI,EAX
 *  00508580   8B4424 1C        MOV EAX,DWORD PTR SS:[ESP+0x1C]
 *  00508584   8D4C24 28        LEA ECX,DWORD PTR SS:[ESP+0x28]
 *  00508588   51               PUSH ECX
 *  00508589   8D5424 1C        LEA EDX,DWORD PTR SS:[ESP+0x1C]
 *  0050858D   52               PUSH EDX
 *  0050858E   50               PUSH EAX
 *  0050858F   8BCE             MOV ECX,ESI
 *  00508591   E8 FAEBFFFF      CALL .00507190
 *  00508596   85C0             TEST EAX,EAX
 *  00508598   74 4E            JE SHORT .005085E8
 *  0050859A   8B96 98000000    MOV EDX,DWORD PTR DS:[ESI+0x98]
 *  005085A0   8B86 80000000    MOV EAX,DWORD PTR DS:[ESI+0x80]
 *  005085A6   8D8E 80000000    LEA ECX,DWORD PTR DS:[ESI+0x80]
 *  005085AC   895424 3C        MOV DWORD PTR SS:[ESP+0x3C],EDX
 *  005085B0   8D5424 18        LEA EDX,DWORD PTR SS:[ESP+0x18]
 *  005085B4   894424 38        MOV DWORD PTR SS:[ESP+0x38],EAX
 *  005085B8   52               PUSH EDX
 *  005085B9   8D4424 34        LEA EAX,DWORD PTR SS:[ESP+0x34]
 *  005085BD   50               PUSH EAX
 *  005085BE   E8 6D190000      CALL .00509F30
 *  005085C3   8BE8             MOV EBP,EAX
 *  005085C5   8B45 00          MOV EAX,DWORD PTR SS:[EBP]
 *  005085C8   3BC3             CMP EAX,EBX
 *  005085CA   74 06            JE SHORT .005085D2
 *  005085CC   3B4424 38        CMP EAX,DWORD PTR SS:[ESP+0x38]
 *  005085D0   74 05            JE SHORT .005085D7
 *  005085D2   E8 E1650A00      CALL .005AEBB8
 *  005085D7   8B4C24 3C        MOV ECX,DWORD PTR SS:[ESP+0x3C]
 *  005085DB   394D 04          CMP DWORD PTR SS:[EBP+0x4],ECX
 *  005085DE   74 08            JE SHORT .005085E8
 *  005085E0   8B5424 28        MOV EDX,DWORD PTR SS:[ESP+0x28]
 *  005085E4   895424 60        MOV DWORD PTR SS:[ESP+0x60],EDX
 *  005085E8   8B4424 1C        MOV EAX,DWORD PTR SS:[ESP+0x1C]
 *  005085EC   895C24 14        MOV DWORD PTR SS:[ESP+0x14],EBX
 *  005085F0   3818             CMP BYTE PTR DS:[EAX],BL
 *  005085F2   0F84 9E030000    JE .00508996
 *  005085F8   8D47 FF          LEA EAX,DWORD PTR DS:[EDI-0x1]
 *  005085FB   33C9             XOR ECX,ECX
 *  005085FD   3BC3             CMP EAX,EBX
 *  005085FF   895C24 38        MOV DWORD PTR SS:[ESP+0x38],EBX
 *  00508603   895C24 20        MOV DWORD PTR SS:[ESP+0x20],EBX
 *  00508607   7E 74            JLE SHORT .0050867D
 *  00508609   8D68 FF          LEA EBP,DWORD PTR DS:[EAX-0x1]
 *  0050860C   D1ED             SHR EBP,1
 *  0050860E   45               INC EBP
 *  0050860F   8D4C2D 00        LEA ECX,DWORD PTR SS:[EBP+EBP]
 *  00508613   894C24 30        MOV DWORD PTR SS:[ESP+0x30],ECX
 *  00508617   EB 07            JMP SHORT .00508620
 *  00508619   8DA424 00000000  LEA ESP,DWORD PTR SS:[ESP]
 *  00508620   6A 02            PUSH 0x2
 *  00508622   68 0CAC6600      PUSH .0066AC0C
 *  00508627   8D4C24 48        LEA ECX,DWORD PTR SS:[ESP+0x48]
 *  0050862B   C74424 60 0F0000>MOV DWORD PTR SS:[ESP+0x60],0xF
 *  00508633   895C24 5C        MOV DWORD PTR SS:[ESP+0x5C],EBX
 *  00508637   885C24 4C        MOV BYTE PTR SS:[ESP+0x4C],BL
 *  0050863B   E8 20A1EFFF      CALL .00402760
 *  00508640   6A FF            PUSH -0x1
 *  00508642   53               PUSH EBX
 *  00508643   8D5424 48        LEA EDX,DWORD PTR SS:[ESP+0x48]
 *  00508647   52               PUSH EDX
 *  00508648   8D4C24 7C        LEA ECX,DWORD PTR SS:[ESP+0x7C]
 *  0050864C   C68424 A4050000 >MOV BYTE PTR SS:[ESP+0x5A4],0x2
 *  00508654   E8 8759F2FF      CALL .0042DFE0
 *  00508659   837C24 58 10     CMP DWORD PTR SS:[ESP+0x58],0x10
 *  0050865E   889C24 98050000  MOV BYTE PTR SS:[ESP+0x598],BL
 *  00508665   72 0D            JB SHORT .00508674
 *  00508667   8B4424 44        MOV EAX,DWORD PTR SS:[ESP+0x44]
 *  0050866B   50               PUSH EAX
 *  0050866C   E8 01610A00      CALL .005AE772
 *  00508671   83C4 04          ADD ESP,0x4
 *  00508674   83ED 01          SUB EBP,0x1
 *  00508677  ^75 A7            JNZ SHORT .00508620
 *  00508679   8B4C24 30        MOV ECX,DWORD PTR SS:[ESP+0x30]
 *  0050867D   3BCF             CMP ECX,EDI
 *  0050867F   7D 5B            JGE SHORT .005086DC
 *  00508681   2BF9             SUB EDI,ECX
 *  00508683   6A 01            PUSH 0x1
 *  00508685   68 10AC6600      PUSH .0066AC10
 *  0050868A   8D4C24 48        LEA ECX,DWORD PTR SS:[ESP+0x48]
 *  0050868E   C74424 60 0F0000>MOV DWORD PTR SS:[ESP+0x60],0xF
 *  00508696   895C24 5C        MOV DWORD PTR SS:[ESP+0x5C],EBX
 *  0050869A   885C24 4C        MOV BYTE PTR SS:[ESP+0x4C],BL
 *  0050869E   E8 BDA0EFFF      CALL .00402760
 *  005086A3   6A FF            PUSH -0x1
 *  005086A5   53               PUSH EBX
 *  005086A6   8D4C24 48        LEA ECX,DWORD PTR SS:[ESP+0x48]
 *  005086AA   51               PUSH ECX
 *  005086AB   8D4C24 7C        LEA ECX,DWORD PTR SS:[ESP+0x7C]
 *  005086AF   C68424 A4050000 >MOV BYTE PTR SS:[ESP+0x5A4],0x3
 *  005086B7   E8 2459F2FF      CALL .0042DFE0
 *  005086BC   837C24 58 10     CMP DWORD PTR SS:[ESP+0x58],0x10
 *  005086C1   889C24 98050000  MOV BYTE PTR SS:[ESP+0x598],BL
 *  005086C8   72 0D            JB SHORT .005086D7
 *  005086CA   8B5424 44        MOV EDX,DWORD PTR SS:[ESP+0x44]
 *  005086CE   52               PUSH EDX
 *  005086CF   E8 9E600A00      CALL .005AE772
 *  005086D4   83C4 04          ADD ESP,0x4
 *  005086D7   83EF 01          SUB EDI,0x1
 *  005086DA  ^75 A7            JNZ SHORT .00508683
 *  005086DC   8BAC24 84000000  MOV EBP,DWORD PTR SS:[ESP+0x84]
 *  005086E3   3B2E             CMP EBP,DWORD PTR DS:[ESI]
 *  005086E5   8B4424 14        MOV EAX,DWORD PTR SS:[ESP+0x14]
 *  005086E9   894424 30        MOV DWORD PTR SS:[ESP+0x30],EAX
 *  005086ED   0F8D D8000000    JGE .005087CB
 *  005086F3   8B4C24 1C        MOV ECX,DWORD PTR SS:[ESP+0x1C]
 *  005086F7   8B5424 14        MOV EDX,DWORD PTR SS:[ESP+0x14]
 *  005086FB   8D3C11           LEA EDI,DWORD PTR DS:[ECX+EDX]
 *  005086FE   8D8424 8C040000  LEA EAX,DWORD PTR SS:[ESP+0x48C]
 *  00508705   50               PUSH EAX
 *  00508706   8D8C24 90000000  LEA ECX,DWORD PTR SS:[ESP+0x90]
 *  0050870D   51               PUSH ECX
 *  0050870E   57               PUSH EDI
 *  0050870F   8BCE             MOV ECX,ESI
 *  00508711   E8 6AEBFFFF      CALL .00507280
 *  00508716   3BC3             CMP EAX,EBX
 *  00508718   7E 34            JLE SHORT .0050874E
 *  0050871A   8D8C24 8C000000  LEA ECX,DWORD PTR SS:[ESP+0x8C]
 *  00508721   8D79 01          LEA EDI,DWORD PTR DS:[ECX+0x1]
 *  00508724   8A11             MOV DL,BYTE PTR DS:[ECX]
 *  00508726   41               INC ECX
 *  00508727   3AD3             CMP DL,BL
 *  00508729  ^75 F9            JNZ SHORT .00508724
 *  0050872B   8B5424 20        MOV EDX,DWORD PTR SS:[ESP+0x20]
 *  0050872F   2BCF             SUB ECX,EDI
 *  00508731   8B7E 04          MOV EDI,DWORD PTR DS:[ESI+0x4]
 *  00508734   03D1             ADD EDX,ECX
 *  00508736   8B0E             MOV ECX,DWORD PTR DS:[ESI]
 *  00508738   895424 20        MOV DWORD PTR SS:[ESP+0x20],EDX
 *  0050873C   03F9             ADD EDI,ECX
 *  0050873E   03D5             ADD EDX,EBP
 *  00508740   3BD7             CMP EDX,EDI
 *  00508742   0F8F 83000000    JG .005087CB
 *  00508748   014424 14        ADD DWORD PTR SS:[ESP+0x14],EAX
 *  0050874C   EB 54            JMP SHORT .005087A2
 *  0050874E   8D4424 28        LEA EAX,DWORD PTR SS:[ESP+0x28]
 *  00508752   50               PUSH EAX
 *  00508753   8D4C24 1C        LEA ECX,DWORD PTR SS:[ESP+0x1C]
 *  00508757   51               PUSH ECX
 *  00508758   57               PUSH EDI
 *  00508759   8BCE             MOV ECX,ESI
 *  0050875B   E8 30EAFFFF      CALL .00507190
 *  00508760   85C0             TEST EAX,EAX
 *  00508762   74 67            JE SHORT .005087CB
 *  00508764   8B15 A4118200    MOV EDX,DWORD PTR DS:[0x8211A4]
 *  0050876A   399A 94690000    CMP DWORD PTR DS:[EDX+0x6994],EBX
 *  00508770   74 0C            JE SHORT .0050877E  ; jichi: pattern found here
 *  00508772   B8 6E5C0000      MOV EAX,0x5C6E
 *  00508777   66:394424 18     CMP WORD PTR SS:[ESP+0x18],AX
 *  0050877C   74 35            JE SHORT .005087B3
 *  0050877E   8B5424 20        MOV EDX,DWORD PTR SS:[ESP+0x20]
 *  00508782   8B4424 28        MOV EAX,DWORD PTR SS:[ESP+0x28]
 *  00508786   8B0E             MOV ECX,DWORD PTR DS:[ESI]
 *  00508788   03D5             ADD EDX,EBP
 *  0050878A   03D0             ADD EDX,EAX
 *  0050878C   3BD1             CMP EDX,ECX
 *  0050878E   7D 3B            JGE SHORT .005087CB
 *  00508790   8B5424 14        MOV EDX,DWORD PTR SS:[ESP+0x14]
 *  00508794   895424 24        MOV DWORD PTR SS:[ESP+0x24],EDX
 *  00508798   03D0             ADD EDX,EAX
 *  0050879A   014424 20        ADD DWORD PTR SS:[ESP+0x20],EAX
 *  0050879E   895424 14        MOV DWORD PTR SS:[ESP+0x14],EDX
 *  005087A2   8B4424 20        MOV EAX,DWORD PTR SS:[ESP+0x20]
 *  005087A6   8D1428           LEA EDX,DWORD PTR DS:[EAX+EBP]
 *  005087A9   3BD1             CMP EDX,ECX
 *  005087AB  ^0F8C 42FFFFFF    JL .005086F3
 *  005087B1   EB 18            JMP SHORT .005087CB
 *  005087B3   8B4424 14        MOV EAX,DWORD PTR SS:[ESP+0x14]
 *  005087B7   894424 24        MOV DWORD PTR SS:[ESP+0x24],EAX
 *  005087BB   034424 28        ADD EAX,DWORD PTR SS:[ESP+0x28]
 *  005087BF   C74424 38 010000>MOV DWORD PTR SS:[ESP+0x38],0x1
 *  005087C7   894424 14        MOV DWORD PTR SS:[ESP+0x14],EAX
 *  005087CB   8B3E             MOV EDI,DWORD PTR DS:[ESI]
 *  005087CD   2BFD             SUB EDI,EBP
 *  005087CF   2B7C24 14        SUB EDI,DWORD PTR SS:[ESP+0x14]
 *  005087D3   8B6C24 1C        MOV EBP,DWORD PTR SS:[ESP+0x1C]
 *  005087D7   037E 04          ADD EDI,DWORD PTR DS:[ESI+0x4]
 *  005087DA   8D4424 24        LEA EAX,DWORD PTR SS:[ESP+0x24]
 *  005087DE   57               PUSH EDI
 *  005087DF   50               PUSH EAX
 *  005087E0   8D4C24 38        LEA ECX,DWORD PTR SS:[ESP+0x38]
 *  005087E4   51               PUSH ECX
 *  005087E5   55               PUSH EBP
 *  005087E6   8BCE             MOV ECX,ESI
 *  005087E8   E8 13F3FFFF      CALL .00507B00
 *  005087ED   57               PUSH EDI
 *  005087EE   8D5424 28        LEA EDX,DWORD PTR SS:[ESP+0x28]
 *  005087F2   52               PUSH EDX
 *  005087F3   8D4424 38        LEA EAX,DWORD PTR SS:[ESP+0x38]
 *  005087F7   50               PUSH EAX
 *  005087F8   55               PUSH EBP
 *  005087F9   8BCE             MOV ECX,ESI
 *  005087FB   E8 A0F3FFFF      CALL .00507BA0
 *  00508800   85C0             TEST EAX,EAX
 *  00508802   74 13            JE SHORT .00508817
 *  00508804   57               PUSH EDI
 *  00508805   8D4C24 28        LEA ECX,DWORD PTR SS:[ESP+0x28]
 *  00508809   51               PUSH ECX
 *  0050880A   8D5424 38        LEA EDX,DWORD PTR SS:[ESP+0x38]
 *  0050880E   52               PUSH EDX
 *  0050880F   55               PUSH EBP
 *  00508810   8BCE             MOV ECX,ESI
 *  00508812   E8 09F5FFFF      CALL .00507D20
 *  00508817   8B7C24 14        MOV EDI,DWORD PTR SS:[ESP+0x14]
 *  0050881B   8B6C24 30        MOV EBP,DWORD PTR SS:[ESP+0x30]
 *  0050881F   8B4C24 1C        MOV ECX,DWORD PTR SS:[ESP+0x1C]
 *  00508823   8BC7             MOV EAX,EDI
 *  00508825   2BC5             SUB EAX,EBP
 *  00508827   50               PUSH EAX
 *  00508828   03CD             ADD ECX,EBP
 *  0050882A   51               PUSH ECX
 *  0050882B   8D9424 94000000  LEA EDX,DWORD PTR SS:[ESP+0x94]
 *  00508832   52               PUSH EDX
 *  00508833   E8 A8430000      CALL .0050CBE0
 *  00508838   8B4424 44        MOV EAX,DWORD PTR SS:[ESP+0x44]
 *  0050883C   83C4 0C          ADD ESP,0xC
 *  0050883F   F7D8             NEG EAX
 *  00508841   1BC0             SBB EAX,EAX
 *  00508843   83E0 02          AND EAX,0x2
 *  00508846   2BF8             SUB EDI,EAX
 *  00508848   2BFD             SUB EDI,EBP
 *  0050884A   8D8424 8C000000  LEA EAX,DWORD PTR SS:[ESP+0x8C]
 *  00508851   889C3C 8C000000  MOV BYTE PTR SS:[ESP+EDI+0x8C],BL
 *  00508858   C74424 58 0F0000>MOV DWORD PTR SS:[ESP+0x58],0xF
 *  00508860   895C24 54        MOV DWORD PTR SS:[ESP+0x54],EBX
 *  00508864   885C24 44        MOV BYTE PTR SS:[ESP+0x44],BL
 *  00508868   8D78 01          LEA EDI,DWORD PTR DS:[EAX+0x1]
 *  0050886B   EB 03            JMP SHORT .00508870
 *  0050886D   8D49 00          LEA ECX,DWORD PTR DS:[ECX]
 *  00508870   8A08             MOV CL,BYTE PTR DS:[EAX]
 *  00508872   40               INC EAX
 *  00508873   3ACB             CMP CL,BL
 *  00508875  ^75 F9            JNZ SHORT .00508870
 *  00508877   2BC7             SUB EAX,EDI
 *  00508879   50               PUSH EAX
 *  0050887A   8D8424 90000000  LEA EAX,DWORD PTR SS:[ESP+0x90]
 *  00508881   50               PUSH EAX
 *  00508882   8D4C24 48        LEA ECX,DWORD PTR SS:[ESP+0x48]
 *  00508886   E8 D59EEFFF      CALL .00402760
 *  0050888B   6A FF            PUSH -0x1
 *  0050888D   53               PUSH EBX
 *  0050888E   8D4C24 48        LEA ECX,DWORD PTR SS:[ESP+0x48]
 *  00508892   51               PUSH ECX
 *  00508893   8D4C24 7C        LEA ECX,DWORD PTR SS:[ESP+0x7C]
 *  00508897   C68424 A4050000 >MOV BYTE PTR SS:[ESP+0x5A4],0x4
 *  0050889F   E8 3C57F2FF      CALL .0042DFE0
 *  005088A4   837C24 58 10     CMP DWORD PTR SS:[ESP+0x58],0x10
 *  005088A9   889C24 98050000  MOV BYTE PTR SS:[ESP+0x598],BL
 *  005088B0   72 0D            JB SHORT .005088BF
 *  005088B2   8B5424 44        MOV EDX,DWORD PTR SS:[ESP+0x44]
 *  005088B6   52               PUSH EDX
 *  005088B7   E8 B65E0A00      CALL .005AE772
 *  005088BC   83C4 04          ADD ESP,0x4
 *  005088BF   8B7E 14          MOV EDI,DWORD PTR DS:[ESI+0x14]
 *  005088C2   8B47 1C          MOV EAX,DWORD PTR DS:[EDI+0x1C]
 *  005088C5   40               INC EAX
 *  005088C6   3947 14          CMP DWORD PTR DS:[EDI+0x14],EAX
 *  005088C9   77 09            JA SHORT .005088D4
 *  005088CB   6A 01            PUSH 0x1
 *  005088CD   8BCF             MOV ECX,EDI
 *  005088CF   E8 2C1F0000      CALL .0050A800
 *  005088D4   8B6F 18          MOV EBP,DWORD PTR DS:[EDI+0x18]
 *  005088D7   036F 1C          ADD EBP,DWORD PTR DS:[EDI+0x1C]
 *  005088DA   8B47 14          MOV EAX,DWORD PTR DS:[EDI+0x14]
 *  005088DD   3BC5             CMP EAX,EBP
 *  005088DF   77 02            JA SHORT .005088E3
 *  005088E1   2BE8             SUB EBP,EAX
 *  005088E3   8B4F 10          MOV ECX,DWORD PTR DS:[EDI+0x10]
 *  005088E6   391CA9           CMP DWORD PTR DS:[ECX+EBP*4],EBX
 *  005088E9   75 10            JNZ SHORT .005088FB
 *  005088EB   6A 20            PUSH 0x20
 *  005088ED   E8 A65E0A00      CALL .005AE798
 *  005088F2   8B57 10          MOV EDX,DWORD PTR DS:[EDI+0x10]
 *  005088F5   83C4 04          ADD ESP,0x4
 *  005088F8   8904AA           MOV DWORD PTR DS:[EDX+EBP*4],EAX
 *  005088FB   8B47 10          MOV EAX,DWORD PTR DS:[EDI+0x10]
 *  005088FE   8B2CA8           MOV EBP,DWORD PTR DS:[EAX+EBP*4]
 *  00508901   896C24 38        MOV DWORD PTR SS:[ESP+0x38],EBP
 *  00508905   896C24 30        MOV DWORD PTR SS:[ESP+0x30],EBP
 *  00508909   C68424 98050000 >MOV BYTE PTR SS:[ESP+0x598],0x5
 *  00508911   3BEB             CMP EBP,EBX
 *  00508913   74 24            JE SHORT .00508939
 *  00508915   8B4C24 6C        MOV ECX,DWORD PTR SS:[ESP+0x6C]
 *  00508919   894D 00          MOV DWORD PTR SS:[EBP],ECX
 *  0050891C   6A FF            PUSH -0x1
 *  0050891E   8D4D 04          LEA ECX,DWORD PTR SS:[EBP+0x4]
 *  00508921   53               PUSH EBX
 *  00508922   8D5424 78        LEA EDX,DWORD PTR SS:[ESP+0x78]
 *  00508926   C741 18 0F000000 MOV DWORD PTR DS:[ECX+0x18],0xF
 *  0050892D   8959 14          MOV DWORD PTR DS:[ECX+0x14],EBX
 *  00508930   52               PUSH EDX
 *  00508931   8859 04          MOV BYTE PTR DS:[ECX+0x4],BL
 *  00508934   E8 379CEFFF      CALL .00402570
 *  00508939   B8 01000000      MOV EAX,0x1
 *  0050893E   0147 1C          ADD DWORD PTR DS:[EDI+0x1C],EAX
 *  00508941   889C24 98050000  MOV BYTE PTR SS:[ESP+0x598],BL
 *  00508948   395C24 64        CMP DWORD PTR SS:[ESP+0x64],EBX
 *  0050894C   75 12            JNZ SHORT .00508960
 *  0050894E   8B4E 14          MOV ECX,DWORD PTR DS:[ESI+0x14]
 *  00508951   8B51 1C          MOV EDX,DWORD PTR DS:[ECX+0x1C]
 *  00508954   8B4C24 68        MOV ECX,DWORD PTR SS:[ESP+0x68]
 *  00508958   2BD0             SUB EDX,EAX
 *  0050895A   8911             MOV DWORD PTR DS:[ECX],EDX
 *  0050895C   894424 64        MOV DWORD PTR SS:[ESP+0x64],EAX
 *  00508960   8B56 14          MOV EDX,DWORD PTR DS:[ESI+0x14]
 *  00508963   8B4A 1C          MOV ECX,DWORD PTR DS:[EDX+0x1C]
 *  00508966   8B5424 5C        MOV EDX,DWORD PTR SS:[ESP+0x5C]
 *  0050896A   014424 2C        ADD DWORD PTR SS:[ESP+0x2C],EAX
 *  0050896E   2BC8             SUB ECX,EAX
 *  00508970   890A             MOV DWORD PTR DS:[EDX],ECX
 *  00508972   8D4C24 70        LEA ECX,DWORD PTR SS:[ESP+0x70]
 *  00508976   E8 A55BF2FF      CALL .0042E520
 *  0050897B   8BBE DC000000    MOV EDI,DWORD PTR DS:[ESI+0xDC]
 *  00508981   8B4424 1C        MOV EAX,DWORD PTR SS:[ESP+0x1C]
 *  00508985   8B4C24 14        MOV ECX,DWORD PTR SS:[ESP+0x14]
 *  00508989   037C24 60        ADD EDI,DWORD PTR SS:[ESP+0x60]
 *  0050898D   381C08           CMP BYTE PTR DS:[EAX+ECX],BL
 *  00508990  ^0F85 62FCFFFF    JNZ .005085F8
 *  00508996   8B96 F4000000    MOV EDX,DWORD PTR DS:[ESI+0xF4]
 *  0050899C   8B4424 68        MOV EAX,DWORD PTR SS:[ESP+0x68]
 *  005089A0   0110             ADD DWORD PTR DS:[EAX],EDX
 *  005089A2   8B8E F4000000    MOV ECX,DWORD PTR DS:[ESI+0xF4]
 *  005089A8   8B4424 5C        MOV EAX,DWORD PTR SS:[ESP+0x5C]
 *  005089AC   0108             ADD DWORD PTR DS:[EAX],ECX
 *  005089AE   83BC24 88000000 >CMP DWORD PTR SS:[ESP+0x88],0x10
 *  005089B6   72 0D            JB SHORT .005089C5
 *  005089B8   8B5424 74        MOV EDX,DWORD PTR SS:[ESP+0x74]
 *  005089BC   52               PUSH EDX
 *  005089BD   E8 B05D0A00      CALL .005AE772
 *  005089C2   83C4 04          ADD ESP,0x4
 *  005089C5   8B4424 2C        MOV EAX,DWORD PTR SS:[ESP+0x2C]
 *  005089C9   8B8C24 90050000  MOV ECX,DWORD PTR SS:[ESP+0x590]
 *  005089D0   64:890D 00000000 MOV DWORD PTR FS:[0],ECX
 *  005089D7   59               POP ECX
 *  005089D8   5F               POP EDI
 *  005089D9   5E               POP ESI
 *  005089DA   5D               POP EBP
 *  005089DB   5B               POP EBX
 *  005089DC   8B8C24 78050000  MOV ECX,DWORD PTR SS:[ESP+0x578]
 *  005089E3   33CC             XOR ECX,ESP
 *  005089E5   E8 FC5C0A00      CALL .005AE6E6
 *  005089EA   81C4 88050000    ADD ESP,0x588
 *  005089F0   C2 1000          RETN 0x10
 *  005089F3   CC               INT3
 *  005089F4   CC               INT3
 *  005089F5   CC               INT3
 *  005089F6   CC               INT3
 *  005089F7   CC               INT3
 *  005089F8   CC               INT3
 */
bool attach(ulong startAddress, ulong stopAddress)
{
  const quint8 bytes[] = {
    0x74, 0x0c,                 // 00508770   74 0c            je short .0050877e  ; jichi: pattern found here
    0xb8, 0x6e,0x5c,0x00,0x00   // 00508772   b8 6e5c0000      mov eax,0x5c6e
  };
  ulong addr = MemDbg::findBytes(bytes, sizeof(bytes), startAddress, stopAddress);
  if (!addr)
    return false;
  addr = MemDbg::findEnclosingAlignedFunction(addr);
  if (!addr)
    return false;
  return winhook::hook_before(addr, Private::hookBefore);
}

} // namespace HistoryHook
} // unnamed namespace

bool CatSystem2Engine::attach()
{
  ulong startAddress, stopAddress;
  if (!Engine::getProcessMemoryRange(&startAddress, &stopAddress))
    return false;
  if (!ScenarioHook::attach(startAddress, stopAddress))
    return false;
  if (HistoryHook::attach(startAddress, stopAddress))
    DOUT("history text found");
  else
    DOUT("history text NOT FOUND");
  HijackManager::instance()->attachFunction((ulong)::GetGlyphOutlineA);
  HijackManager::instance()->attachFunction((ulong)::MultiByteToWideChar);
  return true;
}

// EOF

#if 0

namespace HistoryHook {
namespace Private {
  bool hookBefore(winhook::hook_stack *s)
  {
    static QByteArray data_;
    auto text = (LPCSTR)s->stack[1]; // arg1
    if (!text || !*text)
      return true;
    enum { role = Engine::ScenarioRole };
    auto split = s->stack[0]; // retaddr
    auto sig = Engine::hashThreadSignature(role, split);
    data_ = EngineController::instance()->dispatchTextA(text, role, sig);
    s->stack[1] = (ulong)data_.constData();
    //::strcpy(text, data.constData());
    return true;
  }
} // namespace Private

/**
 *  Sample game: カミカゼ
 *
 *  FIXME: This is the log function
 *
 *  Function found by debugging GetGlyphOutlineA, then traversing function stack
 *  005B516E   CC               INT3
 *  005B516F   CC               INT3
 *  005B5170   51               PUSH ECX ; jichi: text in arg1
 *  005B5171   8B43 1C          MOV EAX,DWORD PTR DS:[EBX+0x1C]
 *  005B5174   55               PUSH EBP
 *  005B5175   56               PUSH ESI
 *  005B5176   57               PUSH EDI
 *  005B5177   33FF             XOR EDI,EDI
 *  005B5179   8978 14          MOV DWORD PTR DS:[EAX+0x14],EDI
 *  005B517C   8978 08          MOV DWORD PTR DS:[EAX+0x8],EDI
 *  005B517F   8B73 1C          MOV ESI,DWORD PTR DS:[EBX+0x1C]
 *  005B5182   8D47 02          LEA EAX,DWORD PTR DS:[EDI+0x2]
 *  005B5185   E8 66ABEFFF      CALL .004AFCF0
 *  005B518A   FF46 14          INC DWORD PTR DS:[ESI+0x14]
 *  005B518D   8346 08 02       ADD DWORD PTR DS:[ESI+0x8],0x2
 *  005B5191   8B36             MOV ESI,DWORD PTR DS:[ESI]
 *  005B5193   C606 00          MOV BYTE PTR DS:[ESI],0x0
 *  005B5196   C646 01 00       MOV BYTE PTR DS:[ESI+0x1],0x0
 *  005B519A   8B43 1C          MOV EAX,DWORD PTR DS:[EBX+0x1C]
 *  005B519D   8B48 14          MOV ECX,DWORD PTR DS:[EAX+0x14]
 *  005B51A0   3BCF             CMP ECX,EDI
 *  005B51A2   7E 04            JLE SHORT .005B51A8
 *  005B51A4   49               DEC ECX
 *  005B51A5   8948 14          MOV DWORD PTR DS:[EAX+0x14],ECX
 *  005B51A8   8B4424 14        MOV EAX,DWORD PTR SS:[ESP+0x14]
 *  005B51AC   33ED             XOR EBP,EBP
 *  005B51AE   8038 00          CMP BYTE PTR DS:[EAX],0x0
 *  005B51B1   0F84 D0000000    JE .005B5287
 *  005B51B7   8B4C24 14        MOV ECX,DWORD PTR SS:[ESP+0x14]
 *  005B51BB   8D3429           LEA ESI,DWORD PTR DS:[ECX+EBP]
 *  005B51BE   8BCE             MOV ECX,ESI
 *  005B51C0   8BD3             MOV EDX,EBX
 *  005B51C2   E8 99FBFFFF      CALL .005B4D60
 *  005B51C7   85C0             TEST EAX,EAX
 *  005B51C9   7E 07            JLE SHORT .005B51D2
 *  005B51CB   03E8             ADD EBP,EAX
 *  005B51CD   E9 A7000000      JMP .005B5279
 *  005B51D2   7D 31            JGE SHORT .005B5205
 *  005B51D4   8B73 1C          MOV ESI,DWORD PTR DS:[EBX+0x1C]
 *  005B51D7   F7D8             NEG EAX
 *  005B51D9   894424 0C        MOV DWORD PTR SS:[ESP+0xC],EAX
 *  005B51DD   E8 0EABEFFF      CALL .004AFCF0
 *  005B51E2   8B4424 0C        MOV EAX,DWORD PTR SS:[ESP+0xC]
 *  005B51E6   FF46 14          INC DWORD PTR DS:[ESI+0x14]
 *  005B51E9   0146 08          ADD DWORD PTR DS:[ESI+0x8],EAX
 *  005B51EC   8B36             MOV ESI,DWORD PTR DS:[ESI]
 *  005B51EE   85C0             TEST EAX,EAX
 *  005B51F0   7E 75            JLE SHORT .005B5267
 *  005B51F2   8B5424 14        MOV EDX,DWORD PTR SS:[ESP+0x14]
 *  005B51F6   8A0C2A           MOV CL,BYTE PTR DS:[EDX+EBP]
 *  005B51F9   880C37           MOV BYTE PTR DS:[EDI+ESI],CL
 *  005B51FC   47               INC EDI
 *  005B51FD   45               INC EBP
 *  005B51FE   83E8 01          SUB EAX,0x1
 *  005B5201  ^75 EF            JNZ SHORT .005B51F2
 *  005B5203   EB 62            JMP SHORT .005B5267
 *  005B5205   8BCE             MOV ECX,ESI
 *  005B5207   E8 34C4EFFF      CALL .004B1640
 *  005B520C   8B73 1C          MOV ESI,DWORD PTR DS:[EBX+0x1C]
 *  005B520F   85C0             TEST EAX,EAX
 *  005B5211   74 2D            JE SHORT .005B5240
 *  005B5213   B8 02000000      MOV EAX,0x2
 *  005B5218   E8 D3AAEFFF      CALL .004AFCF0
 *  005B521D   FF46 14          INC DWORD PTR DS:[ESI+0x14]
 *  005B5220   8346 08 02       ADD DWORD PTR DS:[ESI+0x8],0x2
 *  005B5224   8B5424 14        MOV EDX,DWORD PTR SS:[ESP+0x14]
 *  005B5228   8B36             MOV ESI,DWORD PTR DS:[ESI]
 *  005B522A   8D042A           LEA EAX,DWORD PTR DS:[EDX+EBP]
 *  005B522D   8A00             MOV AL,BYTE PTR DS:[EAX]
 *  005B522F   880437           MOV BYTE PTR DS:[EDI+ESI],AL
 *  005B5232   45               INC EBP
 *  005B5233   47               INC EDI
 *  005B5234   8BCA             MOV ECX,EDX
 *  005B5236   8A1429           MOV DL,BYTE PTR DS:[ECX+EBP]
 *  005B5239   881437           MOV BYTE PTR DS:[EDI+ESI],DL
 *  005B523C   47               INC EDI
 *  005B523D   45               INC EBP
 *  005B523E   EB 27            JMP SHORT .005B5267
 *  005B5240   B8 01000000      MOV EAX,0x1
 *  005B5245   E8 A6AAEFFF      CALL .004AFCF0
 *  005B524A   8B4424 14        MOV EAX,DWORD PTR SS:[ESP+0x14]
 *  005B524E   B9 01000000      MOV ECX,0x1
 *  005B5253   014E 14          ADD DWORD PTR DS:[ESI+0x14],ECX
 *  005B5256   014E 08          ADD DWORD PTR DS:[ESI+0x8],ECX
 *  005B5259   8B36             MOV ESI,DWORD PTR DS:[ESI]
 *  005B525B   8A1428           MOV DL,BYTE PTR DS:[EAX+EBP]
 *  005B525E   03C5             ADD EAX,EBP
 *  005B5260   881437           MOV BYTE PTR DS:[EDI+ESI],DL
 *  005B5263   03F9             ADD EDI,ECX
 *  005B5265   03E9             ADD EBP,ECX
 *  005B5267   C60437 00        MOV BYTE PTR DS:[EDI+ESI],0x0
 *  005B526B   8B43 1C          MOV EAX,DWORD PTR DS:[EBX+0x1C]
 *  005B526E   8B48 14          MOV ECX,DWORD PTR DS:[EAX+0x14]
 *  005B5271   85C9             TEST ECX,ECX
 *  005B5273   7E 04            JLE SHORT .005B5279
 *  005B5275   49               DEC ECX
 *  005B5276   8948 14          MOV DWORD PTR DS:[EAX+0x14],ECX
 *  005B5279   8B4424 14        MOV EAX,DWORD PTR SS:[ESP+0x14]
 *  005B527D   803C28 00        CMP BYTE PTR DS:[EAX+EBP],0x0
 *  005B5281  ^0F85 30FFFFFF    JNZ .005B51B7
 *  005B5287   5F               POP EDI
 *  005B5288   5E               POP ESI
 *  005B5289   B8 01000000      MOV EAX,0x1
 *  005B528E   5D               POP EBP
 *  005B528F   59               POP ECX
 *  005B5290   C2 0400          RETN 0x4
 *  005B5293   CC               INT3
 *  005B5294   CC               INT3
 *  005B5295   CC               INT3
 *  005B5296   CC               INT3
 */
bool attach()
{
  ulong startAddress, stopAddress;
  if (!Engine::getProcessMemoryRange(&startAddress, &stopAddress))
    return false;

  const quint8 bytes[] = {
    0xff,0x46, 0x14,        // 005b521d   ff46 14          inc dword ptr ds:[esi+0x14]
    0x83,0x46, 0x08, 0x02,  // 005b5220   8346 08 02       add dword ptr ds:[esi+0x8],0x2
    0x8b,0x54,0x24, 0x14,   // 005b5224   8b5424 14        mov edx,dword ptr ss:[esp+0x14]
    0x8b,0x36,              // 005b5228   8b36             mov esi,dword ptr ds:[esi]
    0x8d,0x04,0x2a,         // 005b522a   8d042a           lea eax,dword ptr ds:[edx+ebp]
    0x8a,0x00,              // 005b522d   8a00             mov al,byte ptr ds:[eax]
    0x88,0x04,0x37          // 005b522f   880437           mov byte ptr ds:[edi+esi],al
  };

  ulong addr = MemDbg::findBytes(bytes, sizeof(bytes), startAddress, stopAddress);
  if (!addr)
    return false;
  addr = MemDbg::findEnclosingAlignedFunction(addr);
  if (!addr)
    return false;
  return winhook::hook_before(addr, Private::hookBefore);
}

} // namespace HistoryHook
#endif // 0
