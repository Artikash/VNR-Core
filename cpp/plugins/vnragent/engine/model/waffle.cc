// waffle.cc
// 8/18/2015 jichi
#include "engine/model/waffle.h"
#include "engine/enginecontroller.h"
#include "engine/enginedef.h"
#include "engine/enginehash.h"
#include "engine/engineutil.h"
#include "hijack/hijackmanager.h"
#include "memdbg/memsearch.h"
#include "winhook/hookcode.h"
#include <qt_windows.h>
#include <cstdint>

#define DEBUG "model/waffle"
#include "sakurakit/skdebug.h"

/** Private data */

namespace { // unnamed
//ulong moduleBaseAddress_;
namespace ScenarioHook {
namespace Private {
  /**
   *  Arg1 for long text also on the stack:
   *  03E5EC14  30 D1 5C 01 B8 99 C6 08 A0 88 BB 08 50 EC E5 03  0ﾑ\ｸ卮綾P・
   *  					  jichi: source text here
   *  03E5EC24  68 EC E5 03 42 00 00 00 4F 00 00 00 84 F9 A3 00  h・B...O...・｣.
   *  					  jichi: source size here
   *  03E5EC34  A0 F7 7C 00 2C D1 5C 01 38 64 AA 00 10 0B F4 C9  .,ﾑ\8dｪ.
   *  03E5EC44  13 00 00 00 1F 00 00 00 64 00 00 00 00 00 00 00  ......d.......
   *
   *  Arg1 for short text:
   *  023E10E8  61 C1 9A 35 8E 9E 8A D4 82 F0 8E 7E 82 DF 82 BD  aﾁ・時間を止めた
   *  023E10F8  81 42 00 16 0E 00 00 00 0F 00 00 00 9C 98 10 3F  。.......恫?
   *  023E1108  00 EE ED 98 A8 59 11 33 C2 C3 42 83 DF 9C FC C6  .・乖Y3ﾂﾃB・戛ﾆ
   *  023E1118  00 00 00 00 0F 00 00 00 79 7B BA 93 00 DA 8B 46  .......y{ｺ・ﾚ祈
   */
  struct HookArgument
  {
    enum { ShortTextCapacity = 0x10 }; // including \0

    union {
      LPCSTR text;
      char chars[ShortTextCapacity];
    };
    int size;
    int capacity; // excluding \0

    bool isValid() const
    {
      if (size <= 0 || size > capacity)
        return false;
      LPCSTR t = getText();
      return Engine::isAddressWritable(t) && ::strlen(t) == size;
          //&& t[0] > 127 && t[size - 1] > 127 // skip ascii text
    }

    LPCSTR getText() const
    { return size < ShortTextCapacity ? chars : text; }

