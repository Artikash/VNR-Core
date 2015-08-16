// gxp.cc
// 8/15/2015 jichi
#include "engine/model/gxp.h"
#include "engine/enginecontroller.h"
#include "engine/enginedef.h"
#include "engine/enginehash.h"
#include "engine/engineutil.h"
//#include "hijack/hijackmanager.h"
#include "util/textutil.h"
#include "memdbg/memsearch.h"
#include "winhook/hookcode.h"
#include "winhook/hookfun.h"
#include <qt_windows.h>
#include <cstdint>

#define DEBUG "model/gxp"
#include "sakurakit/skdebug.h"

/** Private data */

namespace { // unnamed
ulong moduleBaseAddress_;

namespace ScenarioHook2 { // for new GXP2
namespace Private {
  /**
   *  Sample system text:
   *  cg/bg/ショップ前＿昼.png
   *
   *  Scenario caller:
   *  00187E65  |. 8B06           MOV EAX,DWORD PTR DS:[ESI]
   *  00187E67  |. FF90 A8020000  CALL DWORD PTR DS:[EAX+0x2A8]
   *  00187E6D  |. 8B45 08        MOV EAX,DWORD PTR SS:[EBP+0x8]
   *  00187E70  |. 8D8E A8010000  LEA ECX,DWORD PTR DS:[ESI+0x1A8]
   *  00187E76  |. 3BC8           CMP ECX,EAX
   *  00187E78  |. 74 0A          JE SHORT play.00187E84
   *  00187E7A  |. 6A FF          PUSH -0x1
   *  00187E7C  |. 6A 00          PUSH 0x0
   *  00187E7E  |. 50             PUSH EAX
   *  00187E7F  |. E8 CCDAFBFF    CALL play.00145950
   *  00187E84  |> 8A45 0C        MOV AL,BYTE PTR SS:[EBP+0xC]
   *  00187E87  |. 8B0D 882C4F00  MOV ECX,DWORD PTR DS:[0x4F2C88]
   *  00187E8D  |. 8886 03030000  MOV BYTE PTR DS:[ESI+0x303],AL
   *  00187E93  |. 8A45 10        MOV AL,BYTE PTR SS:[EBP+0x10]
   *  00187E96  |. 8886 05030000  MOV BYTE PTR DS:[ESI+0x305],AL
   *
   *  The history thread that is needed to be skipped to avoid retranslation
   *  0095391A   74 0A            JE SHORT 塔の下の.00953926
   *  0095391C   6A FF            PUSH -0x1
   *  0095391E   6A 00            PUSH 0x0
   *  00953920   50               PUSH EAX
   *  00953921   call
   *  00953926   A1 882CCB00      MOV EAX,DWORD PTR DS:[0xCB2C88]   ; jichi: retaddr
   *  0095392B   A8 01            TEST AL,0x1
   *  0095392D   75 28            JNZ SHORT 塔の下の.00953957
   */
  struct HookArgument
  {
    //enum { ShortTextCapacity = 8 };
    enum { ShortTextCapacity = 0 }; // short text capacity disabled or it could crash the game

    LPCWSTR text; // 0x0
    DWORD unknown[3];
    int size, // 0x10
        capacity;

    bool isValid() const
    {
      if (size <= 0 || size > capacity)
        return false;
      LPCWSTR t = getText();
      return Engine::isAddressWritable(t) && ::wcslen(t) == size
          && t[0] > 127 && t[size - 1] > 127; // skip ascii text
    }

    LPCWSTR getText() const
    { return size < ShortTextCapacity ? (LPCWSTR)this : text; }

