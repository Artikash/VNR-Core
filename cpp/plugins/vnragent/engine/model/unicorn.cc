// unicorn.cc
// 6/22/2015 jichi
#include "engine/model/unicorn.h"
#include "engine/enginecontroller.h"
#include "engine/enginedef.h"
#include "engine/enginehash.h"
#include "engine/engineutil.h"
#include "hijack/hijackmanager.h"
#include "util/textutil.h"
#include "winhook/hookcode.h"
#include "memdbg/memsearch.h"
#include <qt_windows.h>
#include <unordered_set>

#define DEBUG "unicorn"
#include "sakurakit/skdebug.h"

namespace { // unnamed
namespace ScenarioHook {
namespace Private {
  enum { ShortTextCapacity = 0x10 }; // 15, maximum number of characters for short text including '\0'
  struct HookArgument
  {
    DWORD type;       // 0x0, equal zero for scenario
    union {
      LPCSTR  longText;       // 0x4, editable though
      char shortText[ShortTextCapacity];
    };
    int size,     // 0x14, 0xf for short text
        capacity; // 0x18

    // Skip single character
    bool isValid() const { return size > 2 && size < Engine::MaxTextSize && size <= capacity; }
    int role() const { return type == 0 ? Engine::ScenarioRole : Engine::OtherRole; }
    LPCSTR text() const { return capacity <= ShortTextCapacity ? shortText : longText; }
  };

