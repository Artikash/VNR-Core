// age.cc
// 6/1/2014 jichi
// See: http://bbs.sumisora.org/read.php?tid=11044256
// See also ATCode: http://capita.tistory.com/m/post/255
#include "engine/model/age.h"
#include "engine/enginecontroller.h"
#include "engine/enginedef.h"
#include "engine/engineutil.h"
#include "hijack/hijackfuns.h"
#include "memdbg/memsearch.h"
#include "disasm/disasm.h"
#include "winasm/winasmdef.h"
#include "winasm/winasmutil.h"
#include "winhook/hookcode.h"
#include "winhook/hookfun.h"
#include "winhook/hookcall.h"
#include <qt_windows.h>

#define DEBUG "age"
#include "sakurakit/skdebug.h"

namespace { // unnamed
namespace ScenarioHook {
namespace Private {
  /**
   *  Sample game: 姫狩りダンジョンマイスター体験版
   *  Observations:
   *  - ecx: this, must be preserved
   *  - arg1: 0
   *  - arg2: LPCSTR
   *  - arg3: 0
   *  - arg4: 0
   *  - arg5: LPVOID  to unknown
   *  - return: TRUE(1) if succeed
   *
   * FIXME 6/1/2014: This will crash in Chinese locale
   */
  bool hookBefore(winhook::hook_stack *s)
  {
    static QByteArray data_; // persistent storage, which makes this function not thread-safe

    // All threads including character names are linked together
    enum { role = Engine::ScenarioRole, sig = Engine::ScenarioThreadSignature };

    LPCSTR text = (LPCSTR)s->stack[2]; // arg2

    data_ = EngineController::instance()->dispatchTextA(text, sig, role);
    s->stack[2] = (ulong)data_.constData(); // arg2
    return true;
  }

} // namespace Private

bool attach() // attach scenario
{
  ulong startAddress, stopAddress;
  if (!Engine::getCurrentMemoryRange(&startAddress, &stopAddress))
    return 0;

  ulong lastCaller = 0,
        lastCall = 0;
  auto fun = [&lastCaller, &lastCall](ulong caller, ulong call) -> bool {
    lastCaller = caller;
    lastCall = call;
    return true; // find last caller && call
  };
  MemDbg::iterCallerAddressAfterInt3(fun, (ulong)::GetTextExtentPoint32A, startAddress, stopAddress);
  if (!lastCaller)
    return false;
  if (!winhook::hook_before(lastCaller, Private::hookBefore))
    return false;
  winhook::replace_near_call(lastCall, (ulong)Hijack::newGetTextExtentPoint32A);
  return true;
}

} // namespace ScenarioHook

namespace OtherHook {
namespace Private {
  /**
   *  Find the caller of two GetGlyphOutlineA in the middle.
   *  Sample game: 神のラプソディ
   *
   *  0046E173   FF15 2C406100    CALL DWORD PTR DS:[0x61402C]             ; gdi32.GetGlyphOutlineA
   *  0046E1F8   FF15 2C406100    CALL DWORD PTR DS:[0x61402C]             ; gdi32.GetGlyphOutlineA
   *
   *  The two GetGlyphOutlineA are in the same function that are at the end of all GetGlyphOutlineA
   *
   *  0046DBCE   CC               INT3
   *  0046DBCF   CC               INT3
   *  0046DBD0   55               PUSH EBP
   *  0046DBD1   8BEC             MOV EBP,ESP
   *  0046DBD3   81EC 98000000    SUB ESP,0x98
   *  0046DBD9   A1 80D36600      MOV EAX,DWORD PTR DS:[0x66D380]
   *  0046DBDE   33C5             XOR EAX,EBP
   *  0046DBE0   8945 FC          MOV DWORD PTR SS:[EBP-0x4],EAX
   *  0046DBE3   53               PUSH EBX
   *  0046DBE4   56               PUSH ESI
   *  0046DBE5   8B75 1C          MOV ESI,DWORD PTR SS:[EBP+0x1C]
   *  0046DBE8   8BD9             MOV EBX,ECX
   *  0046DBEA   8B83 0C040000    MOV EAX,DWORD PTR DS:[EBX+0x40C]
   *  0046DBF0   57               PUSH EDI
   *  0046DBF1   33FF             XOR EDI,EDI
   *  0046DBF3   33D2             XOR EDX,EDX
   *  0046DBF5   33C9             XOR ECX,ECX
   *
   *  Regsiters:
   *  EAX 00000002
   *  ECX 01F54C9C
   *  EDX 00000002
   *  EBX 028BDD10  ; jichi: text is here in ebx
   *  ESP 0012F240
   *  EBP 0012F2A8
   *  ESI 01F54C9C
   *  EDI 00000092
   *  EIP 0046DBD0 .0046DBD0
   *
   *  Runtime stack:
   *  0012F240   00454A3A  RETURN to .00454A3A from .0046DBD0
   *  0012F244   00000092
   *  0012F248   00000002
   *  0012F24C   00000002
   *  0012F250   FFFFFFF5
   *  0012F254   00000013
   *  0012F258   028BDD10   ; jichi: text is here
   *  0012F25C   00000001
   *  0012F260   00000000
   *  0012F264   01F40020
   *  0012F268   00000000
   *  0012F26C   00000016
   *  0012F270   00000013
   */
  bool hookBefore(winhook::hook_stack *s)
  {
    static QByteArray data_; // persistent storage, which makes this function not thread-safe

    // All threads including character names are linked together
    enum { role = Engine::OtherRole, sig = Engine::OtherThreadSignature };

    LPCSTR text = (LPCSTR)s->stack[6]; // arg6

    data_ = EngineController::instance()->dispatchTextA(text, sig, role);
    s->stack[6] = (ulong)data_.constData(); // arg2
    return true;
  }

} // namespace Private

bool attach() // attach scenario
{
  ulong startAddress, stopAddress;
  if (!Engine::getCurrentMemoryRange(&startAddress, &stopAddress))
    return 0;
  ulong thisCaller = 0,
        thisCall = 0,
        prevCall = 0;
  auto fun = [&thisCaller, &thisCall, &prevCall](ulong caller, ulong call) -> bool {
    if (call - prevCall == 133) { // 0x0046e1f8 - 0x0046e173 = 133
      thisCaller = caller;
      thisCall = call;
      return false; // stop iteration
    }
    prevCall = call;
    return true; // continue iteration
  };
  MemDbg::iterCallerAddressAfterInt3(fun, (ulong)::GetGlyphOutlineA, startAddress, stopAddress);
  if (!thisCaller)
    return false;
  if (!winhook::hook_before(thisCaller, Private::hookBefore))
    return false;
  winhook::replace_near_call(thisCall, (ulong)Hijack::newGetGlyphOutlineA);
  winhook::replace_near_call(prevCall, (ulong)Hijack::newGetGlyphOutlineA);
  return true;
}
} // namespace OtherHook

namespace Patch
{

/**
 *  Disable annoying message box popups
 *  Sample game: 神のラプソディ
 *  Example popup images: http://capita.tistory.com/108
 *
 *  My logic:
 *  1. Find the address of the first sjis message.
 *  2. Find the address when it is used as parameter of push
 *  3. Find the second call after the push
 *  4. Find the function being called
 *  5. Hijack the called function with a dummy msgbox
 *
 *  Sample code region for messages:
 *
 *  - Search: データが壊れています
 *    SJIS: 8366815b835e82aa89f382ea82c482a282dc82b7
 *  Found two matches.
 *  The second match is right, which is after zeros
 *  006162F8  54 2E 42 49 4E 00 00 00 00 00 00 00 00 00 00 00  T.BIN...........
 *  00616308  83 66 81 5B 83 5E 82 AA 89 F3 82 EA 82 C4 82 A2  データが壊れてい
 *  00616318  82 DC 82 B7 81 44 0A 8D C4 83 43 83 93 83 58 83  ます．.再インス・
 *  00616328  67 81 5B 83 8B 82 B5 82 C4 82 AD 82 BE 82 B3 82  gールしてくださ・
 *  00616338  A2 81 44 0A 0A 00 00 00 45 78 69 74 0D 0A 00 00  ｢．.....Exit....
 *
 *  0061840B  5B 28 32 2D 31 29 0D 0A 00 8A D6 90 94 81 46 4C  [(2-1)...関数：L
 *  0061841B  6F 61 64 44 61 74 61 20 83 47 83 89 81 5B 81 46  oadData エラー：
 *  0061842B  83 66 81 5B 83 5E 82 AA 89 F3 82 EA 82 C4 82 A2  データが壊れてい
 *  0061843B  82 DC 82 B7 81 44 0D 0A 00 8A D6 90 94 81 46 4C  ます．...関数：L
 *  0061844B  6F 61 64 44 61 74 61 20 83 47 83 89 81 5B 81 46  oadData エラー：
 *  0061845B  43 52 43 83 47 83 89 81 5B 28 31 2D 32 29 0D 0A  CRCエラー(1-2)..
 *  0061846B  00 8A D6 90 94 81 46 4C 6F 61 64 44 61 74 61 20  .関数：LoadData
 *  0061847B  83 47 83 89 81 5B 81 46 43 52 43 83 47 83 89 81  エラー：CRCエラ・
 *  0061848B  5B 28 31 2D 31 29 0D 0A 00 00 00 00 00 8A D6 90  [(1-1).......関・
 *  0061849B  94 81 46 4C 6F 61 64 44 61 74 61 20 83 47 83 89  煤FLoadData エラ
 *
 *  - Search: プログラムを続行しますか？
 *    SJIS: 8376838d834f8389838082f091b18d7382b582dc82b782a98148
 *  0061599E  83 76 83 8D 83 4F 83 89 83 80 82 F0 91 B1 8D 73  プログラムを続行
 *  006159AE  82 B5 82 DC 82 B7 82 A9 81 48 0A 0A 92 86 8E 7E  しますか？..中止
 *  006159BE  81 46 83 76 83 8D 83 4F 83 89 83 80 82 F0 8F 49  ：プログラムを終
 *  006159CE  97 B9 82 B5 82 DC 82 B7 81 44 0A 8D C4 8E 8E 8D  了します．.再試・
 *  006159DE  73 81 46 82 E0 82 A4 88 EA 93 78 82 B1 82 CC 83  s：もう一度この・
 *  006159EE  52 83 7D 83 93 83 68 82 F0 8E C0 8D 73 82 B5 82  Rマンドを実行し・
 *  006159FE  DC 82 B7 81 44 0A 20 20 20 20 20 20 20 20 28 82  ﾜす．.        (・
 *
 *  00411DA9   8D7E 08          LEA EDI,DWORD PTR DS:[ESI+0x8]
 *  00411DAC   68 08636100      PUSH .00616308  ; jichi: msg pushed here
 *  00411DB1   68 00040000      PUSH 0x400
 *  00411DB6   57               PUSH EDI
 *  00411DB7   E8 945EFFFF      CALL .00407C50
 *  00411DBC   83C4 0C          ADD ESP,0xC
 *  00411DBF   6A 05            PUSH 0x5
 *  00411DC1   57               PUSH EDI
 *  00411DC2   8B96 64EA0500    MOV EDX,DWORD PTR DS:[ESI+0x5EA64]
 *  00411DC8   52               PUSH EDX
 *  00411DC9   8BCE             MOV ECX,ESI
 *  00411DCB   E8 2017FFFF      CALL .004034F0  ; jichi: popup done here
 *  00411DD0   8985 A4F8FFFF    MOV DWORD PTR SS:[EBP-0x75C],EAX
 *  00411DD6   83F8 01          CMP EAX,0x1
 *  00411DD9   75 1F            JNZ SHORT .00411DFA
 *  00411DDB   33C0             XOR EAX,EAX
 *  00411DDD   8985 F8F7FFFF    MOV DWORD PTR SS:[EBP-0x808],EAX
 *  00411DE3   8985 7CF8FFFF    MOV DWORD PTR SS:[EBP-0x784],EAX
 */
namespace Private {