    void setText(LPCSTR _text, int _size)
    {
      if (_size < ShortTextCapacity)
        ::memcpy(chars, _text, _size + 1);
      else
        text = _text;
      capacity = size = _size;
    }
  };
  HookArgument *arg_,
               argValue_;
  bool hookBefore(int role, winhook::hook_stack *s)
  {
    static QByteArray data_; // persistent storage, which makes this function not thread-safe

    //auto reladdr = retaddr - moduleBaseAddress_;
    // Sample game: 完全時間停止 ～無理やり時間を止められた世界でハメられる女たち～
    // Scenario: 0xbfd4d
    // Name:  0xbfd36
    //if (reladdr == 0xc6e75 ||
    //    reladdr == 0xc6e1f ||
    //    reladdr == 0x61a57 ||
    //    reladdr == 0xe762d ||
    //    reladdr == 0xe768a ||
    //    reladdr == 0xe76a6 ||
    //    reladdr == 0xe78d5 ||
    //    reladdr == 0x446e7 ||
    //    reladdr == 0x177317 ||
    //    reladdr == 0x52ca ||
    //    reladdr == 0x529c ||
    //    reladdr == 0x55df)
    //  return true;

    // Sample game: 漫喫ハプニング
    // Scenario: 0x1174bc
    // Name: 0x1174a6
    //if (reladdr == 0x450f ||
    //    reladdr == 0x1b45c ||
    //    reladdr == 0x1b48a ||
    //    reladdr == 0x10fe77 ||
    //    reladdr == 0x11d0c9 ||
    //    reladdr == 0x1100e0 ||
    //    reladdr == 0x10fe93 ||
    //    reladdr == 0x10fde1 ||
    //    reladdr == 0x11d073)
    //  return true;

    //DOUT(retaddr);

    auto arg = (HookArgument *)(s->stack[0] + sizeof(DWORD)); // arg1
    if (!arg || !arg->isValid())
      return true;

    //enum { role = Engine::ScenarioRole };
    //auto role = Engine::OtherRole;;
    //if (reladdr == 0xbfd4d) // scenario thread, only hook to this call instead
    //  role = Engine::ScenarioRole;
    //else if (reladdr == 0xbfd36)
    //  role = Engine::NameRole;
    //else if (reladdr == 0x60285)
    //  role = Engine::FontRole;
    //else
    //  return true;
    //DOUT(retaddr);

    //auto sig = Engine::hashThreadSignature(role, reladdr);
    enum { sig = 0 };
    QByteArray oldData = arg->getText(),
               newData = EngineController::instance()->dispatchTextA(oldData, role, sig);
    if (newData == oldData)
      return true;
    arg_ = arg;
    argValue_ = *arg;

    data_ = newData;
    arg->setText(data_.constData(), data_.size());
    return true;
  }
  bool hookAfter(winhook::hook_stack *)
  {
    if (arg_) {
      *arg_ = argValue_;
      arg_ = nullptr;
    }
    return 0;
  }
} // namespace Private

/**
 *  Sample game: 完全時間停止 ～無理やり時間を止められた世界でハメられる女たち～
 *
 *  Base addr: 09e0000
 *
 *  Debugging method:
 *  - First find the function like memcpy_s by debugging where scenario text is modified.
 *    arg1: target text
 *    arg2: target capacity
 *    arg3: source text
 *    arg4: source size
 *
 *  009E59FA   CC               INT3
 *  009E59FB   CC               INT3
 *  009E59FC   CC               INT3
 *  009E59FD   CC               INT3
 *  009E59FE   CC               INT3
 *  009E59FF   CC               INT3
 *  009E5A00   53               PUSH EBX
 *  009E5A01   8B5C24 08        MOV EBX,DWORD PTR SS:[ESP+0x8]
 *  009E5A05   55               PUSH EBP
 *  009E5A06   8B6C24 10        MOV EBP,DWORD PTR SS:[ESP+0x10]
 *  009E5A0A   56               PUSH ESI
 *  009E5A0B   57               PUSH EDI
 *  009E5A0C   8BF1             MOV ESI,ECX
 *  009E5A0E   396B 14          CMP DWORD PTR DS:[EBX+0x14],EBP
 *  009E5A11   73 05            JNB SHORT play.009E5A18
 *  009E5A13   E8 66B71B00      CALL play.00BA117E
 *  009E5A18   8B7B 14          MOV EDI,DWORD PTR DS:[EBX+0x14]
 *  009E5A1B   8B4424 1C        MOV EAX,DWORD PTR SS:[ESP+0x1C]
 *  009E5A1F   2BFD             SUB EDI,EBP
 *  009E5A21   3BC7             CMP EAX,EDI
 *  009E5A23   73 02            JNB SHORT play.009E5A27
 *  009E5A25   8BF8             MOV EDI,EAX
 *  009E5A27   3BF3             CMP ESI,EBX
 *  009E5A29   75 1F            JNZ SHORT play.009E5A4A
 *  009E5A2B   6A FF            PUSH -0x1
 *  009E5A2D   03FD             ADD EDI,EBP
 *  009E5A2F   57               PUSH EDI
 *  009E5A30   8BCE             MOV ECX,ESI
 *  009E5A32   E8 39FFFFFF      CALL play.009E5970
 *  009E5A37   55               PUSH EBP
 *  009E5A38   6A 00            PUSH 0x0
 *  009E5A3A   8BCE             MOV ECX,ESI
 *  009E5A3C   E8 2FFFFFFF      CALL play.009E5970
 *  009E5A41   5F               POP EDI
 *  009E5A42   8BC6             MOV EAX,ESI
 *  009E5A44   5E               POP ESI
 *  009E5A45   5D               POP EBP
 *  009E5A46   5B               POP EBX
 *  009E5A47   C2 0C00          RETN 0xC
 *  009E5A4A   83FF FE          CMP EDI,-0x2
 *  009E5A4D   76 05            JBE SHORT play.009E5A54
 *  009E5A4F   E8 F2B61B00      CALL play.00BA1146
 *  009E5A54   8B46 18          MOV EAX,DWORD PTR DS:[ESI+0x18]
 *  009E5A57   3BC7             CMP EAX,EDI
 *  009E5A59   73 1B            JNB SHORT play.009E5A76
 *  009E5A5B   8B46 14          MOV EAX,DWORD PTR DS:[ESI+0x14]
 *  009E5A5E   50               PUSH EAX
 *  009E5A5F   57               PUSH EDI
 *  009E5A60   8BCE             MOV ECX,ESI
 *  009E5A62   E8 69010000      CALL play.009E5BD0
 *  009E5A67   85FF             TEST EDI,EDI
 *  009E5A69   76 66            JBE SHORT play.009E5AD1
 *  009E5A6B   837B 18 10       CMP DWORD PTR DS:[EBX+0x18],0x10
 *  009E5A6F   72 2F            JB SHORT play.009E5AA0
 *  009E5A71   8B53 04          MOV EDX,DWORD PTR DS:[EBX+0x4]
 *  009E5A74   EB 2D            JMP SHORT play.009E5AA3
 *  009E5A76   85FF             TEST EDI,EDI
 *  009E5A78  ^75 EF            JNZ SHORT play.009E5A69
 *  009E5A7A   897E 14          MOV DWORD PTR DS:[ESI+0x14],EDI
 *  009E5A7D   83F8 10          CMP EAX,0x10
 *  009E5A80   72 0F            JB SHORT play.009E5A91
 *  009E5A82   8B46 04          MOV EAX,DWORD PTR DS:[ESI+0x4]
 *  009E5A85   5F               POP EDI
 *  009E5A86   C600 00          MOV BYTE PTR DS:[EAX],0x0
 *  009E5A89   8BC6             MOV EAX,ESI
 *  009E5A8B   5E               POP ESI
 *  009E5A8C   5D               POP EBP
 *  009E5A8D   5B               POP EBX
 *  009E5A8E   C2 0C00          RETN 0xC
 *  009E5A91   8D46 04          LEA EAX,DWORD PTR DS:[ESI+0x4]
 *  009E5A94   5F               POP EDI
 *  009E5A95   C600 00          MOV BYTE PTR DS:[EAX],0x0
 *  009E5A98   8BC6             MOV EAX,ESI
 *  009E5A9A   5E               POP ESI
 *  009E5A9B   5D               POP EBP
 *  009E5A9C   5B               POP EBX
 *  009E5A9D   C2 0C00          RETN 0xC
 *  009E5AA0   8D53 04          LEA EDX,DWORD PTR DS:[EBX+0x4]
 *  009E5AA3   8B4E 18          MOV ECX,DWORD PTR DS:[ESI+0x18]
 *  009E5AA6   8D5E 04          LEA EBX,DWORD PTR DS:[ESI+0x4]
 *  009E5AA9   83F9 10          CMP ECX,0x10
 *  009E5AAC   72 04            JB SHORT play.009E5AB2
 *  009E5AAE   8B03             MOV EAX,DWORD PTR DS:[EBX]
 *  009E5AB0   EB 02            JMP SHORT play.009E5AB4
 *  009E5AB2   8BC3             MOV EAX,EBX
 *  009E5AB4   57               PUSH EDI	; jichi: source size
 *  009E5AB5   03D5             ADD EDX,EBP
 *  009E5AB7   52               PUSH EDX	; jichi: source text
 *  009E5AB8   51               PUSH ECX	; jichi: target size
 *  009E5AB9   50               PUSH EAX	; jichi: target text
 *  009E5ABA   E8 F9A91F00      CALL play.00BE04B8	; jichi: called
 *  009E5ABF   83C4 10          ADD ESP,0x10
 *  009E5AC2   837E 18 10       CMP DWORD PTR DS:[ESI+0x18],0x10
 *  009E5AC6   897E 14          MOV DWORD PTR DS:[ESI+0x14],EDI
 *  009E5AC9   72 02            JB SHORT play.009E5ACD
 *  009E5ACB   8B1B             MOV EBX,DWORD PTR DS:[EBX]
 *  009E5ACD   C6043B 00        MOV BYTE PTR DS:[EBX+EDI],0x0
 *  009E5AD1   5F               POP EDI
 *  009E5AD2   8BC6             MOV EAX,ESI
 *  009E5AD4   5E               POP ESI
 *  009E5AD5   5D               POP EBP
 *  009E5AD6   5B               POP EBX
 *  009E5AD7   C2 0C00          RETN 0xC
 *  009E5ADA   CC               INT3
 *  009E5ADB   CC               INT3
 *  009E5ADC   CC               INT3
 *  009E5ADD   CC               INT3
 *
 *  Callers of that function:
 *
 *  0112FCFE   E8 A0670200      CALL 完全時間.011564A3
 *  0112FD03   8B7424 18        MOV ESI,DWORD PTR SS:[ESP+0x18]
 *  0112FD07   8D8424 9C000000  LEA EAX,DWORD PTR SS:[ESP+0x9C]
 *  0112FD0E   50               PUSH EAX
 *  0112FD0F   E8 AC9EF4FF      CALL 完全時間.01079BC0
 *  0112FD14   6A FF            PUSH -0x1
 *  0112FD16   6A 00            PUSH 0x0
 *  0112FD18   8DBE 84000000    LEA EDI,DWORD PTR DS:[ESI+0x84]
 *  0112FD1E   57               PUSH EDI
 *  0112FD1F   8D8C24 B0000000  LEA ECX,DWORD PTR SS:[ESP+0xB0]
 *  0112FD26   C78424 24010000 0B000000 MOV DWORD PTR SS:[ESP+0x124],0xB
 *  0112FD31  -E9 CA02A90C      JMP 0DBC0000    ; jichi: name caller
 *  0112FD36   6A FF            PUSH -0x1
 *  0112FD38   6A 00            PUSH 0x0
 *  0112FD3A   8D86 A0000000    LEA EAX,DWORD PTR DS:[ESI+0xA0]
 *  0112FD40   50               PUSH EAX
 *  0112FD41   8D8C24 CC000000  LEA ECX,DWORD PTR SS:[ESP+0xCC]
 *  0112FD48  -E9 B302AA0C      JMP 0DBD0000    ; jichi: scenario caller
 *  0112FD4D   6A FF            PUSH -0x1
 *  0112FD4F   6A 00            PUSH 0x0
 *  0112FD51   53               PUSH EBX
 *  0112FD52   8D8C24 E8000000  LEA ECX,DWORD PTR SS:[ESP+0xE8]
 *  0112FD59  -E9 A202AB0C      JMP 0DBE0000
 *  0112FD5E   8B46 04          MOV EAX,DWORD PTR DS:[ESI+0x4]
 *  0112FD61   898424 F8000000  MOV DWORD PTR SS:[ESP+0xF8],EAX
 *  0112FD68   8B46 08          MOV EAX,DWORD PTR DS:[ESI+0x8]
 *  0112FD6B   8B7424 1C        MOV ESI,DWORD PTR SS:[ESP+0x1C]
 *  0112FD6F   898424 FC000000  MOV DWORD PTR SS:[ESP+0xFC],EAX
 *  0112FD76   8B46 08          MOV EAX,DWORD PTR DS:[ESI+0x8]
 *  0112FD79   FFB0 00010000    PUSH DWORD PTR DS:[EAX+0x100]
 *  0112FD7F   8BCB             MOV ECX,EBX
 *  0112FD81   E8 8DFAF8FF      CALL 完全時間.010BF813
 *  0112FD86   898424 A0000000  MOV DWORD PTR SS:[ESP+0xA0],EAX
 *  0112FD8D   83F8 FF          CMP EAX,-0x1
 *  0112FD90   75 2B            JNZ SHORT 完全時間.0112FDBD
 *  0112FD92   837B 18 10       CMP DWORD PTR DS:[EBX+0x18],0x10
 *  0112FD96   72 05            JB SHORT 完全時間.0112FD9D
 *  0112FD98   8B5B 04          MOV EBX,DWORD PTR DS:[EBX+0x4]
 *  0112FD9B   EB 03            JMP SHORT 完全時間.0112FDA0
 *  0112FD9D   83C3 04          ADD EBX,0x4
 *  0112FDA0   837F 18 10       CMP DWORD PTR DS:[EDI+0x18],0x10
 *  0112FDA4   72 05            JB SHORT 完全時間.0112FDAB
 *  0112FDA6   8B7F 04          MOV EDI,DWORD PTR DS:[EDI+0x4]
 *  0112FDA9   EB 03            JMP SHORT 完全時間.0112FDAE
 *  0112FDAB   83C7 04          ADD EDI,0x4
 *  0112FDAE   53               PUSH EBX
 *  0112FDAF   57               PUSH EDI
 *  0112FDB0   68 E4BF2D01      PUSH 完全時間.012DBFE4
 *  0112FDB5   E8 A65AF4FF      CALL 完全時間.01075860
 *  0112FDBA   83C4 0C          ADD ESP,0xC
 *  0112FDBD   8B46 08          MOV EAX,DWORD PTR DS:[ESI+0x8]
 *  0112FDC0   8B98 E8000000    MOV EBX,DWORD PTR DS:[EAX+0xE8]
 *  0112FDC6   8B4B 14          MOV ECX,DWORD PTR DS:[EBX+0x14]
 *  0112FDC9   894424 18        MOV DWORD PTR SS:[ESP+0x18],EAX
 *  0112FDCD   8D8424 9C000000  LEA EAX,DWORD PTR SS:[ESP+0x9C]
 *  0112FDD4   E8 F792FCFF      CALL 完全時間.010F90D0
 *  0112FDD9   8D8424 9C000000  LEA EAX,DWORD PTR SS:[ESP+0x9C]
 *  0112FDE0   50               PUSH EAX
 *  0112FDE1   8B43 18          MOV EAX,DWORD PTR DS:[EBX+0x18]
 *  0112FDE4   E8 399AFCFF      CALL 完全時間.010F9822
 *  0112FDE9   8D73 38          LEA ESI,DWORD PTR DS:[EBX+0x38]
 *  0112FDEC   8DBC24 9C000000  LEA EDI,DWORD PTR SS:[ESP+0x9C]
 *  0112FDF3   E8 C8BFF4FF      CALL 完全時間.0107BDC0
 *  0112FDF8   8BC7             MOV EAX,EDI
 *  0112FDFA   50               PUSH EAX
 *  0112FDFB   8D43 30          LEA EAX,DWORD PTR DS:[EBX+0x30]
 *  0112FDFE   E8 2D4AFAFF      CALL 完全時間.010D4830
 *
 *  Sample game: 漫喫ハプニング
 *
 *  Scenario callers:
 *
 *  0039746D   E8 3ED2EEFF      CALL .002846B0
 *  00397472   8B7424 18        MOV ESI,DWORD PTR SS:[ESP+0x18]
 *  00397476   33FF             XOR EDI,EDI
 *  00397478   8D8424 B4000000  LEA EAX,DWORD PTR SS:[ESP+0xB4]
 *  0039747F   50               PUSH EAX
 *  00397480   E8 9BC5F0FF      CALL .002A3A20
 *  00397485   6A FF            PUSH -0x1
 *  00397487   57               PUSH EDI
 *  00397488   8D83 84000000    LEA EAX,DWORD PTR DS:[EBX+0x84]
 *  0039748E   50               PUSH EAX
 *  0039748F   8D8C24 C8000000  LEA ECX,DWORD PTR SS:[ESP+0xC8]
 *  00397496   C78424 3C010000 12000000 MOV DWORD PTR SS:[ESP+0x13C],0x12
 *  003974A1  -E9 5A8BB410      JMP 10EE0000    ; jichi: name
 *  003974A6   6A FF            PUSH -0x1
 *  003974A8   57               PUSH EDI
 *  003974A9   8D83 A0000000    LEA EAX,DWORD PTR DS:[EBX+0xA0]
 *  003974AF   50               PUSH EAX
 *  003974B0   8D8C24 E4000000  LEA ECX,DWORD PTR SS:[ESP+0xE4]
 *  003974B7  -E9 448BB510      JMP 10EF0000    ; jichi: scenario
 *  003974BC   6A FF            PUSH -0x1
 *  003974BE   57               PUSH EDI
 *  003974BF   8DBB BC000000    LEA EDI,DWORD PTR DS:[EBX+0xBC]
 *  003974C5   57               PUSH EDI
 *  003974C6   8D8C24 00010000  LEA ECX,DWORD PTR SS:[ESP+0x100]
 *  003974CD  -E9 2E8BB610      JMP 10F00000
 *  003974D2   8B43 04          MOV EAX,DWORD PTR DS:[EBX+0x4]
 *  003974D5   898424 10010000  MOV DWORD PTR SS:[ESP+0x110],EAX
 *  003974DC   8B43 08          MOV EAX,DWORD PTR DS:[EBX+0x8]
 *  003974DF   898424 14010000  MOV DWORD PTR SS:[ESP+0x114],EAX
 *  003974E6   8B46 08          MOV EAX,DWORD PTR DS:[ESI+0x8]
 *  003974E9   FFB0 00010000    PUSH DWORD PTR DS:[EAX+0x100]
 *  003974EF   8BCF             MOV ECX,EDI
 *  003974F1   E8 D333F5FF      CALL .002EA8C9
 *  003974F6   8B76 08          MOV ESI,DWORD PTR DS:[ESI+0x8]
 *  003974F9   898424 B8000000  MOV DWORD PTR SS:[ESP+0xB8],EAX
 *  00397500   8B9E E8000000    MOV EBX,DWORD PTR DS:[ESI+0xE8]
 *  00397506   8B4B 14          MOV ECX,DWORD PTR DS:[EBX+0x14]
 *  00397509   8D8424 B4000000  LEA EAX,DWORD PTR SS:[ESP+0xB4]
 *  00397510   897424 1C        MOV DWORD PTR SS:[ESP+0x1C],ESI
 *  00397514   E8 C897FCFF      CALL .00360CE1
 *  00397519   8D8424 B4000000  LEA EAX,DWORD PTR SS:[ESP+0xB4]
 */
bool attach(ulong startAddress, ulong stopAddress)
{
  const uint8_t bytes[] = {
    0x8b,0xf1,       // 009e5a0c   8bf1             mov esi,ecx
    0x39,0x6b, 0x14, // 009e5a0e   396b 14          cmp dword ptr ds:[ebx+0x14],ebp
    0x73, 0x05       // 009e5a11   73 05            jnb short play.009e5a18
  };
  ulong addr = MemDbg::findBytes(bytes, sizeof(bytes), startAddress, stopAddress);
  if (!addr)
    return false;
  addr = MemDbg::findEnclosingAlignedFunction(addr);
  if (!addr)
    return false;
  int count = 0;
  auto fun = [&count, startAddress](ulong addr) -> bool {
    // 00397496   C78424 3C010000 12000000 MOV DWORD PTR SS:[ESP+0x13C],0x12
    // 003974A1  -E9 5A8BB410      JMP 10EE0000    ; jichi: name
    // 003974A6   6A FF            PUSH -0x1
    // 003974A8   57               PUSH EDI
    // 003974A9   8D83 A0000000    LEA EAX,DWORD PTR DS:[EBX+0xA0]
    // 003974AF   50               PUSH EAX
    // 003974B0   8D8C24 E4000000  LEA ECX,DWORD PTR SS:[ESP+0xE4]
    // 003974B7  -E9 448BB510      JMP 10EF0000    ; jichi: scenario
    // 003974BC   6A FF            PUSH -0x1
    // 003974BE   57               PUSH EDI
    int role = Engine::OtherRole;
    if (*(DWORD *)(addr - 8) == 0x248c8d50)
      role = Engine::ScenarioRole;
    else if ((*(DWORD *)(addr - 11) & 0x00ffffff) == 0x002484c7)
      role = Engine::NameRole;
    else
      return true;
    auto before = std::bind(Private::hookBefore, role, std::placeholders::_1);
    count += winhook::hook_both(addr, before, Private::hookAfter);
    return true;
  };
  MemDbg::iterNearCallAddress(fun, addr, startAddress, stopAddress);
  DOUT("call number =" << count);
  return count;
}
} // namespace ScenarioHook
} // unnamed namespace

/** Public class */

bool WaffleEngine::attach()
{
  ulong startAddress, stopAddress;
  if (!Engine::getProcessMemoryRange(&startAddress, &stopAddress))
    return false;
  //moduleBaseAddress_ = startAddress;
  if (!ScenarioHook::attach(startAddress, stopAddress))
    return false;
  HijackManager::instance()->attachFunction((ulong)::TextOutA);
  HijackManager::instance()->attachFunction((ulong)::GetTextExtentPoint32A);
  return true;
}

// EOF