    void setText(LPCWSTR _text, int _size)
    {
      if (_size < ShortTextCapacity) {
        size = _size;
        ::memcpy((void *)this, _text, (_size + 1) * sizeof(wchar_t));
      } else {
        text = _text;
        size = _size;
      }
      capacity = size + 1;
    }
  };
  HookArgument *arg_,
               argValue_;
  bool hookBefore(ulong retaddr, winhook::hook_stack *s)
  {
    static QString text_; // persistent storage, which makes this function not thread-safe
    auto arg = (HookArgument *)s->stack[0]; // arg1
    if (!arg->isValid())
      return true;

    auto reladdr = retaddr - moduleBaseAddress_;
    // 00187E7F  |. E8 CCDAFBFF    CALL play.00145950
    // 00187E84  |> 8A45 0C        MOV AL,BYTE PTR SS:[EBP+0xC]
    auto role = Engine::OtherRole;
    if (*(WORD *)retaddr == 0x458a)
      role = Engine::ScenarioRole;
    // 00953926   A1 882CCB00      MOV EAX,DWORD PTR DS:[0xCB2C88]   ; jichi: retaddr
    //if (reladdr == 0x53926) // skip retranslate functions
    //  return true;
    else if (*(BYTE *)retaddr == 0xa1)
      return true;
    QString oldText = QString::fromWCharArray(arg->getText()),
            newText = EngineController::instance()->dispatchTextW(oldText, role, reladdr);
    if (newText == oldText)
      return true;
    text_ = newText;

    arg_ = arg;
    argValue_ = *arg;

    arg->setText(text_.utf16(), text_.size());

    //if (arg->size)
    //  hashes_.insert(Engine::hashWCharArray(arg->text, arg->size));
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
 *  Sample game: 塔の下のエクセルキトゥス体験版
 *  Executable description shows "AVGEngineV2"
 *
 *  Function found by debugging the memcpy function.
 *
 *  Scenario caller, text in the struct of arg1.
 *
 *  0081594C  |. CC             INT3
 *  0081594D  |. CC             INT3
 *  0081594E  |. CC             INT3
 *  0081594F  |. CC             INT3
 *  00815950  |$ 55             PUSH EBP
 *  00815951  |. 8BEC           MOV EBP,ESP
 *  00815953  |. 53             PUSH EBX
 *  00815954  |. 8B5D 08        MOV EBX,DWORD PTR SS:[EBP+0x8]
 *  00815957  |. 56             PUSH ESI
 *  00815958  |. 57             PUSH EDI
 *  00815959  |. 8BF1           MOV ESI,ECX
 *  0081595B  |. 8B4D 0C        MOV ECX,DWORD PTR SS:[EBP+0xC]
 *  0081595E  |. 8B7B 10        MOV EDI,DWORD PTR DS:[EBX+0x10]
 *  00815961  |. 3BF9           CMP EDI,ECX
 *  00815963  |. 0F82 D0000000  JB play.00815A39
 *  00815969  |. 2BF9           SUB EDI,ECX
 *  0081596B  |. 397D 10        CMP DWORD PTR SS:[EBP+0x10],EDI
 *  0081596E  |. 0F427D 10      CMOVB EDI,DWORD PTR SS:[EBP+0x10]
 *  00815972  |. 3BF3           CMP ESI,EBX
 *  00815974  |. 75 20          JNZ SHORT play.00815996	; jichi: jump
 *  00815976  |. 8D040F         LEA EAX,DWORD PTR DS:[EDI+ECX]
 *  00815979  |. 8BCE           MOV ECX,ESI
 *  0081597B  |. 50             PUSH EAX
 *  0081597C  |. E8 DFFEFFFF    CALL play.00815860
 *  00815981  |. FF75 0C        PUSH DWORD PTR SS:[EBP+0xC]
 *  00815984  |. 8BCE           MOV ECX,ESI
 *  00815986  |. 6A 00          PUSH 0x0
 *  00815988  |. E8 13FFFFFF    CALL play.008158A0
 *  0081598D  |. 5F             POP EDI
 *  0081598E  |. 8BC6           MOV EAX,ESI
 *  00815990  |. 5E             POP ESI
 *  00815991  |. 5B             POP EBX
 *  00815992  |. 5D             POP EBP
 *  00815993  |. C2 0C00        RETN 0xC
 *  00815996  |> 81FF FEFFFF7F  CMP EDI,0x7FFFFFFE
 *  0081599C  |. 0F87 A1000000  JA play.00815A43
 *  008159A2  |. 8B46 14        MOV EAX,DWORD PTR DS:[ESI+0x14]
 *  008159A5  |. 3BC7           CMP EAX,EDI
 *  008159A7  |. 73 24          JNB SHORT play.008159CD
 *  008159A9  |. FF76 10        PUSH DWORD PTR DS:[ESI+0x10]
 *  008159AC  |. 8BCE           MOV ECX,ESI
 *  008159AE  |. 57             PUSH EDI
 *  008159AF  |. E8 DC000000    CALL play.00815A90
 *  008159B4  |. 8B4D 0C        MOV ECX,DWORD PTR SS:[EBP+0xC]
 *  008159B7  |. 85FF           TEST EDI,EDI
 *  008159B9  |. 74 75          JE SHORT play.00815A30
 *  008159BB  |> 837B 14 08     CMP DWORD PTR DS:[EBX+0x14],0x8
 *  008159BF  |. 72 02          JB SHORT play.008159C3
 *  008159C1  |. 8B1B           MOV EBX,DWORD PTR DS:[EBX]
 *  008159C3  |> 837E 14 08     CMP DWORD PTR DS:[ESI+0x14],0x8
 *  008159C7  |. 72 2E          JB SHORT play.008159F7
 *  008159C9  |. 8B16           MOV EDX,DWORD PTR DS:[ESI]
 *  008159CB  |. EB 2C          JMP SHORT play.008159F9	; jichi: jump
 *  008159CD  |> 85FF           TEST EDI,EDI
 *  008159CF  |.^75 EA          JNZ SHORT play.008159BB
 *  008159D1  |. 897E 10        MOV DWORD PTR DS:[ESI+0x10],EDI
 *  008159D4  |. 83F8 08        CMP EAX,0x8
 *  008159D7  |. 72 10          JB SHORT play.008159E9
 *  008159D9  |. 8B06           MOV EAX,DWORD PTR DS:[ESI]
 *  008159DB  |. 33C9           XOR ECX,ECX
 *  008159DD  |. 5F             POP EDI
 *  008159DE  |. 66:8908        MOV WORD PTR DS:[EAX],CX
 *  008159E1  |. 8BC6           MOV EAX,ESI
 *  008159E3  |. 5E             POP ESI
 *  008159E4  |. 5B             POP EBX
 *  008159E5  |. 5D             POP EBP
 *  008159E6  |. C2 0C00        RETN 0xC
 *  008159E9  |> 8BC6           MOV EAX,ESI
 *  008159EB  |. 33C9           XOR ECX,ECX
 *  008159ED  |. 5F             POP EDI
 *  008159EE  |. 5E             POP ESI
 *  008159EF  |. 5B             POP EBX
 *  008159F0  |. 66:8908        MOV WORD PTR DS:[EAX],CX
 *  008159F3  |. 5D             POP EBP
 *  008159F4  |. C2 0C00        RETN 0xC
 *  008159F7  |> 8BD6           MOV EDX,ESI
 *  008159F9  |> 85FF           TEST EDI,EDI
 *  008159FB  |. 74 11          JE SHORT play.00815A0E
 *
 *  008159FD  |. 8D043F         LEA EAX,DWORD PTR DS:[EDI+EDI]	; jichi: edi *= 2 for wchar_t
 *  00815A00  |. 50             PUSH EAX	; jichi: size
 *  00815A01  |. 8D044B         LEA EAX,DWORD PTR DS:[EBX+ECX*2]
 *  00815A04  |. 50             PUSH EAX	; jichi: source text
 *  00815A05  |. 52             PUSH EDX	; jichi: target text
 *
 *  00815A06  |. E8 C59E2200    CALL play.00A3F8D0	; jichi: called here
 *  00815A0B  |. 83C4 0C        ADD ESP,0xC
 *  00815A0E  |> 837E 14 08     CMP DWORD PTR DS:[ESI+0x14],0x8
 *  00815A12  |. 897E 10        MOV DWORD PTR DS:[ESI+0x10],EDI
 *  00815A15  |. 72 11          JB SHORT play.00815A28
 *  00815A17  |. 8B06           MOV EAX,DWORD PTR DS:[ESI]
 *  00815A19  |. 33C9           XOR ECX,ECX
 *  00815A1B  |. 66:890C78      MOV WORD PTR DS:[EAX+EDI*2],CX
 *  00815A1F  |. 8BC6           MOV EAX,ESI
 *  00815A21  |. 5F             POP EDI
 *  00815A22  |. 5E             POP ESI
 *  00815A23  |. 5B             POP EBX
 *  00815A24  |. 5D             POP EBP
 *  00815A25  |. C2 0C00        RETN 0xC
 *  00815A28  |> 8BC6           MOV EAX,ESI
 *  00815A2A  |. 33C9           XOR ECX,ECX
 *  00815A2C  |. 66:890C78      MOV WORD PTR DS:[EAX+EDI*2],CX
 *  00815A30  |> 5F             POP EDI
 *  00815A31  |. 8BC6           MOV EAX,ESI
 *  00815A33  |. 5E             POP ESI
 *  00815A34  |. 5B             POP EBX
 *  00815A35  |. 5D             POP EBP
 *  00815A36  |. C2 0C00        RETN 0xC
 *  00815A39  |> 68 A09FB000    PUSH play.00B09FA0                       ;  ASCII "invalid string position"
 *  00815A3E  |. E8 3AB22000    CALL play.00A20C7D
 *  00815A43  |> 68 B89FB000    PUSH play.00B09FB8                       ;  ASCII "string too long"
 *  00815A48  |. E8 02B22000    CALL play.00A20C4F
 *  00815A4D  |. CC             INT3
 *  00815A4E  |. CC             INT3
 *  00815A4F  |. CC             INT3
 *
 *  Alternatve pattern:
 *  Find one of caller of the push "invalid string position" instruction.
 */
bool attach(ulong startAddress, ulong stopAddress)
{
  const uint8_t bytes[] = {
    0x8d,0x04,0x3f, // 08159fd  |. 8d043f         lea eax,dword ptr ds:[edi+edi]	; jichi: edi *= 2 for wchar_t
    0x50,           // 0815a00  |. 50             push eax	; jichi: size
    0x8d,0x04,0x4b, // 0815a01  |. 8d044b         lea eax,dword ptr ds:[ebx+ecx*2]
    0x50,           // 0815a04  |. 50             push eax	; jichi: source text
    0x52            // 0815a05  |. 52             push edx	; jichi: target text
  };
  ulong addr = MemDbg::findBytes(bytes, sizeof(bytes), startAddress, stopAddress);
  if (!addr)
    return addr;
  addr = MemDbg::findEnclosingAlignedFunction(addr);
  if (!addr)
    return addr;
  //return winhook::hook_before(addr, Private::hookBefore);

  int count = 0;
  auto fun = [&count](ulong addr) -> bool {
    auto before = std::bind(Private::hookBefore, addr + 5, std::placeholders::_1);
    count += winhook::hook_both(addr, before, Private::hookAfter);
    return true; // replace all functions
  };
  MemDbg::iterNearCallAddress(fun, addr, startAddress, stopAddress);
  DOUT("call number =" << count);
  return count;
}
} // namespace ScenarioHook2

namespace OtherHook {
namespace Private {
  bool hookBefore(winhook::hook_stack *s)
  {
    static QString text_;
    auto text = (LPCWSTR)s->stack[3]; // arg3
    if (!text || !*text)
      return true;
    auto retaddr = s->stack[0];
    auto reladdr = retaddr - moduleBaseAddress_;
    enum { role = Engine::OtherRole };
    QString oldText = QString::fromWCharArray(text),
            newText = EngineController::instance()->dispatchTextW(oldText, role, reladdr);
    if (newText.isEmpty() ||  oldText == newText)
      return true;
    newText.replace("%r", "\n");
    text_ = newText;
    s->stack[3] = (ulong)text_.utf16();
    return true;
  }
} // Private
/**
 *  Sample game: ウルスラグナ征戦のデュエリス
 *  Executable description shows "AVGEngineV2"
 *
 *  Function found by back-tracking GetGlyphOutlineW
 *
 *  014D449A   CC               INT3
 *  014D449B   CC               INT3
 *  014D449C   CC               INT3
 *  014D449D   CC               INT3
 *  014D449E   CC               INT3
 *  014D449F   CC               INT3
 *  014D44A0   55               PUSH EBP
 *  014D44A1   8BEC             MOV EBP,ESP
 *  014D44A3   6A FF            PUSH -0x1
 *  014D44A5   68 80DA5801      PUSH verethra.0158DA80
 *  014D44AA   64:A1 00000000   MOV EAX,DWORD PTR FS:[0]
 *  014D44B0   50               PUSH EAX
 *  014D44B1   83EC 2C          SUB ESP,0x2C
 *  014D44B4   53               PUSH EBX
 *  014D44B5   56               PUSH ESI
 *  014D44B6   57               PUSH EDI
 *  014D44B7   A1 20B06801      MOV EAX,DWORD PTR DS:[0x168B020]
 *  014D44BC   33C5             XOR EAX,EBP
 *  014D44BE   50               PUSH EAX
 *  014D44BF   8D45 F4          LEA EAX,DWORD PTR SS:[EBP-0xC]
 *  014D44C2   64:A3 00000000   MOV DWORD PTR FS:[0],EAX
 *  014D44C8   8B45 14          MOV EAX,DWORD PTR SS:[EBP+0x14]
 *  014D44CB   C745 C8 00000000 MOV DWORD PTR SS:[EBP-0x38],0x0
 *  014D44D2   C745 CC 00000000 MOV DWORD PTR SS:[EBP-0x34],0x0
 *  014D44D9   C745 D0 00000000 MOV DWORD PTR SS:[EBP-0x30],0x0
 *  014D44E0   8B10             MOV EDX,DWORD PTR DS:[EAX]
 *  014D44E2   8B48 04          MOV ECX,DWORD PTR DS:[EAX+0x4]
 *  014D44E5   8B40 08          MOV EAX,DWORD PTR DS:[EAX+0x8]
 *  014D44E8   2BC2             SUB EAX,EDX
 *  014D44EA   8955 EC          MOV DWORD PTR SS:[EBP-0x14],EDX
 *  014D44ED   894D E8          MOV DWORD PTR SS:[EBP-0x18],ECX
 *  014D44F0   8945 E0          MOV DWORD PTR SS:[EBP-0x20],EAX
 *  014D44F3   6A 04            PUSH 0x4
 *  014D44F5   8D4D C8          LEA ECX,DWORD PTR SS:[EBP-0x38]
 *  014D44F8   C745 FC 00000000 MOV DWORD PTR SS:[EBP-0x4],0x0
 *  014D44FF   E8 8C89EBFF      CALL verethra.0138CE90
 *  014D4504   C745 D4 00000000 MOV DWORD PTR SS:[EBP-0x2C],0x0
 *  014D450B   C745 D8 00000000 MOV DWORD PTR SS:[EBP-0x28],0x0
 *  014D4512   C745 DC 00000000 MOV DWORD PTR SS:[EBP-0x24],0x0
 *  014D4519   6A 04            PUSH 0x4
 *  014D451B   8D4D D4          LEA ECX,DWORD PTR SS:[EBP-0x2C]
 *  014D451E   C645 FC 01       MOV BYTE PTR SS:[EBP-0x4],0x1
 *  014D4522   E8 697BE8FF      CALL verethra.0135C090
 *  014D4527   8B7D 10          MOV EDI,DWORD PTR SS:[EBP+0x10]
 *  014D452A   33C0             XOR EAX,EAX
 *  014D452C   33DB             XOR EBX,EBX
 *  014D452E   8945 F0          MOV DWORD PTR SS:[EBP-0x10],EAX
 *  014D4531   66:3907          CMP WORD PTR DS:[EDI],AX
 *  014D4534   0F84 C3000000    JE verethra.014D45FD
 *  014D453A   BE 01000000      MOV ESI,0x1
 *  014D453F   8945 14          MOV DWORD PTR SS:[EBP+0x14],EAX
 *  014D4542   8975 10          MOV DWORD PTR SS:[EBP+0x10],ESI
 *  014D4545   8B4D D8          MOV ECX,DWORD PTR SS:[EBP-0x28]
 *  014D4548   B8 ABAAAA2A      MOV EAX,0x2AAAAAAB
 *  014D454D   2B4D D4          SUB ECX,DWORD PTR SS:[EBP-0x2C]
 *  014D4550   F7E9             IMUL ECX
 *  014D4552   C1FA 02          SAR EDX,0x2
 *  014D4555   8BC2             MOV EAX,EDX
 *  014D4557   C1E8 1F          SHR EAX,0x1F
 *  014D455A   03C2             ADD EAX,EDX
 *  014D455C   3BC3             CMP EAX,EBX
 *  014D455E   77 09            JA SHORT verethra.014D4569
 *  014D4560   56               PUSH ESI
 *  014D4561   8D4D D4          LEA ECX,DWORD PTR SS:[EBP-0x2C]
 *  014D4564   E8 C712F4FF      CALL verethra.01415830
 *  014D4569   8B45 CC          MOV EAX,DWORD PTR SS:[EBP-0x34]
 *  014D456C   2B45 C8          SUB EAX,DWORD PTR SS:[EBP-0x38]
 *  014D456F   C1F8 02          SAR EAX,0x2
 *  014D4572   3BC3             CMP EAX,EBX
 *  014D4574   77 14            JA SHORT verethra.014D458A
 *  014D4576   8D45 E4          LEA EAX,DWORD PTR SS:[EBP-0x1C]
 *  014D4579   C745 E4 00000000 MOV DWORD PTR SS:[EBP-0x1C],0x0
 *  014D4580   50               PUSH EAX
 *  014D4581   56               PUSH ESI
 *  014D4582   8D4D C8          LEA ECX,DWORD PTR SS:[EBP-0x38]
 *  014D4585   E8 06D2EDFF      CALL verethra.013B1790
 *  014D458A   0FB707           MOVZX EAX,WORD PTR DS:[EDI]
 *  014D458D   66:3B45 28       CMP AX,WORD PTR SS:[EBP+0x28]
 *  014D4591   74 4F            JE SHORT verethra.014D45E2
 *  014D4593   8B75 0C          MOV ESI,DWORD PTR SS:[EBP+0xC]
 *  014D4596   50               PUSH EAX
 *  014D4597   8B76 18          MOV ESI,DWORD PTR DS:[ESI+0x18]
 *  014D459A   E8 41100000      CALL verethra.014D55E0
 *  014D459F   8B4D 14          MOV ECX,DWORD PTR SS:[EBP+0x14]
 *  014D45A2   83C4 04          ADD ESP,0x4
 *  014D45A5   034D D4          ADD ECX,DWORD PTR SS:[EBP-0x2C]
 *  014D45A8   0FAFC6           IMUL EAX,ESI
 *  014D45AB   8B75 F0          MOV ESI,DWORD PTR SS:[EBP-0x10]
 *
 *  014D45AE   99               CDQ
 *  014D45AF   2BC2             SUB EAX,EDX
 *  014D45B1   D1F8             SAR EAX,1
 *  014D45B3   03F0             ADD ESI,EAX
 *  014D45B5   0FB707           MOVZX EAX,WORD PTR DS:[EDI]
 *
 *  014D45B8   50               PUSH EAX
 *  014D45B9   6A 01            PUSH 0x1
 *  014D45BB   8975 F0          MOV DWORD PTR SS:[EBP-0x10],ESI
 *  014D45BE   E8 5D40ECFF      CALL verethra.01398620
 *  014D45C3   8B55 C8          MOV EDX,DWORD PTR SS:[EBP-0x38]
 *  014D45C6   8BCE             MOV ECX,ESI
 *  014D45C8   8B45 0C          MOV EAX,DWORD PTR SS:[EBP+0xC]
 *  014D45CB   39349A           CMP DWORD PTR DS:[EDX+EBX*4],ESI
 *  014D45CE   0F4F0C9A         CMOVG ECX,DWORD PTR DS:[EDX+EBX*4]
 *  014D45D2   890C9A           MOV DWORD PTR DS:[EDX+EBX*4],ECX
 *  014D45D5   8B40 18          MOV EAX,DWORD PTR DS:[EAX+0x18]
 *  014D45D8   03C6             ADD EAX,ESI
 *  014D45DA   8B75 10          MOV ESI,DWORD PTR SS:[EBP+0x10]
 *  014D45DD   3B45 E0          CMP EAX,DWORD PTR SS:[EBP-0x20]
 *  014D45E0   7C 0E            JL SHORT verethra.014D45F0
 *  014D45E2   33C0             XOR EAX,EAX
 *  014D45E4   46               INC ESI
 *  014D45E5   43               INC EBX
 *  014D45E6   8975 10          MOV DWORD PTR SS:[EBP+0x10],ESI
 *  014D45E9   8345 14 18       ADD DWORD PTR SS:[EBP+0x14],0x18
 *  014D45ED   8945 F0          MOV DWORD PTR SS:[EBP-0x10],EAX
 *  014D45F0   83C7 02          ADD EDI,0x2
 *  014D45F3   66:833F 00       CMP WORD PTR DS:[EDI],0x0
 *  014D45F7  ^0F85 48FFFFFF    JNZ verethra.014D4545
 *  014D45FD   8B75 D8          MOV ESI,DWORD PTR SS:[EBP-0x28]
 *  014D4600   B8 ABAAAA2A      MOV EAX,0x2AAAAAAB
 *  014D4605   8B5D D4          MOV EBX,DWORD PTR SS:[EBP-0x2C]
 *  014D4608   8BCE             MOV ECX,ESI
 *  014D460A   2BCB             SUB ECX,EBX
 *  014D460C   F7E9             IMUL ECX
 *  014D460E   C1FA 02          SAR EDX,0x2
 *  014D4611   8BCA             MOV ECX,EDX
 *  014D4613   C1E9 1F          SHR ECX,0x1F
 *  014D4616   03CA             ADD ECX,EDX
 *  014D4618   894D E0          MOV DWORD PTR SS:[EBP-0x20],ECX
 *  014D461B   75 53            JNZ SHORT verethra.014D4670
 *  014D461D   8B7D 08          MOV EDI,DWORD PTR SS:[EBP+0x8]
 *  014D4620   8B4D EC          MOV ECX,DWORD PTR SS:[EBP-0x14]
 *  014D4623   8B45 E8          MOV EAX,DWORD PTR SS:[EBP-0x18]
 *  014D4626   890F             MOV DWORD PTR DS:[EDI],ECX
 *  014D4628   8947 04          MOV DWORD PTR DS:[EDI+0x4],EAX
 *  014D462B   894F 08          MOV DWORD PTR DS:[EDI+0x8],ECX
 *  014D462E   8947 0C          MOV DWORD PTR DS:[EDI+0xC],EAX
 *  014D4631   85DB             TEST EBX,EBX
 *  014D4633   74 17            JE SHORT verethra.014D464C
 *  014D4635   FF75 14          PUSH DWORD PTR SS:[EBP+0x14]
 *  014D4638   8D45 17          LEA EAX,DWORD PTR SS:[EBP+0x17]
 *  014D463B   50               PUSH EAX
 *  014D463C   56               PUSH ESI
 *  014D463D   53               PUSH EBX
 *  014D463E   E8 3D22E8FF      CALL verethra.01356880
 *  014D4643   53               PUSH EBX
 *  014D4644   E8 8CAE0600      CALL verethra.0153F4D5
 *  014D4649   83C4 14          ADD ESP,0x14
 *  014D464C   8B45 C8          MOV EAX,DWORD PTR SS:[EBP-0x38]
 *  014D464F   85C0             TEST EAX,EAX
 *  014D4651   74 09            JE SHORT verethra.014D465C
 *  014D4653   50               PUSH EAX
 *  014D4654   E8 7CAE0600      CALL verethra.0153F4D5
 *  014D4659   83C4 04          ADD ESP,0x4
 *  014D465C   8BC7             MOV EAX,EDI
 *  014D465E   8B4D F4          MOV ECX,DWORD PTR SS:[EBP-0xC]
 *  014D4661   64:890D 00000000 MOV DWORD PTR FS:[0],ECX
 *  014D4668   59               POP ECX
 *  014D4669   5F               POP EDI
 *  014D466A   5E               POP ESI
 *  014D466B   5B               POP EBX
 *  014D466C   8BE5             MOV ESP,EBP
 *  014D466E   5D               POP EBP
 *  014D466F   C3               RETN
 *  014D4670   33C0             XOR EAX,EAX
 *  014D4672   C745 E4 00000000 MOV DWORD PTR SS:[EBP-0x1C],0x0
 *  014D4679   33F6             XOR ESI,ESI
 *  014D467B   8945 10          MOV DWORD PTR SS:[EBP+0x10],EAX
 *  014D467E   85C9             TEST ECX,ECX
 *  014D4680   0F84 C0000000    JE verethra.014D4746
 *  014D4686   8B4D EC          MOV ECX,DWORD PTR SS:[EBP-0x14]
 *  014D4689   8BFB             MOV EDI,EBX
 *  014D468B   8B5D 0C          MOV EBX,DWORD PTR SS:[EBP+0xC]
 *  014D468E   894D E4          MOV DWORD PTR SS:[EBP-0x1C],ECX
 *  014D4691   66:0F6EC9        MOVD MM1,ECX
 *  014D4695   0F5B             ???                                      ; Unknown command
 *  014D4697   C9               LEAVE
 *  014D4698   F3:0F114D F0     MOVSS DWORD PTR SS:[EBP-0x10],XMM1
 *  014D469D   8D49 00          LEA ECX,DWORD PTR DS:[ECX]
 *  014D46A0   8B73 18          MOV ESI,DWORD PTR DS:[EBX+0x18]
 *  014D46A3   0FAFF0           IMUL ESI,EAX
 *  014D46A6   0375 E8          ADD ESI,DWORD PTR SS:[EBP-0x18]
 *  014D46A9   837F 14 08       CMP DWORD PTR DS:[EDI+0x14],0x8
 *  014D46AD   72 07            JB SHORT verethra.014D46B6
 *  014D46AF   8B07             MOV EAX,DWORD PTR DS:[EDI]
 *  014D46B1   8945 14          MOV DWORD PTR SS:[EBP+0x14],EAX
 *  014D46B4   EB 05            JMP SHORT verethra.014D46BB
 *  014D46B6   8BC7             MOV EAX,EDI
 *  014D46B8   897D 14          MOV DWORD PTR SS:[EBP+0x14],EDI
 *  014D46BB   FF75 28          PUSH DWORD PTR SS:[EBP+0x28]
 *  014D46BE   66:0F6EC6        MOVD MM0,ESI
 *  014D46C2   FF75 24          PUSH DWORD PTR SS:[EBP+0x24]
 *  014D46C5   0F5B             ???                                      ; Unknown command
 *  014D46C7   C06A 00 FF       SHR BYTE PTR DS:[EDX],0xFF               ; Shift constant out of range 1..31
 *  014D46CB   75 20            JNZ SHORT verethra.014D46ED
 *  014D46CD   68 A0956901      PUSH verethra.016995A0
 *  014D46D2   83EC 0C          SUB ESP,0xC
 *  014D46D5   C74424 08 000080>MOV DWORD PTR SS:[ESP+0x8],0x3F800000
 *  014D46DD   F3:0F114424 04   MOVSS DWORD PTR SS:[ESP+0x4],XMM0
 *  014D46E3   F3:0F110C24      MOVSS DWORD PTR SS:[ESP],XMM1
 *  014D46E8   50               PUSH EAX
 *  014D46E9   53               PUSH EBX
 *  014D46EA   E8 7156F4FF      CALL verethra.01419D60
 *  014D46EF   83C4 28          ADD ESP,0x28
 *  014D46F2   66:0F6EC6        MOVD MM0,ESI
 *  014D46F6   0F5B             ???                                      ; Unknown command
 *  014D46F8   C0FF 75          SAR BH,0x75                              ; Shift constant out of range 1..31
 *  014D46FB   28FF             SUB BH,BH
 *  014D46FD   75 24            JNZ SHORT verethra.014D4723
 *  014D46FF   FF75 1C          PUSH DWORD PTR SS:[EBP+0x1C]
 *  014D4702   FF75 18          PUSH DWORD PTR SS:[EBP+0x18]
 *  014D4705   68 A0956901      PUSH verethra.016995A0
 *  014D470A   83EC 08          SUB ESP,0x8
 *  014D470D   F3:0F114424 04   MOVSS DWORD PTR SS:[ESP+0x4],XMM0
 *  014D4713   F3:0F1045 F0     MOVSS XMM0,DWORD PTR SS:[EBP-0x10]
 *  014D4718   F3:0F110424      MOVSS DWORD PTR SS:[ESP],XMM0
 *  014D471D   FF75 14          PUSH DWORD PTR SS:[EBP+0x14]
 *  014D4720   53               PUSH EBX
 *  014D4721   E8 2AF5FFFF      CALL verethra.014D3C50
 *  014D4726   8B45 10          MOV EAX,DWORD PTR SS:[EBP+0x10]
 *  014D4729   83C4 24          ADD ESP,0x24
 *  014D472C   8B4D E0          MOV ECX,DWORD PTR SS:[EBP-0x20]
 *  014D472F   40               INC EAX
 *  014D4730   F3:0F104D F0     MOVSS XMM1,DWORD PTR SS:[EBP-0x10]
 *  014D4735   83C7 18          ADD EDI,0x18
 *  014D4738   8945 10          MOV DWORD PTR SS:[EBP+0x10],EAX
 *  014D473B   3BC1             CMP EAX,ECX
 *  014D473D  ^0F82 5DFFFFFF    JB verethra.014D46A0
 *  014D4743   8B5D D4          MOV EBX,DWORD PTR SS:[EBP-0x2C]
 *  014D4746   8B55 08          MOV EDX,DWORD PTR SS:[EBP+0x8]
 *  014D4749   8B45 EC          MOV EAX,DWORD PTR SS:[EBP-0x14]
 *  014D474C   8B7D C8          MOV EDI,DWORD PTR SS:[EBP-0x38]
 *  014D474F   8902             MOV DWORD PTR DS:[EDX],EAX
 *  014D4751   8B45 E8          MOV EAX,DWORD PTR SS:[EBP-0x18]
 *  014D4754   8942 04          MOV DWORD PTR DS:[EDX+0x4],EAX
 *  014D4757   8B448F FC        MOV EAX,DWORD PTR DS:[EDI+ECX*4-0x4]
 *  014D475B   0345 E4          ADD EAX,DWORD PTR SS:[EBP-0x1C]
 *  014D475E   8942 08          MOV DWORD PTR DS:[EDX+0x8],EAX
 *  014D4761   8B45 0C          MOV EAX,DWORD PTR SS:[EBP+0xC]
 *  014D4764   8B40 18          MOV EAX,DWORD PTR DS:[EAX+0x18]
 *  014D4767   03C6             ADD EAX,ESI
 *  014D4769   8942 0C          MOV DWORD PTR DS:[EDX+0xC],EAX
 *  014D476C   85DB             TEST EBX,EBX
 *  014D476E   74 46            JE SHORT verethra.014D47B6
 *  014D4770   8B45 D8          MOV EAX,DWORD PTR SS:[EBP-0x28]
 *  014D4773   8BF3             MOV ESI,EBX
 *  014D4775   3BD8             CMP EBX,EAX
 *  014D4777   74 34            JE SHORT verethra.014D47AD
 *  014D4779   8DA424 00000000  LEA ESP,DWORD PTR SS:[ESP]
 *  014D4780   837E 14 08       CMP DWORD PTR DS:[ESI+0x14],0x8
 *  014D4784   72 0D            JB SHORT verethra.014D4793
 *  014D4786   FF36             PUSH DWORD PTR DS:[ESI]
 *  014D4788   E8 48AD0600      CALL verethra.0153F4D5
 *  014D478D   8B45 D8          MOV EAX,DWORD PTR SS:[EBP-0x28]
 *  014D4790   83C4 04          ADD ESP,0x4
 *  014D4793   33C9             XOR ECX,ECX
 *  014D4795   C746 14 07000000 MOV DWORD PTR DS:[ESI+0x14],0x7
 *  014D479C   C746 10 00000000 MOV DWORD PTR DS:[ESI+0x10],0x0
 *  014D47A3   66:890E          MOV WORD PTR DS:[ESI],CX
 *  014D47A6   83C6 18          ADD ESI,0x18
 *  014D47A9   3BF0             CMP ESI,EAX
 *  014D47AB  ^75 D3            JNZ SHORT verethra.014D4780
 *  014D47AD   53               PUSH EBX
 *  014D47AE   E8 22AD0600      CALL verethra.0153F4D5
 *  014D47B3   83C4 04          ADD ESP,0x4
 *  014D47B6   85FF             TEST EDI,EDI
 *  014D47B8   74 09            JE SHORT verethra.014D47C3
 *  014D47BA   57               PUSH EDI
 *  014D47BB   E8 15AD0600      CALL verethra.0153F4D5
 *  014D47C0   83C4 04          ADD ESP,0x4
 *  014D47C3   8B45 08          MOV EAX,DWORD PTR SS:[EBP+0x8]
 *  014D47C6   8B4D F4          MOV ECX,DWORD PTR SS:[EBP-0xC]
 *  014D47C9   64:890D 00000000 MOV DWORD PTR FS:[0],ECX
 *  014D47D0   59               POP ECX
 *  014D47D1   5F               POP EDI
 *  014D47D2   5E               POP ESI
 *  014D47D3   5B               POP EBX
 *  014D47D4   8BE5             MOV ESP,EBP
 *  014D47D6   5D               POP EBP
 *  014D47D7   C3               RETN
 *  014D47D8   CC               INT3
 *  014D47D9   CC               INT3
 *  014D47DA   CC               INT3
 *  014D47DB   CC               INT3
 *  014D47DC   CC               INT3
 *  014D47DD   CC               INT3
 *  014D47DE   CC               INT3
 *  014D47DF   CC               INT3
 */
bool attach(ulong startAddress, ulong stopAddress)
{
  const uint8_t bytes[] = {
    0x99,           // 014d45ae   99               cdq
    0x2b,0xc2,      // 014d45af   2bc2             sub eax,edx
    0xd1,0xf8,      // 014d45b1   d1f8             sar eax,1
    0x03,0xf0,      // 014d45b3   03f0             add esi,eax
    0x0f,0xb7,0x07  // 014d45b5   0fb707           movzx eax,word ptr ds:[edi]
  };
  int count = 0;
  auto fun = [&count](ulong addr) -> bool {
    count +=
        (addr = MemDbg::findEnclosingAlignedFunction(addr))
        && winhook::hook_before(addr, Private::hookBefore);
    return true;
  };
  MemDbg::iterFindBytes(fun, bytes, sizeof(bytes), startAddress, stopAddress);
  DOUT("call number =" << count);
  return count;
}
} // namespace OtherHook

#if 0
namespace DebugHook {
namespace Private {
  bool hookBefore(winhook::hook_stack *s)
  {
    static QString text_;
    auto text = (LPCWSTR)s->stack[1]; // arg1
    if (!text || !*text || text[0] <= 127)
      return true;
    auto retaddr = s->stack[0];
    auto reladdr = retaddr - moduleBaseAddress_;
    if (reladdr == 0x25660 || reladdr == 0x28301)
      return true;
    auto role = Engine::OtherRole;
    QString oldText = QString::fromWCharArray(text),
            newText = EngineController::instance()->dispatchTextW(oldText, role, reladdr);
    if (newText.isEmpty() ||  oldText == newText)
      return true;
    text_ = newText;
    s->stack[1] = (ulong)text_.utf16();
    return true;
  }
} // Private
/**
 *  Sample game: 塔の下のエクセルキトゥス体験版
 *  Executable description shows "AVGEngineV2"
 *
 *  This is the memcpy function.
 *
 *  arg1: target text
 *  arg2: source tex
 *  arg3: size
 *
 *  00A3F8CB     CC             INT3
 *  00A3F8CC     CC             INT3
 *  00A3F8CD     CC             INT3
 *  00A3F8CE     CC             INT3
 *  00A3F8CF     CC             INT3
 *  00A3F8D0   $ 57             PUSH EDI
 *  00A3F8D1   . 56             PUSH ESI
 *  00A3F8D2   . 8B7424 10      MOV ESI,DWORD PTR SS:[ESP+0x10]
 *  00A3F8D6   . 8B4C24 14      MOV ECX,DWORD PTR SS:[ESP+0x14]
 *  00A3F8DA   . 8B7C24 0C      MOV EDI,DWORD PTR SS:[ESP+0xC]
 *  00A3F8DE   . 8BC1           MOV EAX,ECX
 *  00A3F8E0   . 8BD1           MOV EDX,ECX
 *  00A3F8E2   . 03C6           ADD EAX,ESI
 *  00A3F8E4   . 3BFE           CMP EDI,ESI
 *  00A3F8E6   . 76 08          JBE SHORT play.00A3F8F0
 *  00A3F8E8   . 3BF8           CMP EDI,EAX
 *  00A3F8EA   . 0F82 68030000  JB play.00A3FC58
 *  00A3F8F0   > 0FBA25 581FBC0>BT DWORD PTR DS:[0xBC1F58],0x1
 *  00A3F8F8   . 73 07          JNB SHORT play.00A3F901
 *  00A3F8FA   . F3:A4          REP MOVS BYTE PTR ES:[EDI],BYTE PTR DS:[ESI]	; jichi: here
 *  00A3F8FC   . E9 17030000    JMP play.00A3FC18
 *  00A3F901   > 81F9 80000000  CMP ECX,0x80
 *  00A3F907   . 0F82 CE010000  JB play.00A3FADB
 *  00A3F90D   . 8BC7           MOV EAX,EDI
 *  00A3F90F   . 33C6           XOR EAX,ESI
 *  00A3F911   . A9 0F000000    TEST EAX,0xF
 *  00A3F916   . 75 0E          JNZ SHORT play.00A3F926
 *  00A3F918   . 0FBA25 3060B90>BT DWORD PTR DS:[0xB96030],0x1
 *  00A3F920   . 0F82 DA040000  JB play.00A3FE00
 *  00A3F926   > 0FBA25 581FBC0>BT DWORD PTR DS:[0xBC1F58],0x0
 *  00A3F92E   . 0F83 A7010000  JNB play.00A3FADB
 *  00A3F934   . F7C7 03000000  TEST EDI,0x3
 *  00A3F93A   . 0F85 B8010000  JNZ play.00A3FAF8
 *  00A3F940   . F7C6 03000000  TEST ESI,0x3
 *  00A3F946   . 0F85 97010000  JNZ play.00A3FAE3
 *  00A3F94C   . 0FBAE7 02      BT EDI,0x2
 *  00A3F950   . 73 0D          JNB SHORT play.00A3F95F
 *  00A3F952   . 8B06           MOV EAX,DWORD PTR DS:[ESI]
 *  00A3F954   . 83E9 04        SUB ECX,0x4
 *  00A3F957   . 8D76 04        LEA ESI,DWORD PTR DS:[ESI+0x4]
 *  00A3F95A   . 8907           MOV DWORD PTR DS:[EDI],EAX
 *  00A3F95C   . 8D7F 04        LEA EDI,DWORD PTR DS:[EDI+0x4]
 *  00A3F95F   > 0FBAE7 03      BT EDI,0x3
 *  00A3F963   . 73 11          JNB SHORT play.00A3F976
 *  00A3F965   . F3:            PREFIX REP:                              ;  Superfluous prefix
 *  00A3F966   . 0F7E0E         MOVD DWORD PTR DS:[ESI],MM1
 *  00A3F969   . 83E9 08        SUB ECX,0x8
 *  00A3F96C   . 8D76 08        LEA ESI,DWORD PTR DS:[ESI+0x8]
 *  00A3F96F     66             DB 66                                    ;  CHAR 'f'
 *  00A3F970     0F             DB 0F
 *  00A3F971     D6             DB D6
 *  00A3F972     0F             DB 0F
 */
bool attach(ulong startAddress, ulong stopAddress)
{
  /*
  // There are two exact the same functions
  // The last function is hooked.
  // It seems the functions are generated by inlining
  const uint8_t bytes[] = {
    0x57,                 // 00a3f8d0   $ 57             push edi
    0x56,                 // 00a3f8d1   . 56             push esi
    0x8b,0x74,0x24, 0x10, // 00a3f8d2   . 8b7424 10      mov esi,dword ptr ss:[esp+0x10]
    0x8b,0x4c,0x24, 0x14, // 00a3f8d6   . 8b4c24 14      mov ecx,dword ptr ss:[esp+0x14]
    0x8b,0x7c,0x24, 0x0c  // 00a3f8da   . 8b7c24 0c      mov edi,dword ptr ss:[esp+0xc]
  };
  ulong lastCall = 0;
  auto fun = [&lastCall](ulong addr) -> bool {
    //if (lastCall)
    lastCall = addr;
    return true;
  };
  MemDbg::iterFindBytes(fun, bytes, sizeof(bytes), startAddress, stopAddress);
  return lastCall && winhook::hook_before(lastCall, Private::hookBefore);
  */

  const uint8_t bytes[] = {
    0x8b,0xf1,                              // 001d45e9  |. 8bf1                mov esi,ecx
    0xc7,0x46, 0x14, 0x07,0x00,0x00,0x00,   // 001d45eb  |. c746 14 07000000    mov dword ptr ds:[esi+0x14],0x7
    0xc7,0x46, 0x10, 0x00,0x00,0x00,0x00,   // 001d45f2  |. c746 10 00000000    mov dword ptr ds:[esi+0x10],0x0
    0x66,0x89,0x06,                         // 001d45f9  |. 66:8906             mov word ptr ds:[esi],ax
    0x66,0x39,0x02                          // 001d45fc  |. 66:3902             cmp word ptr ds:[edx],ax   ; jichi: debug breakpoint stops here
  };
  ulong addr = MemDbg::findBytes(bytes, sizeof(bytes), startAddress, stopAddress);
  if (!addr)
    return false;
  addr = MemDbg::findEnclosingAlignedFunction(addr);
  if (!addr)
    return false;
  return winhook::hook_before(addr, Private::hookBefore);
}
} // namespace DebugHook
#endif // 0

} // unnamed namespace

/** Public class */

bool GXPEngine::attach()
{
  ulong startAddress, stopAddress;
  if (!Engine::getProcessMemoryRange(&startAddress, &stopAddress))
    return false;

  moduleBaseAddress_ = startAddress; // used to calculate reladdr for debug purposes
  //if (ScenarioHook1::attach(startAddress, stopAddress))
  //  DOUT("found GXP1");
  if (ScenarioHook2::attach(startAddress, stopAddress)) {
    DOUT("found GXP2");

    if (OtherHook::attach(startAddress, stopAddress))
      DOUT("other text found");
    else
      DOUT("other text NOT FOUND");
  }

  //HijackManager::instance()->attachFunction((ulong)::GetGlyphOutlineW);
  return true;
}

// EOF

#if 0

namespace ChoiceHook { // FIXME: I am not able to distinguish Choice text out
namespace Private {
  bool hookBefore(winhook::hook_stack *s)
  {
    static QString text_;
    auto text = (LPCWSTR)s->eax;
    if (!text || !*text
        || text[0] <= 127)
      return true;
    size_t size = ::wcslen(text);
    if(text[size - 1] <= 127
       || text[size + 1] != 0 || text[size + 2] == 0)
      return true;
    auto retaddr = s->stack[2];
    auto reladdr = retaddr - moduleBaseAddress_;
    if (reladdr != 0x218d1)
      return true;
    auto role = Engine::ChoiceRole;
    QString oldText = QString::fromWCharArray(text),
            newText = EngineController::instance()->dispatchTextW(oldText, role, reladdr);
    if (newText.isEmpty() ||  oldText == newText)
      return true;
    //if (text[0] != 0x30d7)
    //  return true;
    text_ = newText;
    s->eax = (ulong)text_.utf16();
    return true;
  }
} // Private
/**
 *  Sample game: 塔の下のエクセルキトゥス体験版
 *  Executable description shows "AVGEngineV2"
 *
 *  arg1: source text
 *  arg2: target text to return
 *
 *  00E8562C     CC               INT3
 *  00E8562D     CC               INT3
 *  00E8562E     CC               INT3
 *  00E8562F     CC               INT3
 *  00E85630  /$ 55               PUSH EBP
 *  00E85631  |. 8BEC             MOV EBP,ESP
 *  00E85633  |. 51               PUSH ECX
 *  00E85634  |. 8B41 0C          MOV EAX,DWORD PTR DS:[ECX+0xC]	; jichi: text here in [ecx+0xc]
 *  00E85637  |. 8B4D 08          MOV ECX,DWORD PTR SS:[EBP+0x8]
 *  00E8563A  |. C745 FC 00000000 MOV DWORD PTR SS:[EBP-0x4],0x0
 *  00E85641  |. 56               PUSH ESI
 *  00E85642  |. 85C0             TEST EAX,EAX
 *  00E85644  |. 75 14            JNZ SHORT play.00E8565A
 *  00E85646  |. 68 C89F1501      PUSH play.01159FC8                       ; /Arg1 = 01159FC8
 *  00E8564B  |. E8 90EFFDFF      CALL play.00E645E0                       ; \play.001D45E0
 *  00E85650  |. 8B45 08          MOV EAX,DWORD PTR SS:[EBP+0x8]
 *  00E85653  |. 5E               POP ESI
 *  00E85654  |. 8BE5             MOV ESP,EBP
 *  00E85656  |. 5D               POP EBP
 *  00E85657  |. C2 0400          RETN 0x4
 *  00E8565A  |> 50               PUSH EAX                                 ; /Arg1
 *  00E8565B  |. E8 80EFFDFF      CALL play.00E645E0                       ; \play.001D45E0
 *  00E85660  |. 8B45 08          MOV EAX,DWORD PTR SS:[EBP+0x8]
 *  00E85663  |. 5E               POP ESI
 *  00E85664  |. 8BE5             MOV ESP,EBP
 *  00E85666  |. 5D               POP EBP
 *  00E85667  \. C2 0400          RETN 0x4
 *  00E8566A     CC               INT3
 *  00E8566B     CC               INT3
 *  00E8566C     CC               INT3
 *  00E8566D     CC               INT3
 */
bool attach(ulong startAddress, ulong stopAddress)
{
  const uint8_t bytes[] = {
    0x55,                                // 00e85630  /$ 55               push ebp
    0x8b,0xec,                           // 00e85631  |. 8bec             mov ebp,esp
    0x51,                                // 00e85633  |. 51               push ecx
    0x8b,0x41, 0x0c,                     // 00e85634  |. 8b41 0c          mov eax,dword ptr ds:[ecx+0xc]	; jichi: text here in [ecx+0xc]
    0x8b,0x4d, 0x08,                     // 00e85637  |. 8b4d 08          mov ecx,dword ptr ss:[ebp+0x8]
    0xc7,0x45, 0xfc, 0x00,0x00,0x00,0x00 // 00e8563a  |. c745 fc 00000000 mov dword ptr ss:[ebp-0x4],0x0
  };
  enum { addr_offset = 0x00e85637 - 0x00e85630 };
  ulong addr = MemDbg::findBytes(bytes, sizeof(bytes), startAddress, stopAddress);
  if (!addr)
    return false;
  return winhook::hook_before(addr + addr_offset, Private::hookBefore);
}
} // namespace ChoiceHook

namespace OtherHook {
namespace Private {
  struct HookArgument
  {
    DWORD unknown[3];
    LPCWSTR text; // 0xc

