// horkeye.cc
// 6/24/2015 jichi
#include "engine/model/horkeye.h"
#include "engine/enginecontroller.h"
#include "engine/enginedef.h"
#include "engine/enginehash.h"
#include "engine/engineutil.h"
#include "hijack/hijackmanager.h"
#include "memdbg/memsearch.h"
#include "winhook/hookcode.h"
#include <qt_windows.h>

#define DEBUG "horkeye"
#include "sakurakit/skdebug.h"

namespace { // unnamed
namespace ScenarioHook {
namespace Private {

  bool hookBefore(winhook::hook_stack *s)
  {
    enum { role = Engine::ScenarioRole, sig = Engine::ScenarioThreadSignature };
    static QByteArray data_;
    auto text = (LPCSTR)s->stack[1]; // text in arg1
    QByteArray oldData = text,
               newData = EngineController::instance()->dispatchTextA(oldData, sig, role);
    if (newData.isEmpty() || newData == oldData)
      return true;
    data_ = newData;
    //s->stack[1] = (ulong)data_.constData();
    return true;
  }

} // namespace Private

/** 10/20/2014 jichi: HorkEye, http://horkeye.com
 *  Sample games:
 *  - 結城友奈は勇者である 体験版
 *  - 姉小路直子と銀色の死神
 *
 *  No GDI functions are used by this game.
 *
 *  Debug method:
 *  There are two matched texts.
 *  The one having fixed address is used to insert hw breakpoints.
 *
 *  I found are two functions addressing the address, both of which seems to be good.
 *  The first one is used:
 *
 *  013cda60   8d4c24 1c        lea ecx,dword ptr ss:[esp+0x1c]
 *  013cda64   51               push ecx
 *  013cda65   68 48a8c201      push .01c2a848                                     ; ascii "if"
 *  013cda6a   e8 d1291600      call .01530440
 *  013cda6f   83c4 0c          add esp,0xc
 *  013cda72   6a 01            push 0x1
 *  013cda74   83ec 1c          sub esp,0x1c
 *  013cda77   8bcc             mov ecx,esp
 *  013cda79   896424 30        mov dword ptr ss:[esp+0x30],esp
 *  013cda7d   6a 10            push 0x10
 *  013cda7f   c741 14 0f000000 mov dword ptr ds:[ecx+0x14],0xf
 *  013cda86   c741 10 00000000 mov dword ptr ds:[ecx+0x10],0x0
 *  013cda8d   68 80125601      push .01561280
 *  013cda92   c601 00          mov byte ptr ds:[ecx],0x0
 *  013cda95   e8 5681ffff      call .013c5bf0
 *  013cda9a   e8 717a0900      call .01465510
 *  013cda9f   83c4 20          add esp,0x20
 *  013cdaa2   b8 01000000      mov eax,0x1
 *  013cdaa7   8b8c24 b8000000  mov ecx,dword ptr ss:[esp+0xb8]
 *  013cdaae   5f               pop edi
 *  013cdaaf   5e               pop esi
 *  013cdab0   5d               pop ebp
 *  013cdab1   5b               pop ebx
 *  013cdab2   33cc             xor ecx,esp
 *  013cdab4   e8 c7361600      call .01531180
 *  013cdab9   81c4 ac000000    add esp,0xac
 *  013cdabf   c3               retn
 *  013cdac0   83ec 40          sub esp,0x40    ; jichi: text here in arg1
 *  013cdac3   a1 24805d01      mov eax,dword ptr ds:[0x15d8024]
 *  013cdac8   8b15 c4709901    mov edx,dword ptr ds:[0x19970c4]
 *  013cdace   8d0c00           lea ecx,dword ptr ds:[eax+eax]
 *  013cdad1   a1 9c506b01      mov eax,dword ptr ds:[0x16b509c]
 *  013cdad6   0305 18805d01    add eax,dword ptr ds:[0x15d8018]
 *  013cdadc   53               push ebx
 *  013cdadd   8b5c24 48        mov ebx,dword ptr ss:[esp+0x48]
 *  013cdae1   55               push ebp
 *  013cdae2   8b6c24 50        mov ebp,dword ptr ss:[esp+0x50]
 *  013cdae6   894c24 34        mov dword ptr ss:[esp+0x34],ecx
 *  013cdaea   8b0d 20805d01    mov ecx,dword ptr ds:[0x15d8020]
 *  013cdaf0   894424 18        mov dword ptr ss:[esp+0x18],eax
 *  013cdaf4   a1 1c805d01      mov eax,dword ptr ds:[0x15d801c]
 *  013cdaf9   03c8             add ecx,eax
 *  013cdafb   56               push esi
 *  013cdafc   33f6             xor esi,esi
 *  013cdafe   d1f8             sar eax,1
 *  013cdb00   45               inc ebp
 *  013cdb01   896c24 24        mov dword ptr ss:[esp+0x24],ebp
 *  013cdb05   897424 0c        mov dword ptr ss:[esp+0xc],esi
 *  013cdb09   894c24 18        mov dword ptr ss:[esp+0x18],ecx
 *  013cdb0d   8a0c1a           mov cl,byte ptr ds:[edx+ebx]        jichi: here
 *  013cdb10   894424 30        mov dword ptr ss:[esp+0x30],eax
 *  013cdb14   8a441a 01        mov al,byte ptr ds:[edx+ebx+0x1]
 *  013cdb18   57               push edi
 *  013cdb19   897424 14        mov dword ptr ss:[esp+0x14],esi
 *  013cdb1d   3935 c8709901    cmp dword ptr ds:[0x19970c8],esi
 *
 *  The hooked place is only accessed once.
 *  013cdb0d   8a0c1a           mov cl,byte ptr ds:[edx+ebx]        jichi: here
 *  ebx is the text to be base address.
 *  edx is the offset to skip character name.
 *
 *  023B66A0  81 79 89 C4 EA A3 2C 53 30 30 35 5F 42 5F 30 30  【夏凜,S005_B_00
 *  023B66B0  30 32 81 7A 81 75 83 6F 81 5B 83 65 83 62 83 4E  02】「バーテック
 *  023B66C0  83 58 82 CD 82 B1 82 C1 82 BF 82 CC 93 73 8D 87  スはこっちの都合
 *  023B66D0  82 C8 82 C7 82 A8 8D 5C 82 A2 82 C8 82 B5 81 63  などお構いなし…
 *
 *  There are garbage in character name.
 *
 *  1/15/2015
 *  Alternative hook that might not need a text filter:
 *  http://www.hongfire.com/forum/showthread.php/36807-AGTH-text-extraction-tool-for-games-translation/page753
 *  /HA-4@552B5:姉小路直子と銀色の死神.exe
 *  If this hook no longer works, try that one instead.
 */
bool attach() // attach scenario
{
  ulong startAddress, stopAddress;
  if (!Engine::getProcessMemoryRange(&startAddress, &stopAddress))
    return false;

  const quint8 bytes[] = {
    0x89,0x6c,0x24, 0x24,   // 013cdb01   896c24 24        mov dword ptr ss:[esp+0x24],ebp
    0x89,0x74,0x24, 0x0c,   // 013cdb05   897424 0c        mov dword ptr ss:[esp+0xc],esi
    0x89,0x4c,0x24, 0x18,   // 013cdb09   894c24 18        mov dword ptr ss:[esp+0x18],ecx
    0x8a,0x0c,0x1a          // 013cdb0d   8a0c1a           mov cl,byte ptr ds:[edx+ebx]        jichi: text is here
  };
  ulong addr = MemDbg::findBytes(bytes, sizeof(bytes), startAddress, stopAddress);
  if (!addr)
    return false;

  // 013cdabf   c3               retn
  // 013cdac0   83ec 40          sub esp,0x40    ; jichi: text here in arg1
  //
  // 0x013cdb0d-0x013cdabf = 7
  for (DWORD i = addr; i > addr - 0x100; i--)
    if (*(DWORD *)i == 0x40ec83c3)
      return winhook::hook_before(i, Private::hookBefore);
  return false;
}

} // namespace ScenarioHook

} // unnamed namespace

/** Public class */

bool HorkEyeEngine::attach()
{
  if (!ScenarioHook::attach())
    return false;
  HijackManager::instance()->attachFunction((ulong)::MultiByteToWideChar);
  return true;
}

// EOF
