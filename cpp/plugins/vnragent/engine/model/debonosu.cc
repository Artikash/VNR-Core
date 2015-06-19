// debonosu.cc
// 6/18/2015 jichi
#include "engine/model/debonosu.h"
#include "engine/enginecontroller.h"
#include "engine/enginedef.h"
#include "engine/engineutil.h"
#include "hijack/hijackmanager.h"
#include "winhook/hookcode.h"
#include "memdbg/memsearch.h"
#include "ntinspect/ntinspect.h"
#include "winasm/winasmdef.h"
#include <qt_windows.h>

#define DEBUG "debonosu"
#include "sakurakit/skdebug.h"

namespace { // unnamed
namespace ScenarioHook {
namespace Private {

  enum Version {
    NewVersion, // text in ecx
    OldVersion  // text in arg1
  } version_;

  bool hookBefore(winhook::hook_stack *s)
  {
    static QByteArray data_;
    LPCSTR *lpText;
    if (version_ == OldVersion) // add esp, $  old Debonosu game
      lpText = (LPCSTR *)&s->stack[1]; // text in arg1
    else // new Debonosu game
      lpText = (LPCSTR *)&s->ecx; // text in ecx

    auto text = *lpText;
    if (!text || !*text)
      return true;

    enum { role = Engine::ScenarioRole, sig = Engine::ScenarioThreadSignature };
    data_ = EngineController::instance()->dispatchTextA(text, sig, role);
    *lpText = data_.constData();
    //::strcpy(text, data.constData());
    return true;
  }