  // My own msgbox function to hook
  int msgbox(const char *text, const char *caption, uint type)
  {
    Q_UNUSED(text);
    Q_UNUSED(caption);
    if (type == MB_RETRYCANCEL)
      return IDRETRY;
    return 0;
  }

  // Return the second call address between start and stop addresses
  ulong find_call(ulong start, ulong stop)
  {
    for (ulong addr = start, size = ::disasm((LPCVOID)start); size && addr < stop; addr += size) {
      if (*(BYTE *)addr == s1_call_)
        return addr;
      size = ::disasm((LPCVOID)addr);
    }
    return 0;
  };
} // namespace Private
bool removePopups()
{
  ulong startAddress, stopAddress;
  if (!Engine::getCurrentMemoryRange(&startAddress, &stopAddress))
    return false;

  // hexstr: データが壊れています．
  // Prepend 00 at the beginning
  const char *msg = "\x00\x83\x66\x81\x5b\x83\x5e\x82\xaa\x89\xf3\x82\xea\x82\xc4\x82\xa2\x82\xdc\x82\xb7\x81\x44";
  ulong addr = MemDbg::findBytes(msg, 1 + ::strlen(msg+1), startAddress, stopAddress);
  if (!addr)
    return false;
  addr++; // skip the leading zero

  addr = MemDbg::findPushAddress(addr, startAddress, stopAddress);
  if (!addr)
    return false;

  ulong limit = addr + 100;  // 0x00411DCB - 0x00411DAC = 31
  addr = Private::find_call(addr, limit);
  if (!addr)
    return false;
  addr += ::disasm((LPCVOID)addr); // skip the current call instruction
  addr = Private::find_call(addr, limit); // find second call
  if (!addr)
    return false;

  addr = winasm::get_jmp_absaddr(addr);
  return winhook::replace_fun(addr, (ulong)Private::msgbox);
}

} // namespace Patch

} // unnamed namespace

