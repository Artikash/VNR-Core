// cs2.cc
// 6/21/2015 jichi
#include "engine/model/cs2.h"
#include "engine/enginecontroller.h"
#include "engine/enginedef.h"
#include "engine/enginehash.h"
#include "engine/engineutil.h"
#include "hijack/hijackmanager.h"
#include "winhook/hookcode.h"
#include "memdbg/memsearch.h"
#include <qt_windows.h>

#define DEBUG "CatSystem2"
#include "sakurakit/skdebug.h"

namespace { // unnamed
namespace ScenarioHook {
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
    data_ = EngineController::instance()->dispatchTextA(text, sig, role);
    s->stack[1] = (ulong)data_.constData();
    //::strcpy(text, data.constData());
    return true;
  }
} // namespace Private

/**
 *  Sample game: カミカゼ
 *  text in arg1
 *
 *  Function found by debugging GetGlyphOutlineA, then traversing function stack
 *  005B516E   CC               INT3
 *  005B516F   CC               INT3
 *  005B5170   51               PUSH ECX
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

} // namespace ScenarioHook
} // unnamed namespace

bool CatSystem2Engine::attach()
{
  if (!ScenarioHook::attach())
    return false;
  HijackManager::instance()->attachFunction((ulong)::GetGlyphOutlineA);
  //HijackManager::instance()->attachFunction((ulong)::GetTextExtentPoint32A);
  return true;
}

// EOF