  // Get the address of the scenario function
  ulong findFunction(ulong startAddress, ulong stopAddress)
  {
    ulong funaddr = NtInspect::getModuleExportFunctionA("kernel32.dll", "lstrcatA");
    if (!funaddr)
      return false;
    funaddr = NtInspect::getProcessImportAddress(funaddr);
    if (!funaddr)
      return false;

    DWORD callinst = 0x15ff | (funaddr << 16); // jichi 10/20/2014: far call dword ptr ds
    funaddr >>= 16;
    for (DWORD i = startAddress; i < stopAddress - 4; i++)
      if (*(DWORD *)i == callinst &&
          *(WORD *)(i + 4) == funaddr && // call dword ptr lstrcatA
          *(BYTE *)(i - 5) == 0x68) { // push $
        DWORD push = *(DWORD *)(i - 4); // the global value being pushed
        for (DWORD j = i + 6, k = j + 0x10; j < k; j++)
          if (*(BYTE *)j == 0xb8 && *(DWORD *)(j + 1) == push)
            if (DWORD addr = MemDbg::findEnclosingAlignedFunction(i, 0x200))
              return addr;
      }
    return 0;
  }

} // namespace Private

/**
 *  Sample game: 神楽花莚譚
 *
 *  base address: 0x0da0000
 *
 *  00DBD9BE   CC               INT3
 *  00DBD9BF   CC               INT3
 *  00DBD9C0   53               PUSH EBX
 *  00DBD9C1   56               PUSH ESI
 *  00DBD9C2   57               PUSH EDI
 *  00DBD9C3   8BF9             MOV EDI,ECX
 *  00DBD9C5   C605 1830F200 00 MOV BYTE PTR DS:[0xF23018],0x0
 *  00DBD9CC   85FF             TEST EDI,EDI
 *  00DBD9CE   0F84 B8000000    JE .00DBDA8C
 *  00DBD9D4   8B1D 18A4E700    MOV EBX,DWORD PTR DS:[0xE7A418]          ; user32.CharNextA
 *  00DBD9DA   8D9B 00000000    LEA EBX,DWORD PTR DS:[EBX]
 *  00DBD9E0   6A 23            PUSH 0x23
 *  00DBD9E2   57               PUSH EDI
 *  00DBD9E3   E8 DE360600      CALL .00E210C6
 *  00DBD9E8   8BF0             MOV ESI,EAX
 *  00DBD9EA   83C4 08          ADD ESP,0x8
 *  00DBD9ED   85F6             TEST ESI,ESI
 *  00DBD9EF   0F84 87000000    JE .00DBDA7C
 *  00DBD9F5   2BC7             SUB EAX,EDI
 *  00DBD9F7   50               PUSH EAX
 *  00DBD9F8   57               PUSH EDI
 *  00DBD9F9   68 1830F200      PUSH .00F23018
 *  00DBD9FE   E8 2D6D0600      CALL .00E24730
 *  00DBDA03   83C4 0C          ADD ESP,0xC
 *  00DBDA06   BF 00000000      MOV EDI,0x0
 *  00DBDA0B   46               INC ESI
 *  00DBDA0C   74 48            JE SHORT .00DBDA56
 *  00DBDA0E   85F6             TEST ESI,ESI
 *  00DBDA10   74 1F            JE SHORT .00DBDA31
 *  00DBDA12   56               PUSH ESI
 *  00DBDA13   FFD3             CALL EBX
 *  00DBDA15   8BC8             MOV ECX,EAX
 *  00DBDA17   33C0             XOR EAX,EAX
 *  00DBDA19   85C9             TEST ECX,ECX
 *  00DBDA1B   75 08            JNZ SHORT .00DBDA25
 *  00DBDA1D   3846 01          CMP BYTE PTR DS:[ESI+0x1],AL
 *  00DBDA20   0F95C0           SETNE AL
 *  00DBDA23   EB 08            JMP SHORT .00DBDA2D
 *  00DBDA25   2BCE             SUB ECX,ESI
 *  00DBDA27   83F9 01          CMP ECX,0x1
 *  00DBDA2A   0F9FC0           SETG AL
 *  00DBDA2D   85C0             TEST EAX,EAX
 *  00DBDA2F   75 25            JNZ SHORT .00DBDA56
 *  00DBDA31   0FBE06           MOVSX EAX,BYTE PTR DS:[ESI]
 *  00DBDA34   50               PUSH EAX
 *  00DBDA35   E8 54450600      CALL .00E21F8E
 *  00DBDA3A   83C4 04          ADD ESP,0x4
 *  00DBDA3D   85C0             TEST EAX,EAX
 *  00DBDA3F   74 15            JE SHORT .00DBDA56
 *  00DBDA41   0FBE06           MOVSX EAX,BYTE PTR DS:[ESI]
 *  00DBDA44   8D3CBF           LEA EDI,DWORD PTR DS:[EDI+EDI*4]
 *  00DBDA47   56               PUSH ESI
 *  00DBDA48   8D7F E8          LEA EDI,DWORD PTR DS:[EDI-0x18]
 *  00DBDA4B   8D3C78           LEA EDI,DWORD PTR DS:[EAX+EDI*2]
 *  00DBDA4E   FFD3             CALL EBX
 *  00DBDA50   8BF0             MOV ESI,EAX
 *  00DBDA52   85F6             TEST ESI,ESI
 *  00DBDA54  ^75 BC            JNZ SHORT .00DBDA12
 *  00DBDA56   8BCF             MOV ECX,EDI
 *  00DBDA58   E8 53FDFFFF      CALL .00DBD7B0
 *  00DBDA5D   50               PUSH EAX
 *  00DBDA5E   68 1830F200      PUSH .00F23018 ; jichi: pattern starts
 *  00DBDA63   FF15 98A2E700    CALL DWORD PTR DS:[0xE7A298]             ; kernel32.lstrcatA
 *  00DBDA69   8BFE             MOV EDI,ESI
 *  00DBDA6B   85F6             TEST ESI,ESI
 *  00DBDA6D  ^0F85 6DFFFFFF    JNZ .00DBD9E0
 *  00DBDA73   B8 1830F200      MOV EAX,.00F23018 ; jichi: pattern starts
 *  00DBDA78   5F               POP EDI
 *  00DBDA79   5E               POP ESI
 *  00DBDA7A   5B               POP EBX
 *  00DBDA7B   C3               RETN
 *  00DBDA7C   85FF             TEST EDI,EDI
 *  00DBDA7E   74 0C            JE SHORT .00DBDA8C
 *  00DBDA80   57               PUSH EDI
 *  00DBDA81   68 1830F200      PUSH .00F23018
 *  00DBDA86   FF15 98A2E700    CALL DWORD PTR DS:[0xE7A298]             ; kernel32.lstrcatA
 *  00DBDA8C   5F               POP EDI
 *  00DBDA8D   5E               POP ESI
 *  00DBDA8E   B8 1830F200      MOV EAX,.00F23018
 *  00DBDA93   5B               POP EBX
 *  00DBDA94   C3               RETN
 *  00DBDA95   CC               INT3
 *  00DBDA96   CC               INT3
 *  00DBDA97   CC               INT3
 *  00DBDA98   CC               INT3
 *  00DBDA99   CC               INT3
 */
bool attach()
{
  ulong startAddress, stopAddress;
  if (!Engine::getProcessMemoryRange(&startAddress, &stopAddress))
    return false;
  ulong addr = Private::findFunction(startAddress, stopAddress);
  if (!addr)
    return false;

  ulong call = MemDbg::findNearCallAddress(addr, startAddress, stopAddress);
  if (!call)
    return false;
  Private::version_ = *(WORD *)(call + 4) == 0xc483 ? Private::OldVersion : Private::NewVersion;

  return winhook::hook_before(addr, Private::hookBefore);

  //int count = 0;
  //auto fun = [&count](ulong addr) -> bool {
  //  if (winhook::hook_before(addr, Private::hookBefore)) {
  //    // 0xc483 = add esp, $  old Debonosu game
  //    Private::version_ = *(WORD *)(addr + 4) == 0xc483 ? Private::OldVersion : Private::NewVersion;
  //    count++;
  //  }
  //  return true; // replace all functions
  //};
  //MemDbg::iterNearCallAddress(fun, addr, startAddress, stopAddress);
  //DOUT("call number =" << count);
  //return count;
}

} // namespace ScenarioHook
} // unnamed namespace

bool DebonosuEngine::attach()
{
  if (!ScenarioHook::attach())
    return false;
  //HijackManager::instance()->attachFunction((ulong)::GetTextExtentPoint32A);
  //HijackManager::instance()->attachFunction((ulong)::CharNextA);
  HijackManager::instance()->attachFunction((ulong)::MultiByteToWideChar);
  return true;
}

// EOF