/** Public class */

bool ARCGameEngine::attach()
{
  if (!ScenarioHook::attach())
    return false;
  if (OtherHook::attach())
    DOUT("other text found");
  else
    DOUT("other text NOT FOUND");
  if (Patch::removePopups())
    DOUT("remove popups succeed");
  else
    DOUT("remove popups FAILED");
  return true;
}

QString ARCGameEngine::textFilter(const QString &text, int role)
{
  Q_UNUSED(role);
  QString ret = text;
  ret = ret.replace(L'\ufffd', L'\u2015');
  return ret;
}

// EOF

#if 0

/**
 *  Sample game: 姫狩りダンジョンマイスター体験版
 *
 *  Caller of the last GetTextExtentPoint32A: 0x451170
 *  0045116e   cc               int3
 *  0045116f   cc               int3
 *  00451170   6a ff            push -0x1 ; jichi: function starts
 *  00451172   68 c0e25500      push age.0055e2c0
 *  00451177   64:a1 00000000   mov eax,dword ptr fs:[0]
 *  0045117d   50               push eax
 *  0045117e   81ec a4000000    sub esp,0xa4
 *  00451184   a1 80485b00      mov eax,dword ptr ds:[0x5b4880]
 *  00451189   33c4             xor eax,esp
 *  0045118b   898424 a0000000  mov dword ptr ss:[esp+0xa0],eax
 *  00451192   53               push ebx
 *  00451193   55               push ebp
 *  00451194   56               push esi
 *  00451195   57               push edi
 *  00451196   a1 80485b00      mov eax,dword ptr ds:[0x5b4880]
 *  0045119b   33c4             xor eax,esp
 *  0045119d   50               push eax
 *  0045119e   8d8424 b8000000  lea eax,dword ptr ss:[esp+0xb8]
 *  004511a5   64:a3 00000000   mov dword ptr fs:[0],eax
 *  004511ab   8bbc24 cc000000  mov edi,dword ptr ss:[esp+0xcc]
 *  004511b2   8b8424 d0000000  mov eax,dword ptr ss:[esp+0xd0]
 *  004511b9   8bf1             mov esi,ecx
 *  004511bb   897c24 24        mov dword ptr ss:[esp+0x24],edi
 *  004511bf   894424 44        mov dword ptr ss:[esp+0x44],eax
 *  004511c3   c78424 b0000000 >mov dword ptr ss:[esp+0xb0],0xf
 *  004511ce   c78424 ac000000 >mov dword ptr ss:[esp+0xac],0x0
 *  004511d9   c68424 9c000000 >mov byte ptr ss:[esp+0x9c],0x0
 *  004511e1   ba 01000000      mov edx,0x1
 *  004511e6   899424 c0000000  mov dword ptr ss:[esp+0xc0],edx
 *  004511ed   803f 00          cmp byte ptr ds:[edi],0x0
 *  004511f0   75 1b            jnz short age.0045120d
 *  004511f2   c78424 c0000000 >mov dword ptr ss:[esp+0xc0],-0x1
 *  004511fd   8d4c24 6c        lea ecx,dword ptr ss:[esp+0x6c]
 *  00451201   e8 baacffff      call age.0044bec0
 *  00451206   33c0             xor eax,eax
 *  00451208   e9 38060000      jmp age.00451845
 *  0045120d   8b9c24 c8000000  mov ebx,dword ptr ss:[esp+0xc8]
 *  00451214   85db             test ebx,ebx
 *  00451216   75 06            jnz short age.0045121e
 *  00451218   8b9e c8040000    mov ebx,dword ptr ds:[esi+0x4c8]
 *  0045121e   8b8c9e 14040000  mov ecx,dword ptr ds:[esi+ebx*4+0x414]
 *  00451225   c64424 16 00     mov byte ptr ss:[esp+0x16],0x0
 *  0045122a   3951 70          cmp dword ptr ds:[ecx+0x70],edx
 *  0045122d   897c24 18        mov dword ptr ss:[esp+0x18],edi
 *  00451231   894424 20        mov dword ptr ss:[esp+0x20],eax
 *  00451235   897c24 1c        mov dword ptr ss:[esp+0x1c],edi
 *  00451239   75 2f            jnz short age.0045126a
 *  0045123b   8b8c24 d4000000  mov ecx,dword ptr ss:[esp+0xd4]
 *  00451242   51               push ecx
 *  00451243   50               push eax
 *  00451244   57               push edi
 *  00451245   53               push ebx
 *  00451246   8bce             mov ecx,esi
 *  00451248   e8 c3f8ffff      call age.00450b10
 *  0045124d   8bf0             mov esi,eax
 *  0045124f   c78424 c0000000 >mov dword ptr ss:[esp+0xc0],-0x1
 *  0045125a   8d4c24 6c        lea ecx,dword ptr ss:[esp+0x6c]
 *  0045125e   e8 5dacffff      call age.0044bec0
 *  00451263   8bc6             mov eax,esi
 *  00451265   e9 db050000      jmp age.00451845
 *  0045126a   8b41 30          mov eax,dword ptr ds:[ecx+0x30]
 *  0045126d   85c0             test eax,eax
 *  0045126f   74 16            je short age.00451287
 *  00451271   8b51 34          mov edx,dword ptr ds:[ecx+0x34]
 *  00451274   2bd0             sub edx,eax
 *  00451276   b8 67666666      mov eax,0x66666667
 *  0045127b   f7ea             imul edx
 *  0045127d   c1fa 03          sar edx,0x3
 *  00451280   8bc2             mov eax,edx
 *  00451282   c1e8 1f          shr eax,0x1f
 *  00451285   03c2             add eax,edx
 *  00451287   83c0 ff          add eax,-0x1
 *  0045128a   50               push eax
 *  0045128b   894424 2c        mov dword ptr ss:[esp+0x2c],eax
 *  0045128f   e8 7c90ffff      call age.0044a310
 *  00451294   8b10             mov edx,dword ptr ds:[eax]
 *  00451296   8b40 04          mov eax,dword ptr ds:[eax+0x4]
 *  00451299   894424 34        mov dword ptr ss:[esp+0x34],eax
 *  0045129d   8b849e 14040000  mov eax,dword ptr ds:[esi+ebx*4+0x414]
 *  004512a4   8b48 30          mov ecx,dword ptr ds:[eax+0x30]
 *  004512a7   83c0 2c          add eax,0x2c
 *  004512aa   85c9             test ecx,ecx
 *  004512ac   895424 30        mov dword ptr ss:[esp+0x30],edx
 *  004512b0   75 04            jnz short age.004512b6
 *  004512b2   33c0             xor eax,eax
 *  004512b4   eb 16            jmp short age.004512cc
 *  004512b6   8b50 08          mov edx,dword ptr ds:[eax+0x8]
 *  004512b9   2bd1             sub edx,ecx
 *  004512bb   b8 67666666      mov eax,0x66666667
 *  004512c0   f7ea             imul edx
 *  004512c2   c1fa 03          sar edx,0x3
 *  004512c5   8bc2             mov eax,edx
 *  004512c7   c1e8 1f          shr eax,0x1f
 *  004512ca   03c2             add eax,edx
 *  004512cc   83c0 ff          add eax,-0x1
 *  004512cf   8986 70050000    mov dword ptr ds:[esi+0x570],eax
 *  004512d5   803f 00          cmp byte ptr ds:[edi],0x0
 *  004512d8   0f84 10040000    je age.004516ee
 *  004512de   8bbc9e 14040000  mov edi,dword ptr ds:[esi+ebx*4+0x414]
 *  004512e5   8b6f 34          mov ebp,dword ptr ds:[edi+0x34]
 *  004512e8   83c7 2c          add edi,0x2c
 *  004512eb   396f 04          cmp dword ptr ds:[edi+0x4],ebp
 *  004512ee   76 05            jbe short age.004512f5
 *  004512f0   e8 9a190f00      call age.00542c8f
 *  004512f5   8d45 ec          lea eax,dword ptr ss:[ebp-0x14]
 *  004512f8   3b47 08          cmp eax,dword ptr ds:[edi+0x8]
 *  004512fb   896c24 54        mov dword ptr ss:[esp+0x54],ebp
 *  004512ff   77 05            ja short age.00451306
 *  00451301   3b47 04          cmp eax,dword ptr ds:[edi+0x4]
 *  00451304   73 05            jnb short age.0045130b
 *  00451306   e8 84190f00      call age.00542c8f
 *  0045130b   83c5 ec          add ebp,-0x14
 *  0045130e   3b6f 08          cmp ebp,dword ptr ds:[edi+0x8]
 *  00451311   72 05            jb short age.00451318
 *  00451313   e8 77190f00      call age.00542c8f
 *  00451318   8d7d 04          lea edi,dword ptr ss:[ebp+0x4]
 *  0045131b   8b6c24 18        mov ebp,dword ptr ss:[esp+0x18]
 *  0045131f   8a45 00          mov al,byte ptr ss:[ebp]
 *  00451322   3c 20            cmp al,0x20
 *  00451324   72 04            jb short age.0045132a
 *  00451326   3c 7f            cmp al,0x7f
 *  00451328   72 35            jb short age.0045135f
 *  0045132a   8ac8             mov cl,al
 *  0045132c   80c1 60          add cl,0x60
 *  0045132f   80f9 3f          cmp cl,0x3f
 *  00451332   76 2b            jbe short age.0045135f
 *  00451334   3c 80            cmp al,0x80
 *  00451336   72 27            jb short age.0045135f
 *  00451338   66:8b55 00       mov dx,word ptr ss:[ebp]
 *  0045133c   66:895424 14     mov word ptr ss:[esp+0x14],dx
 *  00451341   8a4424 14        mov al,byte ptr ss:[esp+0x14]
 *  00451345   3c 20            cmp al,0x20
 *  00451347   72 04            jb short age.0045134d
 *  00451349   3c 7f            cmp al,0x7f
 *  0045134b   72 16            jb short age.00451363
 *  0045134d   3c a0            cmp al,0xa0
 *  0045134f   72 04            jb short age.00451355
 *  00451351   3c e0            cmp al,0xe0
 *  00451353   72 0e            jb short age.00451363
 *  00451355   c74424 2c 020000>mov dword ptr ss:[esp+0x2c],0x2
 *  0045135d   eb 11            jmp short age.00451370
 *  0045135f   884424 14        mov byte ptr ss:[esp+0x14],al
 *  00451363   c64424 15 00     mov byte ptr ss:[esp+0x15],0x0
 *  00451368   c74424 2c 010000>mov dword ptr ss:[esp+0x2c],0x1
 *  00451370   8d4424 14        lea eax,dword ptr ss:[esp+0x14]
 *  00451374   8d50 01          lea edx,dword ptr ds:[eax+0x1]
 *  00451377   8a08             mov cl,byte ptr ds:[eax]
 *  00451379   83c0 01          add eax,0x1
 *  0045137c   84c9             test cl,cl
 *  0045137e  ^75 f7            jnz short age.00451377
 *  00451380   2bc2             sub eax,edx
 *  00451382   8d4c24 48        lea ecx,dword ptr ss:[esp+0x48]
 *  00451386   51               push ecx
 *  00451387   50               push eax
 *  00451388   8b86 50040000    mov eax,dword ptr ds:[esi+0x450]
 *  0045138e   8d5424 1c        lea edx,dword ptr ss:[esp+0x1c]
 *  00451392   52               push edx
 *  00451393   50               push eax
 *  00451394   ff15 34205600    call dword ptr ds:[0x562034]             ; gdi32.gettextextentpoint32a
 *  0045139a   8b4f 04          mov ecx,dword ptr ds:[edi+0x4]
 *  0045139d   034c24 4c        add ecx,dword ptr ss:[esp+0x4c]
 *  004513a1   8b17             mov edx,dword ptr ds:[edi]
 *  004513a3   035424 48        add edx,dword ptr ss:[esp+0x48]
 *  004513a7   51               push ecx
 *  004513a8   8b8c9e 14040000  mov ecx,dword ptr ds:[esi+ebx*4+0x414]
 *  004513af   52               push edx
 *  004513b0   e8 bb760000      call age.00458a70
 *  004513b5   a8 01            test al,0x1
 *  004513b7   0f84 28020000    je age.004515e5
 *  004513bd   83be 78050000 01 cmp dword ptr ds:[esi+0x578],0x1
 *  004513c4   0f85 23020000    jnz age.004515ed
 *  004513ca   0fb64424 14      movzx eax,byte ptr ss:[esp+0x14]
 *  004513cf   8a5424 15        mov dl,byte ptr ss:[esp+0x15]
 *  004513d3   8da8 7fffffff    lea ebp,dword ptr ds:[eax-0x81]
 *  004513d9   8bc5             mov eax,ebp
 *  004513db   85c0             test eax,eax
 *  004513dd   75 08            jnz short age.004513e7
 *  004513df   0fb6c2           movzx eax,dl
 *  004513e2   83e8 41          sub eax,0x41
 *  004513e5   74 0e            je short age.004513f5
 *  004513e7   85c0             test eax,eax
 *  004513e9   b9 01000000      mov ecx,0x1
 *  004513ee   7f 07            jg short age.004513f7
 *  004513f0   83c9 ff          or ecx,0xffffffff
 *  004513f3   eb 02            jmp short age.004513f7
 *  004513f5   33c9             xor ecx,ecx
 *  004513f7   85c9             test ecx,ecx
 *  004513f9   0f84 ee010000    je age.004515ed
 *  004513ff   8bc5             mov eax,ebp
 *  00451401   85c0             test eax,eax
 *  00451403   75 08            jnz short age.0045140d
 *  00451405   0fb6c2           movzx eax,dl
 *  00451408   83e8 42          sub eax,0x42
 *  0045140b   74 0e            je short age.0045141b
 *  0045140d   85c0             test eax,eax
 *  0045140f   b9 01000000      mov ecx,0x1
 *  00451414   7f 07            jg short age.0045141d
 *  00451416   83c9 ff          or ecx,0xffffffff
 *  00451419   eb 02            jmp short age.0045141d
 *  0045141b   33c9             xor ecx,ecx
 *  0045141d   85c9             test ecx,ecx
 *  0045141f   0f84 c8010000    je age.004515ed
 *  00451425   8bc5             mov eax,ebp
 *  00451427   85c0             test eax,eax
 *  00451429   75 08            jnz short age.00451433
 *  0045142b   0fb6c2           movzx eax,dl
 *  0045142e   83e8 76          sub eax,0x76
 *  00451431   74 0e            je short age.00451441
 *  00451433   85c0             test eax,eax
 *  00451435   b9 01000000      mov ecx,0x1
 *  0045143a   7f 07            jg short age.00451443
 *  0045143c   83c9 ff          or ecx,0xffffffff
 *  0045143f   eb 02            jmp short age.00451443
 *  00451441   33c9             xor ecx,ecx
 *  00451443   85c9             test ecx,ecx
 *  00451445   0f84 a2010000    je age.004515ed
 *  0045144b   8b7c24 18        mov edi,dword ptr ss:[esp+0x18]
 *  0045144f   2b7c24 1c        sub edi,dword ptr ss:[esp+0x1c]
 *  00451453   897c24 40        mov dword ptr ss:[esp+0x40],edi
 *  00451457   83c7 01          add edi,0x1
 *  0045145a   57               push edi
 *  0045145b   e8 4e0a0f00      call age.00541eae
 *  00451460   8b4c24 20        mov ecx,dword ptr ss:[esp+0x20]
 *  00451464   8be8             mov ebp,eax
 *
 *  Sample game: 天秤のLaDEA体験版
 *  at 0x468fa0
 *  Caller of the -3 GetTextExtentPoint32A: 0x46921f
 *  Caller of the -2 GetTextExtentPoint32A: 0x46965f
 *  Caller of the -1 GetTextExtentPoint32A: 0x4699b1
 *
 */