    bool isValid() const
    {
      return Engine::isAddressWritable(text)
          && text[0] > 127 && text[::wcslen(text) - 1] > 127; // skip ascii text
    }
  };
  typedef HookArgument *(__fastcall *hook_fun_t)(void *ecx, void *edx, int offset1, int offset2);
  hook_fun_t oldHookFun;
  HookArgument * __fastcall newHookFun(void *ecx, void *edx, ulong offset1, ulong offset2)
  {
    auto arg = oldHookFun(ecx, edx, offset1, offset2);
    if (arg && arg->isValid()) {
      auto role = Engine::OtherRole;
      ulong split = (offset1 << 4) | offset2;
      auto sig = split;
      QString oldText = QString::fromWCharArray(arg->text),
              newText = EngineController::instance()->dispatchTextW(oldText, role, sig);
    }
    return arg;
  }
} // Private
/**
 *  Sample game: 塔の下のエクセルキトゥス体験版
 *  Executable description shows "AVGEngineV2"
 *
 *  arg1 and arg2 are offsets
 *  ecx is this
 *  eax is retaddr
 *
 *  00FC489E     CC             INT3
 *  00FC489F     CC             INT3
 *  00FC48A0  /$ 55             PUSH EBP
 *  00FC48A1  |. 8BEC           MOV EBP,ESP
 *  00FC48A3  |. A1 704D6201    MOV EAX,DWORD PTR DS:[0x1624D70]
 *  00FC48A8  |. 56             PUSH ESI
 *  00FC48A9  |. 8BF1           MOV ESI,ECX
 *  00FC48AB  |. A8 01          TEST AL,0x1
 *  00FC48AD  |. 75 48          JNZ SHORT play.00FC48F7
 *  00FC48AF  |. 83C8 01        OR EAX,0x1
 *  00FC48B2  |. C705 784D6201 >MOV DWORD PTR DS:[0x1624D78],play.012AC4>
 *  00FC48BC  |. 0F57C0         XORPS XMM0,XMM0
 *  00FC48BF  |. A3 704D6201    MOV DWORD PTR DS:[0x1624D70],EAX
 *  00FC48C4  |. 68 004F2201    PUSH play.01224F00                       ; /Arg1 = 01224F00
 *  00FC48C9  |. C705 884D6201 >MOV DWORD PTR DS:[0x1624D88],0x11        ; |
 *  00FC48D3  |. 66:0F1305 904D>MOVLPS QWORD PTR DS:[0x1624D90],XMM0     ; |
 *  00FC48DB  |. C705 804D6201 >MOV DWORD PTR DS:[0x1624D80],0x0         ; |
 *  00FC48E5  |. C705 844D6201 >MOV DWORD PTR DS:[0x1624D84],0x0         ; |
 *  00FC48EF  |. E8 23942000    CALL play.011CDD17                       ; \play.003FDD17
 *  00FC48F4  |. 83C4 04        ADD ESP,0x4
 *  00FC48F7  |> 8B4D 08        MOV ECX,DWORD PTR SS:[EBP+0x8]
 *  00FC48FA  |. 85C9           TEST ECX,ECX
 *  00FC48FC  |. 78 36          JS SHORT play.00FC4934
 *  00FC48FE  |. 8B55 0C        MOV EDX,DWORD PTR SS:[EBP+0xC]
 *  00FC4901  |. 85D2           TEST EDX,EDX
 *  00FC4903  |. 78 2F          JS SHORT play.00FC4934
 *  00FC4905  |. 8B46 38        MOV EAX,DWORD PTR DS:[ESI+0x38]
 *  00FC4908  |. 2B46 34        SUB EAX,DWORD PTR DS:[ESI+0x34]
 *  00FC490B  |. C1F8 04        SAR EAX,0x4
 *  00FC490E  |. 3BC1           CMP EAX,ECX
 *  00FC4910  |. 7E 22          JLE SHORT play.00FC4934
 *  00FC4912  |. 8B46 34        MOV EAX,DWORD PTR DS:[ESI+0x34]
 *  00FC4915  |. C1E1 04        SHL ECX,0x4
 *  00FC4918  |. 03C1           ADD EAX,ECX
 *  00FC491A  |. 8B48 08        MOV ECX,DWORD PTR DS:[EAX+0x8]
 *  00FC491D  |. 2B48 04        SUB ECX,DWORD PTR DS:[EAX+0x4]
 *  00FC4920  |. C1F9 05        SAR ECX,0x5
 *  00FC4923  |. 3BCA           CMP ECX,EDX
 *  00FC4925  |. 7E 0D          JLE SHORT play.00FC4934
 *  00FC4927  |. C1E2 05        SHL EDX,0x5
 *  00FC492A  |. 0350 04        ADD EDX,DWORD PTR DS:[EAX+0x4]
 *  00FC492D  |. 8BC2           MOV EAX,EDX
 *  00FC492F  |. 5E             POP ESI
 *  00FC4930  |. 5D             POP EBP
 *  00FC4931  |. C2 0800        RETN 0x8
 *  00FC4934  |> B8 784D6201    MOV EAX,play.01624D78
 *  00FC4939  |. 5E             POP ESI
 *  00FC493A  |. 5D             POP EBP
 *  00FC493B  \. C2 0800        RETN 0x8
 *  00FC493E     CC             INT3
 *  00FC493F     CC             INT3
 */
bool attach(ulong startAddress, ulong stopAddress)
{
  const uint8_t bytes[] = {
    0x56,           // 00fc48a8  |. 56             push esi
    0x8b,0xf1,      // 00fc48a9  |. 8bf1           mov esi,ecx
    0xa8, 0x01,     // 00fc48ab  |. a8 01          test al,0x1
    0x75, 0x48,     // 00fc48ad  |. 75 48          jnz short play.00fc48f7
    0x83,0xc8, 0x01 // 00fc48af  |. 83c8 01        or eax,0x1
  };
  ulong addr = MemDbg::findBytes(bytes, sizeof(bytes), startAddress, stopAddress);
  if (!addr)
    return false;
  addr = MemDbg::findEnclosingAlignedFunction(addr);
  if (!addr)
    return false;
  return Private::oldHookFun = (Private::hook_fun_t)winhook::replace_fun(addr, (ulong)Private::newHookFun);
}
} // namespace OtherHook

/**
 *  Sample game: 塔の下のエクセルキトゥス体験版
 *  Executable description shows "AVGEngineV2"
 *
 *  This is the memcpy function.
 *
 *  arg1: target text
 *  arg2: source tex
 *  arg3: size
 *
 *  00A3F8CB     CC             INT3
 *  00A3F8CC     CC             INT3
 *  00A3F8CD     CC             INT3
 *  00A3F8CE     CC             INT3
 *  00A3F8CF     CC             INT3
 *  00A3F8D0   $ 57             PUSH EDI
 *  00A3F8D1   . 56             PUSH ESI
 *  00A3F8D2   . 8B7424 10      MOV ESI,DWORD PTR SS:[ESP+0x10]
 *  00A3F8D6   . 8B4C24 14      MOV ECX,DWORD PTR SS:[ESP+0x14]
 *  00A3F8DA   . 8B7C24 0C      MOV EDI,DWORD PTR SS:[ESP+0xC]
 *  00A3F8DE   . 8BC1           MOV EAX,ECX
 *  00A3F8E0   . 8BD1           MOV EDX,ECX
 *  00A3F8E2   . 03C6           ADD EAX,ESI
 *  00A3F8E4   . 3BFE           CMP EDI,ESI
 *  00A3F8E6   . 76 08          JBE SHORT play.00A3F8F0
 *  00A3F8E8   . 3BF8           CMP EDI,EAX
 *  00A3F8EA   . 0F82 68030000  JB play.00A3FC58
 *  00A3F8F0   > 0FBA25 581FBC0>BT DWORD PTR DS:[0xBC1F58],0x1
 *  00A3F8F8   . 73 07          JNB SHORT play.00A3F901
 *  00A3F8FA   . F3:A4          REP MOVS BYTE PTR ES:[EDI],BYTE PTR DS:[ESI]	; jichi: here
 *  00A3F8FC   . E9 17030000    JMP play.00A3FC18
 *  00A3F901   > 81F9 80000000  CMP ECX,0x80
 *  00A3F907   . 0F82 CE010000  JB play.00A3FADB
 *  00A3F90D   . 8BC7           MOV EAX,EDI
 *  00A3F90F   . 33C6           XOR EAX,ESI
 *  00A3F911   . A9 0F000000    TEST EAX,0xF
 *  00A3F916   . 75 0E          JNZ SHORT play.00A3F926
 *  00A3F918   . 0FBA25 3060B90>BT DWORD PTR DS:[0xB96030],0x1
 *  00A3F920   . 0F82 DA040000  JB play.00A3FE00
 *  00A3F926   > 0FBA25 581FBC0>BT DWORD PTR DS:[0xBC1F58],0x0
 *  00A3F92E   . 0F83 A7010000  JNB play.00A3FADB
 *  00A3F934   . F7C7 03000000  TEST EDI,0x3
 *  00A3F93A   . 0F85 B8010000  JNZ play.00A3FAF8
 *  00A3F940   . F7C6 03000000  TEST ESI,0x3
 *  00A3F946   . 0F85 97010000  JNZ play.00A3FAE3
 *  00A3F94C   . 0FBAE7 02      BT EDI,0x2
 *  00A3F950   . 73 0D          JNB SHORT play.00A3F95F
 *  00A3F952   . 8B06           MOV EAX,DWORD PTR DS:[ESI]
 *  00A3F954   . 83E9 04        SUB ECX,0x4
 *  00A3F957   . 8D76 04        LEA ESI,DWORD PTR DS:[ESI+0x4]
 *  00A3F95A   . 8907           MOV DWORD PTR DS:[EDI],EAX
 *  00A3F95C   . 8D7F 04        LEA EDI,DWORD PTR DS:[EDI+0x4]
 *  00A3F95F   > 0FBAE7 03      BT EDI,0x3
 *  00A3F963   . 73 11          JNB SHORT play.00A3F976
 *  00A3F965   . F3:            PREFIX REP:                              ;  Superfluous prefix
 *  00A3F966   . 0F7E0E         MOVD DWORD PTR DS:[ESI],MM1
 *  00A3F969   . 83E9 08        SUB ECX,0x8
 *  00A3F96C   . 8D76 08        LEA ESI,DWORD PTR DS:[ESI+0x8]
 *  00A3F96F     66             DB 66                                    ;  CHAR 'f'
 *  00A3F970     0F             DB 0F
 *  00A3F971     D6             DB D6
 *  00A3F972     0F             DB 0F
 */
bool attach(ulong startAddress, ulong stopAddress)
{
  // There are two exact the same functions
  // The last function is hooked.
  // It seems the functions are generated by inlining
  const uint8_t bytes[] = {
    0x57,                 // 00a3f8d0   $ 57             push edi
    0x56,                 // 00a3f8d1   . 56             push esi
    0x8b,0x74,0x24, 0x10, // 00a3f8d2   . 8b7424 10      mov esi,dword ptr ss:[esp+0x10]
    0x8b,0x4c,0x24, 0x14, // 00a3f8d6   . 8b4c24 14      mov ecx,dword ptr ss:[esp+0x14]
    0x8b,0x7c,0x24, 0x0c  // 00a3f8da   . 8b7c24 0c      mov edi,dword ptr ss:[esp+0xc]
  };
  ulong lastCall = 0;
  auto fun = [&lastCall](ulong addr) -> bool {
    //if (lastCall)
    lastCall = addr;
    return true;
  };
  MemDbg::iterFindBytes(fun, bytes, sizeof(bytes), startAddress, stopAddress);
  return lastCall && winhook::hook_before(lastCall, Private::hookBefore);
}

/**
 *  Sample game: 塔の下のエクセルキトゥス体験版
 *  Executable description shows "AVGEngineV2"
 *  Source text in arg1, which is truncated
 *
 *  001D45DA     CC                  INT3
 *  001D45DB     CC                  INT3
 *  001D45DC     CC                  INT3
 *  001D45DD     CC                  INT3
 *  001D45DE     CC                  INT3
 *  001D45DF     CC                  INT3
 *  001D45E0  /$ 55                  PUSH EBP
 *  001D45E1  |. 8BEC                MOV EBP,ESP
 *  001D45E3  |. 8B55 08             MOV EDX,DWORD PTR SS:[EBP+0x8]
 *  001D45E6  |. 33C0                XOR EAX,EAX
 *  001D45E8  |. 56                  PUSH ESI
 *  001D45E9  |. 8BF1                MOV ESI,ECX
 *  001D45EB  |. C746 14 07000000    MOV DWORD PTR DS:[ESI+0x14],0x7
 *  001D45F2  |. C746 10 00000000    MOV DWORD PTR DS:[ESI+0x10],0x0
 *  001D45F9  |. 66:8906             MOV WORD PTR DS:[ESI],AX
 *  001D45FC  |. 66:3902             CMP WORD PTR DS:[EDX],AX   ; jichi: debug breakpoint stops here
 *  001D45FF  |. 75 12               JNZ SHORT play.001D4613
 *  001D4601  |. 33C9                XOR ECX,ECX
 *  001D4603  |. 51                  PUSH ECX
 *  001D4604  |. 52                  PUSH EDX
 *  001D4605  |. 8BCE                MOV ECX,ESI
 *  001D4607  |. E8 A4160000         CALL play.001D5CB0
 *  001D460C  |. 8BC6                MOV EAX,ESI
 *  001D460E  |. 5E                  POP ESI
 *  001D460F  |. 5D                  POP EBP
 *  001D4610  |. C2 0400             RETN 0x4
 *  001D4613  |> 8BCA                MOV ECX,EDX
 *  001D4615  |. 57                  PUSH EDI
 *  001D4616  |. 8D79 02             LEA EDI,DWORD PTR DS:[ECX+0x2]
 *  001D4619  |. 8DA424 00000000     LEA ESP,DWORD PTR SS:[ESP]
 *  001D4620  |> 66:8B01             MOV AX,WORD PTR DS:[ECX]
 *  001D4623  |. 83C1 02             ADD ECX,0x2
 *  001D4626  |. 66:85C0             TEST AX,AX
 *  001D4629  |.^75 F5               JNZ SHORT play.001D4620
 *  001D462B  |. 2BCF                SUB ECX,EDI
 *  001D462D  |. D1F9                SAR ECX,1
 *  001D462F  |. 5F                  POP EDI
 *  001D4630  |. 51                  PUSH ECX
 *  001D4631  |. 52                  PUSH EDX
 *  001D4632  |. 8BCE                MOV ECX,ESI
 *  001D4634  |. E8 77160000         CALL play.001D5CB0
 *  001D4639  |. 8BC6                MOV EAX,ESI
 *  001D463B  |. 5E                  POP ESI
 *  001D463C  |. 5D                  POP EBP
 *  001D463D  \. C2 0400             RETN 0x4
 *  001D4640  /. 55                  PUSH EBP
 *  001D4641  |. 8BEC                MOV EBP,ESP
 *  001D4643  |. 51                  PUSH ECX
 *  001D4644  |. 56                  PUSH ESI
 *  001D4645  |. 8B75 08             MOV ESI,DWORD PTR SS:[EBP+0x8]
 *  001D4648  |. 33C0                XOR EAX,EAX
 *  001D464A  |. 50                  PUSH EAX
 *  001D464B  |. 68 C89F4C00         PUSH play.004C9FC8
 *  001D4650  |. 8BCE                MOV ECX,ESI
 *  001D4652  |. C745 FC 00000000    MOV DWORD PTR SS:[EBP-0x4],0x0
 *  001D4659  |. C746 14 07000000    MOV DWORD PTR DS:[ESI+0x14],0x7
 *  001D4660  |. C746 10 00000000    MOV DWORD PTR DS:[ESI+0x10],0x0
 *  001D4667  |. 66:8906             MOV WORD PTR DS:[ESI],AX
 *  001D466A  |. E8 41160000         CALL play.001D5CB0
 *  001D466F  |. 8BC6                MOV EAX,ESI
 *  001D4671  |. 5E                  POP ESI
 *  001D4672  |. 8BE5                MOV ESP,EBP
 *  001D4674  |. 5D                  POP EBP
 *  001D4675  \. C2 0400             RETN 0x4
 *  001D4678     CC                  INT3
 *  001D4679     CC                  INT3
 *  001D467A     CC                  INT3
 *  001D467B     CC                  INT3
 *  001D467C     CC                  INT3
 *  001D467D     CC                  INT3
 *  001D467E     CC                  INT3
 *  001D467F     CC                  INT3
 */
bool attach(ulong startAddress, ulong stopAddress)
{
  const uint8_t bytes[] = {
    0x8b,0xf1,                              // 001d45e9  |. 8bf1                mov esi,ecx
    0xc7,0x46, 0x14, 0x07,0x00,0x00,0x00,   // 001d45eb  |. c746 14 07000000    mov dword ptr ds:[esi+0x14],0x7
    0xc7,0x46, 0x10, 0x00,0x00,0x00,0x00,   // 001d45f2  |. c746 10 00000000    mov dword ptr ds:[esi+0x10],0x0
    0x66,0x89,0x06,                         // 001d45f9  |. 66:8906             mov word ptr ds:[esi],ax
    0x66,0x39,0x02                          // 001d45fc  |. 66:3902             cmp word ptr ds:[edx],ax   ; jichi: debug breakpoint stops here
  };
  ulong addr = MemDbg::findBytes(bytes, sizeof(bytes), startAddress, stopAddress);
  if (!addr)
    return false;
  addr = MemDbg::findEnclosingAlignedFunction(addr);
  if (!addr)
    return false;
  return winhook::hook_before(addr, Private::hookBefore);
}

// FIXME: text get split to lines
// Need to find text containing "%r"
namespace ScenarioHook1 { // for old GXP1
namespace Private {
  /**
   *  Sample system text:
   *  cg/bg/ショップ前＿昼.png
   *
   *  Scenario caller:
   *  00187E65  |. 8B06           MOV EAX,DWORD PTR DS:[ESI]
   *  00187E67  |. FF90 A8020000  CALL DWORD PTR DS:[EAX+0x2A8]
   *  00187E6D  |. 8B45 08        MOV EAX,DWORD PTR SS:[EBP+0x8]
   *  00187E70  |. 8D8E A8010000  LEA ECX,DWORD PTR DS:[ESI+0x1A8]
   *  00187E76  |. 3BC8           CMP ECX,EAX
   *  00187E78  |. 74 0A          JE SHORT play.00187E84
   *  00187E7A  |. 6A FF          PUSH -0x1
   *  00187E7C  |. 6A 00          PUSH 0x0
   *  00187E7E  |. 50             PUSH EAX
   *  00187E7F  |. E8 CCDAFBFF    CALL play.00145950
   *  00187E84  |> 8A45 0C        MOV AL,BYTE PTR SS:[EBP+0xC]
   *  00187E87  |. 8B0D 882C4F00  MOV ECX,DWORD PTR DS:[0x4F2C88]
   *  00187E8D  |. 8886 03030000  MOV BYTE PTR DS:[ESI+0x303],AL
   *  00187E93  |. 8A45 10        MOV AL,BYTE PTR SS:[EBP+0x10]
   *  00187E96  |. 8886 05030000  MOV BYTE PTR DS:[ESI+0x305],AL
   *
   *  The history thread that is needed to be skipped to avoid retranslation
   *  0095391A   74 0A            JE SHORT 塔の下の.00953926
   *  0095391C   6A FF            PUSH -0x1
   *  0095391E   6A 00            PUSH 0x0
   *  00953920   50               PUSH EAX
   *  00953921   call
   *  00953926   A1 882CCB00      MOV EAX,DWORD PTR DS:[0xCB2C88]   ; jichi: retaddr
   *  0095392B   A8 01            TEST AL,0x1
   *  0095392D   75 28            JNZ SHORT 塔の下の.00953957
   */
  bool hookBefore(winhook::hook_stack *s)
  {
    static QString text_; // persistent storage, which makes this function not thread-safe
    auto text = (LPWSTR)s->stack[1]; // arg1
    auto size = s->stack[2]; // arg2
    if (!(size > 0 && Engine::isAddressWritable(text) && ::wcslen(text) == size
          && text[0] > 127 && text[size - 1] > 127))
      return true;

    auto retaddr = s->stack[0];
    auto reladdr = retaddr - moduleBaseAddress_;
    auto role = Engine::OtherRole;
    QString oldText = QString::fromWCharArray(text),
            newText = EngineController::instance()->dispatchTextW(oldText, role, reladdr);
    if (newText == oldText)
      return true;
    text_ = newText;
    s->stack[1] = (ulong)text_.utf16();
    s->stack[2] = text_.size();
    return true;
  }
} // namespace Private

/**
 *  Sample game: 塔の下のエクセルキトゥス体験版
 *  Executable description shows "AVGEngineV2"
 *
 *  Find one of caller of the push "invalid string position" instruction.
 *
 *  arg1: text
 *  arg2: size
 *
 *  00EC3DAE   CC               INT3
 *  00EC3DAF   CC               INT3
 *  00EC3DB0   53               PUSH EBX
 *  00EC3DB1   56               PUSH ESI
 *  00EC3DB2   8BF1             MOV ESI,ECX
 *  00EC3DB4   8B4C24 0C        MOV ECX,DWORD PTR SS:[ESP+0xC]
 *  00EC3DB8   57               PUSH EDI
 *  00EC3DB9   85C9             TEST ECX,ECX
 *  00EC3DBB   74 49            JE SHORT trial.00EC3E06
 *  00EC3DBD   8B7E 18          MOV EDI,DWORD PTR DS:[ESI+0x18]
 *  00EC3DC0   8D46 04          LEA EAX,DWORD PTR DS:[ESI+0x4]
 *  00EC3DC3   83FF 08          CMP EDI,0x8
 *  00EC3DC6   72 04            JB SHORT trial.00EC3DCC
 *  00EC3DC8   8B10             MOV EDX,DWORD PTR DS:[EAX]
 *  00EC3DCA   EB 02            JMP SHORT trial.00EC3DCE
 *  00EC3DCC   8BD0             MOV EDX,EAX
 *  00EC3DCE   3BCA             CMP ECX,EDX
 *  00EC3DD0   72 34            JB SHORT trial.00EC3E06
 *  00EC3DD2   83FF 08          CMP EDI,0x8
 *  00EC3DD5   72 04            JB SHORT trial.00EC3DDB
 *  00EC3DD7   8B10             MOV EDX,DWORD PTR DS:[EAX]
 *  00EC3DD9   EB 02            JMP SHORT trial.00EC3DDD
 *  00EC3DDB   8BD0             MOV EDX,EAX
 *  00EC3DDD   8B5E 14          MOV EBX,DWORD PTR DS:[ESI+0x14]
 *  00EC3DE0   8D145A           LEA EDX,DWORD PTR DS:[EDX+EBX*2]
 *  00EC3DE3   3BD1             CMP EDX,ECX
 *  00EC3DE5   76 1F            JBE SHORT trial.00EC3E06
 *  00EC3DE7   83FF 08          CMP EDI,0x8
 *  00EC3DEA   72 02            JB SHORT trial.00EC3DEE
 *  00EC3DEC   8B00             MOV EAX,DWORD PTR DS:[EAX]
 *  00EC3DEE   8B5424 14        MOV EDX,DWORD PTR SS:[ESP+0x14]
 *  00EC3DF2   2BC8             SUB ECX,EAX
 *  00EC3DF4   52               PUSH EDX
 *  00EC3DF5   D1F9             SAR ECX,1
 *  00EC3DF7   51               PUSH ECX
 *  00EC3DF8   56               PUSH ESI
 *  00EC3DF9   8BCE             MOV ECX,ESI
 *  00EC3DFB   E8 00FDFFFF      CALL trial.00EC3B00
 *  00EC3E00   5F               POP EDI
 *  00EC3E01   5E               POP ESI
 *  00EC3E02   5B               POP EBX
 *  00EC3E03   C2 0800          RETN 0x8
 *  00EC3E06   8B7C24 14        MOV EDI,DWORD PTR SS:[ESP+0x14]
 *  00EC3E0A   81FF FEFFFF7F    CMP EDI,0x7FFFFFFE
 *  00EC3E10   76 05            JBE SHORT trial.00EC3E17
 *  00EC3E12   E8 BFA01200      CALL trial.00FEDED6
 *  00EC3E17   8B46 18          MOV EAX,DWORD PTR DS:[ESI+0x18]
 *  00EC3E1A   3BC7             CMP EAX,EDI
 *  00EC3E1C   73 21            JNB SHORT trial.00EC3E3F
 *  00EC3E1E   8B46 14          MOV EAX,DWORD PTR DS:[ESI+0x14]
 *  00EC3E21   50               PUSH EAX
 *  00EC3E22   57               PUSH EDI
 *  00EC3E23   8BCE             MOV ECX,ESI
 *  00EC3E25   E8 36FEFFFF      CALL trial.00EC3C60
 *  00EC3E2A   85FF             TEST EDI,EDI
 *  00EC3E2C   76 67            JBE SHORT trial.00EC3E95
 *  00EC3E2E   8B4E 18          MOV ECX,DWORD PTR DS:[ESI+0x18]
 *  00EC3E31   55               PUSH EBP
 *  00EC3E32   8D6E 04          LEA EBP,DWORD PTR DS:[ESI+0x4]
 *  00EC3E35   83F9 08          CMP ECX,0x8
 *  00EC3E38   72 31            JB SHORT trial.00EC3E6B
 *  00EC3E3A   8B45 00          MOV EAX,DWORD PTR SS:[EBP]
 *  00EC3E3D   EB 2E            JMP SHORT trial.00EC3E6D
 *  00EC3E3F   85FF             TEST EDI,EDI
 *  00EC3E41  ^75 E9            JNZ SHORT trial.00EC3E2C
 *  00EC3E43   897E 14          MOV DWORD PTR DS:[ESI+0x14],EDI
 *  00EC3E46   83F8 08          CMP EAX,0x8
 *  00EC3E49   72 10            JB SHORT trial.00EC3E5B
 *  00EC3E4B   8B46 04          MOV EAX,DWORD PTR DS:[ESI+0x4]
 *  00EC3E4E   33C9             XOR ECX,ECX
 *  00EC3E50   5F               POP EDI
 *  00EC3E51   66:8908          MOV WORD PTR DS:[EAX],CX
 *  00EC3E54   8BC6             MOV EAX,ESI
 *  00EC3E56   5E               POP ESI
 *  00EC3E57   5B               POP EBX
 *  00EC3E58   C2 0800          RETN 0x8
 *  00EC3E5B   8D46 04          LEA EAX,DWORD PTR DS:[ESI+0x4]
 *  00EC3E5E   33C9             XOR ECX,ECX
 *  00EC3E60   5F               POP EDI
 *  00EC3E61   66:8908          MOV WORD PTR DS:[EAX],CX
 *  00EC3E64   8BC6             MOV EAX,ESI
 *  00EC3E66   5E               POP ESI
 *  00EC3E67   5B               POP EBX
 *  00EC3E68   C2 0800          RETN 0x8
 *  00EC3E6B   8BC5             MOV EAX,EBP
 *  00EC3E6D   8B5424 14        MOV EDX,DWORD PTR SS:[ESP+0x14]
 *  00EC3E71   8D1C3F           LEA EBX,DWORD PTR DS:[EDI+EDI]
 *  00EC3E74   53               PUSH EBX
 *  00EC3E75   52               PUSH EDX
 *  00EC3E76   03C9             ADD ECX,ECX
 *  00EC3E78   51               PUSH ECX
 *  00EC3E79   50               PUSH EAX
 *  00EC3E7A   E8 58A61200      CALL trial.00FEE4D7
 *  00EC3E7F   83C4 10          ADD ESP,0x10
 *  00EC3E82   837E 18 08       CMP DWORD PTR DS:[ESI+0x18],0x8
 *  00EC3E86   897E 14          MOV DWORD PTR DS:[ESI+0x14],EDI
 *  00EC3E89   72 03            JB SHORT trial.00EC3E8E
 *  00EC3E8B   8B6D 00          MOV EBP,DWORD PTR SS:[EBP]
 *  00EC3E8E   33D2             XOR EDX,EDX
 *  00EC3E90   66:89142B        MOV WORD PTR DS:[EBX+EBP],DX
 *  00EC3E94   5D               POP EBP
 *  00EC3E95   5F               POP EDI
 *  00EC3E96   8BC6             MOV EAX,ESI
 *  00EC3E98   5E               POP ESI
 *  00EC3E99   5B               POP EBX
 *  00EC3E9A   C2 0800          RETN 0x8
 *  00EC3E9D   CC               INT3
 *  00EC3E9E   CC               INT3
 *  00EC3E9F   CC               INT3
 *  00EC3EA0   56               PUSH ESI
 *  00EC3EA1   8B7424 08        MOV ESI,DWORD PTR SS:[ESP+0x8]
 *  00EC3EA5   8BC6             MOV EAX,ESI
 *  00EC3EA7   57               PUSH EDI
 *  00EC3EA8   8D78 02          LEA EDI,DWORD PTR DS:[EAX+0x2]
 *  00EC3EAB   EB 03            JMP SHORT trial.00EC3EB0
 *  00EC3EAD   8D49 00          LEA ECX,DWORD PTR DS:[ECX]
 *  00EC3EB0   66:8B10          MOV DX,WORD PTR DS:[EAX]
 *  00EC3EB3   83C0 02          ADD EAX,0x2
 *  00EC3EB6   66:85D2          TEST DX,DX
 *  00EC3EB9  ^75 F5            JNZ SHORT trial.00EC3EB0
 *  00EC3EBB   2BC7             SUB EAX,EDI
 *  00EC3EBD   D1F8             SAR EAX,1
 *  00EC3EBF   50               PUSH EAX
 *  00EC3EC0   56               PUSH ESI
 *  00EC3EC1   E8 EAFEFFFF      CALL trial.00EC3DB0
 *  00EC3EC6   5F               POP EDI
 *  00EC3EC7   5E               POP ESI
 *  00EC3EC8   C2 0400          RETN 0x4
 *  00EC3ECB   CC               INT3
 *  00EC3ECC   CC               INT3
 *  00EC3ECD   CC               INT3
 *  00EC3ECE   CC               INT3
 */
bool attach(ulong startAddress, ulong stopAddress)
{
  const uint8_t bytes[] = {
    0x8b,0x7c,0x24, 0x14,           // 00ec3e06   8b7c24 14        mov edi,dword ptr ss:[esp+0x14]
    0x81,0xff, 0xfe,0xff,0xff,0x7f, // 00ec3e0a   81ff feffff7f    cmp edi,0x7ffffffe
    0x76, 0x05                      // 00ec3e10   76 05            jbe short trial.00ec3e17
  };
  ulong addr = MemDbg::findBytes(bytes, sizeof(bytes), startAddress, stopAddress);
  if (!addr)
    return addr;
  addr = MemDbg::findEnclosingAlignedFunction(addr);
  if (!addr)
    return addr;
  return winhook::hook_before(addr, Private::hookBefore);
}
} // namespace ScenarioHook1

namespace ChoiceHook {
namespace Private {
  bool hookBefore(winhook::hook_stack *s)
  {
    static QString text_;
    auto text = (LPCWSTR)s->stack[3]; // arg3
    if (!text || !*text)
      return true;
    auto retaddr = s->stack[0];
    auto reladdr = retaddr - moduleBaseAddress_;
    enum { role = Engine::ChoiceRole };
    QString oldText = QString::fromWCharArray(text),
            newText = EngineController::instance()->dispatchTextW(oldText, role, reladdr);
    if (newText.isEmpty() ||  oldText == newText)
      return true;
    text_ = newText;
    s->stack[3] = (ulong)text_.utf16();
    return true;
  }
} // Private
/**
 *  Sample game: 塔の下のエクセルキトゥス体験版
 *  Executable description shows "AVGEngineV2"
 *
 *  This is the actual draw function. Text is looped.
 *  Function found by debugging the popup message (the second message is accessed):
 *
 *  % hexstr ゲームを終了しますか utf16
 *  b230fc30e0309230427d864e57307e3059304b30
 *
 *  002AFC58  |011D140E  RETURN to play.011D140E from play.01340FF0
 *  002AFC5C  |002AFD14
 *  002AFC60  |01537308  play.01537308
 *  002AFC64  |0A426B58	; jichi: text
 *  002AFC68  |002AFCC4
 *  002AFC6C  |FFFFFFFF
 *  002AFC70  |00000000
 *  002AFC74  |80000000
 *  002AFC78  |00000002
 *  002AFC7C  |0000007C
 *  002AFC80  |00000007
 *  002AFC84  |0A6A6640
 *  002AFC88  |01532D90  play.01532D90
 *  002AFC8C  |00000000
 *
 *  01340FEA     CC                INT3
 *  01340FEB     CC                INT3
 *  01340FEC     CC                INT3
 *  01340FED     CC                INT3
 *  01340FEE     CC                INT3
 *  01340FEF     CC                INT3
 *  01340FF0   $ 55                PUSH EBP
 *  01340FF1   . 8BEC              MOV EBP,ESP
 *  01340FF3   . 6A FF             PUSH -0x1
 *  01340FF5   . 68 00054001       PUSH play.01400500
 *  01340FFA   . 64:A1 00000000    MOV EAX,DWORD PTR FS:[0]
 *  01341000   . 50                PUSH EAX
 *  01341001   . 83EC 30           SUB ESP,0x30
 *  01341004   . 53                PUSH EBX
 *  01341005   . 56                PUSH ESI
 *  01341006   . 57                PUSH EDI
 *  01341007   . A1 10605001       MOV EAX,DWORD PTR DS:[0x1506010]
 *  0134100C   . 33C5              XOR EAX,EBP
 *  0134100E   . 50                PUSH EAX
 *  0134100F   . 8D45 F4           LEA EAX,DWORD PTR SS:[EBP-0xC]
 *  01341012   . 64:A3 00000000    MOV DWORD PTR FS:[0],EAX
 *  01341018   . 8B45 14           MOV EAX,DWORD PTR SS:[EBP+0x14]
 *  0134101B   . C745 C4 00000000  MOV DWORD PTR SS:[EBP-0x3C],0x0
 *  01341022   . C745 C8 00000000  MOV DWORD PTR SS:[EBP-0x38],0x0
 *  01341029   . C745 CC 00000000  MOV DWORD PTR SS:[EBP-0x34],0x0
 *  01341030   . 8B08              MOV ECX,DWORD PTR DS:[EAX]
 *  01341032   . 8B50 04           MOV EDX,DWORD PTR DS:[EAX+0x4]
 *  01341035   . 8B40 08           MOV EAX,DWORD PTR DS:[EAX+0x8]
 *  01341038   . 2BC1              SUB EAX,ECX
 *  0134103A   . 894D E8           MOV DWORD PTR SS:[EBP-0x18],ECX
 *  0134103D   . 8955 E4           MOV DWORD PTR SS:[EBP-0x1C],EDX
 *  01341040   . 8945 EC           MOV DWORD PTR SS:[EBP-0x14],EAX
 *  01341043   . 6A 04             PUSH 0x4
 *  01341045   . 8D4D C4           LEA ECX,DWORD PTR SS:[EBP-0x3C]
 *  01341048   . C745 FC 00000000  MOV DWORD PTR SS:[EBP-0x4],0x0
 *  0134104F   . E8 9CC7E7FF       CALL play.011BD7F0
 *  01341054   . C745 D0 00000000  MOV DWORD PTR SS:[EBP-0x30],0x0
 *  0134105B   . C745 D4 00000000  MOV DWORD PTR SS:[EBP-0x2C],0x0
 *  01341062   . C745 D8 00000000  MOV DWORD PTR SS:[EBP-0x28],0x0
 *  01341069   . 6A 04             PUSH 0x4                                 ; /Arg1 = 00000004
 *  0134106B   . 8D4D D0           LEA ECX,DWORD PTR SS:[EBP-0x30]          ; |
 *  0134106E   . C645 FC 01        MOV BYTE PTR SS:[EBP-0x4],0x1            ; |
 *  01341072   . E8 59B1E4FF       CALL play.0118C1D0                       ; \play.001DC1D0
 *  01341077   . 8B7D 10           MOV EDI,DWORD PTR SS:[EBP+0x10]
 *  0134107A   . 33C0              XOR EAX,EAX
 *  0134107C   . 33DB              XOR EBX,EBX
 *  0134107E   . 8945 F0           MOV DWORD PTR SS:[EBP-0x10],EAX
 *  01341081   . 66:3907           CMP WORD PTR DS:[EDI],AX	; jichi: here
 *  01341084   . 0F84 C3000000     JE play.0134114D
 *  0134108A   . BE 01000000       MOV ESI,0x1
 *  0134108F   . 8945 14           MOV DWORD PTR SS:[EBP+0x14],EAX
 *  01341092   . 8975 10           MOV DWORD PTR SS:[EBP+0x10],ESI
 *  01341095   > 8B4D D4           MOV ECX,DWORD PTR SS:[EBP-0x2C]
 *  01341098   . B8 ABAAAA2A       MOV EAX,0x2AAAAAAB
 *  0134109D   . 2B4D D0           SUB ECX,DWORD PTR SS:[EBP-0x30]
 *  013410A0   . F7E9              IMUL ECX
 *  013410A2   . C1FA 02           SAR EDX,0x2
 *  013410A5   . 8BC2              MOV EAX,EDX
 *  013410A7   . C1E8 1F           SHR EAX,0x1F
 *  013410AA   . 03C2              ADD EAX,EDX
 *  013410AC   . 3BC3              CMP EAX,EBX
 *  013410AE   . 77 09             JA SHORT play.013410B9
 *  013410B0   . 56                PUSH ESI                                 ; /Arg1
 *  013410B1   . 8D4D D0           LEA ECX,DWORD PTR SS:[EBP-0x30]          ; |
 *  013410B4   . E8 D703F1FF       CALL play.01251490                       ; \play.002A1490
 *  013410B9   > 8B45 C8           MOV EAX,DWORD PTR SS:[EBP-0x38]
 *  013410BC   . 2B45 C4           SUB EAX,DWORD PTR SS:[EBP-0x3C]
 *  013410BF   . C1F8 02           SAR EAX,0x2
 *  013410C2   . 3BC3              CMP EAX,EBX
 *  013410C4   . 77 14             JA SHORT play.013410DA
 *  013410C6   . 8D45 E0           LEA EAX,DWORD PTR SS:[EBP-0x20]
 *  013410C9   . C745 E0 00000000  MOV DWORD PTR SS:[EBP-0x20],0x0
 *  013410D0   . 50                PUSH EAX                                 ; /Arg2
 *  013410D1   . 56                PUSH ESI                                 ; |Arg1
 *  013410D2   . 8D4D C4           LEA ECX,DWORD PTR SS:[EBP-0x3C]          ; |
 *  013410D5   . E8 0610EAFF       CALL play.011E20E0                       ; \play.002320E0
 *  013410DA   > 0FB707            MOVZX EAX,WORD PTR DS:[EDI]
 *  013410DD   . 66:3B45 28        CMP AX,WORD PTR SS:[EBP+0x28]	; jichi: here
 *  013410D1   . 56                PUSH ESI                                 ; |Arg1
 *  013410D2   . 8D4D C4           LEA ECX,DWORD PTR SS:[EBP-0x3C]          ; |
 *  013410D5   . E8 0610EAFF       CALL play.011E20E0                       ; \play.002320E0
 *  013410DA   > 0FB707            MOVZX EAX,WORD PTR DS:[EDI]
 *  013410DD   . 66:3B45 28        CMP AX,WORD PTR SS:[EBP+0x28]
 *  013410E1   . 74 4F             JE SHORT play.01341132
 *  013410E3   . 8B75 0C           MOV ESI,DWORD PTR SS:[EBP+0xC]
 *  013410E6   . 50                PUSH EAX                                 ; /Arg1
 *  013410E7   . 8B76 18           MOV ESI,DWORD PTR DS:[ESI+0x18]          ; |
 *  013410EA   . E8 D1160000       CALL play.013427C0                       ; \play.003927C0
 *  013410EF   . 8B4D 14           MOV ECX,DWORD PTR SS:[EBP+0x14]
 *  013410F2   . 83C4 04           ADD ESP,0x4
 *  013410F5   . 034D D0           ADD ECX,DWORD PTR SS:[EBP-0x30]
 *  013410F8   . 0FAFC6            IMUL EAX,ESI
 *  013410FB   . 8B75 F0           MOV ESI,DWORD PTR SS:[EBP-0x10]
 *  013410FE   . 99                CDQ
 *  013410FF   . 2BC2              SUB EAX,EDX
 *  01341101   . D1F8              SAR EAX,1
 *  01341103   . 03F0              ADD ESI,EAX
 *  01341105   . 0FB707            MOVZX EAX,WORD PTR DS:[EDI]	; jichi
 *  01341108   . 50                PUSH EAX
 *  01341109   . 6A 01             PUSH 0x1
 *  0134110B   . 8975 F0           MOV DWORD PTR SS:[EBP-0x10],ESI
 *  0134110E   . E8 6D7EE8FF       CALL play.011C8F80
 *  01341113   . 8B55 C4           MOV EDX,DWORD PTR SS:[EBP-0x3C]
 *  01341116   . 8BCE              MOV ECX,ESI
 *  01341118   . 8B45 0C           MOV EAX,DWORD PTR SS:[EBP+0xC]
 *  ...
 */
bool attach(ulong startAddress, ulong stopAddress)
{
  const uint8_t bytes[] = {
    0x8b,0x45, 0x14,                        // 01341018   . 8b45 14           mov eax,dword ptr ss:[ebp+0x14]
    0xc7,0x45, 0xc4, 0x00,0x00,0x00,0x00    // 0134101b   . c745 c4 00000000  mov dword ptr ss:[ebp-0x3c],0x0
  };
  ulong addr = MemDbg::findBytes(bytes, sizeof(bytes), startAddress, stopAddress);
  if (!addr)
    return false;
  addr = MemDbg::findEnclosingAlignedFunction(addr);
  if (!addr)
    return false;
  return winhook::hook_before(addr, Private::hookBefore);
}
} // namespace ChoiceHook


#endif // 0