  HookArgument *arg_,
               argValue_;
  bool hookBefore(ulong addr, winhook::hook_stack *s)
  {
    static QByteArray data_;
    static std::unordered_set<LPCSTR> texts_; // addresses of the translated texts
    auto arg = (HookArgument *)s->stack[0]; // arg1
    if (!arg || !arg->isValid())
      return true;
    auto text = arg->text();
    if (!text || !*text || Util::allAscii(text) || texts_.find(text) != texts_.cend())
      return true;
    auto role = arg->role();
    auto split = addr + 5; // use retaddr as split, 5 = jumpsize
    auto sig = Engine::hashThreadSignature(role, split);
    sig = addr; // use return address as split
    QByteArray oldData(text, arg->size),
               newData = EngineController::instance()->dispatchTextA(oldData, sig, role);
    if (newData == oldData)
      return true;
    return true;
    //texts_.insert(text);
    data_ = newData;
    arg_ = arg;
    argValue_ = *arg;
    arg->longText = data_.constData();
    arg->size = data_.size();
    arg->capacity = arg->size;
    return true;
  }
  bool hookAfter(winhook::hook_stack *)
  {
    if (arg_) {
      *arg_ = argValue_;
      arg_ = nullptr;
    }
    return true;
  }
} // namespace Private

/**
 *  Sample game:  戦極姫6
 *
 *  Function found by debugging GetGlyphOutlineA, then traversing stack addresses.
 *
 *  Runtime stack of arg1 when the first dword value is zero
 *  0355F9E4  00 00 00 00 E8 8E 26 03 00 00 00 00 00 00 00 00
 *  0355F9F4  00 00 00 00 14 00 00 00 1F 00 00 00 00 00 00 00
 *  0355FA04  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
 *  0355FA14  00 00 00 00 0F 00 00 00 00 00 00 00 BE BE BE FF
 *  0355FA24  FF FF FF FF 00 00 00 00 00 00 00 00 00 00 00 00
 *  0355FA34  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
 *  0355FA44  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
 *  0355FA54  00                                               .
 *
 *  00403ACE   CC               INT3
 *  00403ACF   CC               INT3
 *  00403AD0   53               PUSH EBX ; jichi: text in arg1 + 0x4
 *  00403AD1   8B5C24 08        MOV EBX,DWORD PTR SS:[ESP+0x8] ; jichi: ebx is the arg1
 *  00403AD5   55               PUSH EBP
 *  00403AD6   8B6C24 10        MOV EBP,DWORD PTR SS:[ESP+0x10]
 *  00403ADA   56               PUSH ESI
 *  00403ADB   57               PUSH EDI
 *  00403ADC   8BF1             MOV ESI,ECX
 *  00403ADE   396B 14          CMP DWORD PTR DS:[EBX+0x14],EBP
 *  00403AE1   73 05            JNB SHORT .00403AE8
 *  00403AE3   E8 1A511500      CALL .00558C02
 *  00403AE8   8B7B 14          MOV EDI,DWORD PTR DS:[EBX+0x14]
 *  00403AEB   8B4424 1C        MOV EAX,DWORD PTR SS:[ESP+0x1C]
 *  00403AEF   2BFD             SUB EDI,EBP
 *  00403AF1   3BC7             CMP EAX,EDI
 *  00403AF3   73 02            JNB SHORT .00403AF7
 *  00403AF5   8BF8             MOV EDI,EAX
 *  00403AF7   3BF3             CMP ESI,EBX
 *  00403AF9   75 1F            JNZ SHORT .00403B1A
 *  00403AFB   6A FF            PUSH -0x1
 *  00403AFD   03FD             ADD EDI,EBP
 *  00403AFF   57               PUSH EDI
 *  00403B00   8BCE             MOV ECX,ESI
 *  00403B02   E8 A9FDFFFF      CALL .004038B0
 *  00403B07   55               PUSH EBP
 *  00403B08   6A 00            PUSH 0x0
 *  00403B0A   8BCE             MOV ECX,ESI
 *  00403B0C   E8 9FFDFFFF      CALL .004038B0
 *  00403B11   5F               POP EDI
 *  00403B12   8BC6             MOV EAX,ESI
 *  00403B14   5E               POP ESI
 *  00403B15   5D               POP EBP
 *  00403B16   5B               POP EBX
 *  00403B17   C2 0C00          RETN 0xC
 *  00403B1A   83FF FE          CMP EDI,-0x2
 *  00403B1D   76 05            JBE SHORT .00403B24
 *  00403B1F   E8 A6501500      CALL .00558BCA
 *  00403B24   8B46 18          MOV EAX,DWORD PTR DS:[ESI+0x18]
 *  00403B27   3BC7             CMP EAX,EDI
 *  00403B29   73 1B            JNB SHORT .00403B46
 *  00403B2B   8B46 14          MOV EAX,DWORD PTR DS:[ESI+0x14]
 *  00403B2E   50               PUSH EAX
 *  00403B2F   57               PUSH EDI
 *  00403B30   8BCE             MOV ECX,ESI
 *  00403B32   E8 49FEFFFF      CALL .00403980
 *  00403B37   85FF             TEST EDI,EDI
 *  00403B39   76 66            JBE SHORT .00403BA1
 *  00403B3B   837B 18 10       CMP DWORD PTR DS:[EBX+0x18],0x10
 *  00403B3F   72 2F            JB SHORT .00403B70
 *  00403B41   8B53 04          MOV EDX,DWORD PTR DS:[EBX+0x4] ; jichi: for long text
 *  00403B44   EB 2D            JMP SHORT .00403B73
 *  00403B46   85FF             TEST EDI,EDI
 *  00403B48  ^75 EF            JNZ SHORT .00403B39
 *  00403B4A   897E 14          MOV DWORD PTR DS:[ESI+0x14],EDI
 *  00403B4D   83F8 10          CMP EAX,0x10
 *  00403B50   72 0F            JB SHORT .00403B61
 *  00403B52   8B46 04          MOV EAX,DWORD PTR DS:[ESI+0x4]
 *  00403B55   5F               POP EDI
 *  00403B56   C600 00          MOV BYTE PTR DS:[EAX],0x0
 *  00403B59   8BC6             MOV EAX,ESI
 *  00403B5B   5E               POP ESI
 *  00403B5C   5D               POP EBP
 *  00403B5D   5B               POP EBX
 *  00403B5E   C2 0C00          RETN 0xC
 *  00403B61   8D46 04          LEA EAX,DWORD PTR DS:[ESI+0x4]
 *  00403B64   5F               POP EDI
 *  00403B65   C600 00          MOV BYTE PTR DS:[EAX],0x0
 *  00403B68   8BC6             MOV EAX,ESI
 *  00403B6A   5E               POP ESI
 *  00403B6B   5D               POP EBP
 *  00403B6C   5B               POP EBX
 *  00403B6D   C2 0C00          RETN 0xC
 *  00403B70   8D53 04          LEA EDX,DWORD PTR DS:[EBX+0x4]	; jichi: source text for short text
 *  00403B73   8B4E 18          MOV ECX,DWORD PTR DS:[ESI+0x18] ; jichi: size?
 *  00403B76   8D5E 04          LEA EBX,DWORD PTR DS:[ESI+0x4]	; jichi: target
 *  00403B79   83F9 10          CMP ECX,0x10
 *  00403B7C   72 04            JB SHORT .00403B82
 *  00403B7E   8B03             MOV EAX,DWORD PTR DS:[EBX]
 *  00403B80   EB 02            JMP SHORT .00403B84
 *  00403B82   8BC3             MOV EAX,EBX
 *  00403B84   57               PUSH EDI ; jichi: arg4, source size?
 *  00403B85   03D5             ADD EDX,EBP
 *  00403B87   52               PUSH EDX ; jichi: arg3, source text
 *  00403B88   51               PUSH ECX ; jichi: arg2, size?
 *  00403B89   50               PUSH EAX ; jichi: arg1, target text address to modify
 *  00403B8A   E8 F5C11200      CALL .0052FD84	; jichi: text here:, which copied from arg2 to arg1
 *  00403B8F   83C4 10          ADD ESP,0x10
 *  00403B92   837E 18 10       CMP DWORD PTR DS:[ESI+0x18],0x10
 *  00403B96   897E 14          MOV DWORD PTR DS:[ESI+0x14],EDI
 *  00403B99   72 02            JB SHORT .00403B9D
 *  00403B9B   8B1B             MOV EBX,DWORD PTR DS:[EBX]
 *  00403B9D   C6043B 00        MOV BYTE PTR DS:[EBX+EDI],0x0
 *  00403BA1   5F               POP EDI
 *  00403BA2   8BC6             MOV EAX,ESI
 *  00403BA4   5E               POP ESI
 *  00403BA5   5D               POP EBP
 *  00403BA6   5B               POP EBX
 *  00403BA7   C2 0C00          RETN 0xC
 *  00403BAA   CC               INT3
 *  00403BAB   CC               INT3
 *  00403BAC   CC               INT3
 *  00403BAD   CC               INT3
 *  00403BAE   CC               INT3
 */
bool attach()
{
  ulong startAddress, stopAddress;
  if (!Engine::getProcessMemoryRange(&startAddress, &stopAddress))
    return false;

  const quint8 bytes[] = {
    0x85,0xff,              // 00403b37   85ff             test edi,edi
    0x76, 0x66,             // 00403b39   76 66            jbe short .00403ba1
    0x83,0x7b, 0x18, 0x10,  // 00403b3b   837b 18 10       cmp dword ptr ds:[ebx+0x18],0x10
    0x72, 0x2f,             // 00403b3f   72 2f            jb short .00403b70
    0x8b,0x53, 0x04,        // 00403b41   8b53 04          mov edx,dword ptr ds:[ebx+0x4] ; jichi: for long text
    0xeb, 0x2d,             // 00403b44   eb 2d            jmp short .00403b73
    0x85,0xff               // 00403b46   85ff             test edi,edi
  };
  ulong addr = MemDbg::findBytes(bytes, sizeof(bytes), startAddress, stopAddress);
  if (!addr)
    return false;
  addr = MemDbg::findEnclosingAlignedFunction(addr);
  if (!addr)
    return false;
  //return winhook::hook_before(addr, Private::hookBefore);
  int count = 0;
  auto fun = [&count](ulong call) -> bool {
    auto hookBefore = [call](winhook::hook_stack *s) {
      return Private::hookBefore(call, s);
    };
    count += winhook::hook_both(call, hookBefore, Private::hookAfter);
    return true; // find all calls
  };
  MemDbg::iterNearCallAddress(fun, addr, startAddress, stopAddress);
  DOUT("call number =" << count);
  return count;
}

} // namespace ScenarioHook
} // unnamed namespace

bool UnicornEngine::attach()
{
  if (!ScenarioHook::attach())
    return false;
  HijackManager::instance()->attachFunction((ulong)::GetGlyphOutlineA);
  return true;
}

// EOF