static DWORD searchEushully(DWORD startAddress, DWORD stopAddress)
{
  //const BYTE bytes[] = { // size = 14
  //  0x01,0x53, 0x58,                // 0153 58          add dword ptr ds:[ebx+58],edx
  //  0x8b,0x95, 0x34,0xfd,0xff,0xff, // 8b95 34fdffff    mov edx,dword ptr ss:[ebp-2cc]
  //  0x8b,0x43, 0x58,                // 8b43 58          mov eax,dword ptr ds:[ebx+58]
  //  0x3b,0xd7                       // 3bd7             cmp edx,edi ; hook here
  //};
  //enum { cur_ins_size = 2 };
  //enum { hook_offset = sizeof(bytes) - cur_ins_size }; // = 14 - 2  = 12, current inst is the last one
  const BYTE bytes1[] = {
    0x3b,0xd7, // 013baf32  |. 3bd7       |cmp edx,edi ; jichi: ITH hook here, char saved in edi
    0x75,0x4b  // 013baf34  |. 75 4b      |jnz short siglusen.013baf81
  };
  //enum { hook_offset = 0 };
  //DWORD range1 = min(stopAddress - startAddress, Engine::MaximumMemoryRange);
  DWORD addr = MemDbg::findBytes(bytes1, sizeof(bytes1), startAddress, stopAddress);
  if (!addr)
    //ConsoleOutput("vnreng:Siglus2: pattern not found");
    return 0;

  const BYTE bytes2[] = {
    0x55,      // 013bac70  /$ 55       push ebp ; jichi: function starts
    0x8b,0xec, // 013bac71  |. 8bec     mov ebp,esp
    0x6a,0xff  // 013bac73  |. 6a ff    push -0x1
  };
  enum { range = 0x300 }; // 0x013baf32  -0x013bac70 = 706 = 0x2c2
  return MemDbg::findBytes(bytes2, sizeof(bytes2), addr - range, addr); // search backward
  //if (!addr)
  //  //ConsoleOutput("vnreng:Siglus2: pattern not found");
  //  return 0;
  //return addr;
}

#endif // 0
