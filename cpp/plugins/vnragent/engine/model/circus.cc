// circus.cc
// 6/5/2014 jichi
#include "engine/model/circus.h"
#include "engine/enginecontroller.h"
#include "engine/enginedef.h"
#include "engine/enginehash.h"
#include "engine/engineutil.h"
#include "hijack/hijackfuns.h"
//#include "hijack/hijackmanager.h"
#include "memdbg/memsearch.h"
#include "winhook/hookcode.h"
#include <qt_windows.h>

#define DEBUG "circus"
#include "sakurakit/skdebug.h"

/** Private data */

namespace { // unnamed

namespace ScenarioHook {

namespace Private {

  // Skip leading tags such as @K and @c5
  LPCSTR ltrim(LPCSTR s)
  {
    if (*s == '@')
      while ((signed char)*++s > 0);
    return s;
  }

  DWORD nameReturnAddress_,
        scenarioReturnAddress_;

  /**
   *  Sample game: DC3, function: 0x4201d0
   *
   *  IDA: sub_4201D0      proc near
   *  - arg_0 = dword ptr  4
   *  - arg_4 = dword ptr  8
   *
   *  Observations:
   *  - arg1: LPVOID, pointed to unknown object
   *  - arg2: LPCSTR, the actual text
   *
   *  Example runtime stack:
   *  0012F15C   0040C208  RETURN to .0040C208 from .00420460
   *  0012F160   0012F7CC ; jichi: unknown stck
   *  0012F164   0012F174 ; jichi: text
   *  0012F168   0012F6CC
   *  0012F16C   0012F7CC
   *  0012F170   0012F7CC
   */
  bool hookBefore(winhook::hook_stack *s)
  {
    static QByteArray data_; // persistent storage, which makes this function not thread-safe
    auto text = (LPCSTR)s->stack[2], // arg2
         trimmedText = ltrim(text);
    if (!*trimmedText)
      return true;
    auto retaddr = s->stack[0]; // retaddr
    auto role = retaddr == scenarioReturnAddress_ ? Engine::ScenarioRole :
                retaddr == nameReturnAddress_ ? Engine::NameRole :
                Engine::OtherRole;
                //s->ebx? Engine::OtherRole : // other threads ebx is not zero
                //// 004201e4  |. 83c1 02        |add ecx,0x2
                //// 004201e7  |. eb 04          |jmp short dc3.004201ed
                //*(BYTE *)(retaddr + 3) == 0xe9 // old name
                //? Engine::NameRole : // retaddr+3 is jmp
                //Engine::ScenarioRole;
    auto split = retaddr;
    auto sig = Engine::hashThreadSignature(role, split);
    QByteArray oldData = trimmedText,
               newData = EngineController::instance()->dispatchTextA(trimmedText, sig, role);
    if (oldData == newData)
      return true;
    if (trimmedText != text)
      newData.prepend(text, trimmedText - text);
    data_ = newData;
    s->stack[2] = (DWORD)data_.constData(); // reset arg2
    return true;
  }

  // Alternatively, using the following pattern bytes also works:
  //
  // 3c24750583c102eb0488024241
  //
  // 004201e0  |> 3c 24          /cmp al,0x24
  // 004201e2  |. 75 05          |jnz short dc3.004201e9
  // 004201e4  |. 83c1 02        |add ecx,0x2
  // 004201e7  |. eb 04          |jmp short dc3.004201ed
  // 004201e9  |> 8802           |mov byte ptr ds:[edx],al
  // 004201eb  |. 42             |inc edx
  // 004201ec  |. 41             |inc ecx
  ulong findFunctionAddress(ulong startAddress, ulong stopAddress) // find the function to hook
  {
    //return 0x4201d0; // DC3 function address
    for (ulong i = startAddress + 0x1000; i < stopAddress -4; i++)
      // *  004201e0  |> 3c 24          /cmp al,0x24
      // *  004201e2  |. 75 05          |jnz short dc3.004201e9
      if ((*(ulong *)i & 0xffffff) == 0x75243c) { // cmp al, 24; je
        enum { range = 0x80 }; // the range is small, since it is a small function
        if (ulong addr = MemDbg::findEnclosingAlignedFunction(i, range))
          return addr;
      }
    return 0;
  }

} // namespace Private

/**
 *  jichi 6/5/2014: Sample function from DC3 at 0x4201d0
 *
 *  Sample game: DC3PP
 *  0042CE1E   68 E0F0B700      PUSH .00B7F0E0
 *  0042CE23   A3 0C824800      MOV DWORD PTR DS:[0x48820C],EAX
 *  0042CE28   E8 A352FFFF      CALL .004220D0  ; jichi: name thread
 *  0042CE2D   C705 08024D00 01>MOV DWORD PTR DS:[0x4D0208],0x1
 *  0042CE37   EB 52            JMP SHORT .0042CE8B
 *  0042CE39   392D 08024D00    CMP DWORD PTR DS:[0x4D0208],EBP
 *  0042CE3F   74 08            JE SHORT .0042CE49
 *  0042CE41   392D 205BB900    CMP DWORD PTR DS:[0xB95B20],EBP
 *  0042CE47   74 07            JE SHORT .0042CE50
 *  0042CE49   C605 E0F0B700 00 MOV BYTE PTR DS:[0xB7F0E0],0x0
 *  0042CE50   8D5424 40        LEA EDX,DWORD PTR SS:[ESP+0x40]
 *  0042CE54   52               PUSH EDX
 *  0042CE55   68 30B5BA00      PUSH .00BAB530
 *  0042CE5A   892D 08024D00    MOV DWORD PTR DS:[0x4D0208],EBP
 *  0042CE60   E8 6B52FFFF      CALL .004220D0  ; jichi: scenario thread
 *  0042CE65   C705 A0814800 FF>MOV DWORD PTR DS:[0x4881A0],-0x1
 *  0042CE6F   892D 2C824800    MOV DWORD PTR DS:[0x48822C],EBP
 *
 *  Sample game: 水夏弐律
 *
 *  004201ce     cc             int3
 *  004201cf     cc             int3
 *  004201d0  /$ 8b4c24 08      mov ecx,dword ptr ss:[esp+0x8]
 *  004201d4  |. 8a01           mov al,byte ptr ds:[ecx]
 *  004201d6  |. 84c0           test al,al
 *  004201d8  |. 74 1c          je short dc3.004201f6
 *  004201da  |. 8b5424 04      mov edx,dword ptr ss:[esp+0x4]
 *  004201de  |. 8bff           mov edi,edi
 *  004201e0  |> 3c 24          /cmp al,0x24
 *  004201e2  |. 75 05          |jnz short dc3.004201e9
 *  004201e4  |. 83c1 02        |add ecx,0x2
 *  004201e7  |. eb 04          |jmp short dc3.004201ed
 *  004201e9  |> 8802           |mov byte ptr ds:[edx],al
 *  004201eb  |. 42             |inc edx
 *  004201ec  |. 41             |inc ecx
 *  004201ed  |> 8a01           |mov al,byte ptr ds:[ecx]
 *  004201ef  |. 84c0           |test al,al
 *  004201f1  |.^75 ed          \jnz short dc3.004201e0
 *  004201f3  |. 8802           mov byte ptr ds:[edx],al
 *  004201f5  |. c3             retn
 *  004201f6  |> 8b4424 04      mov eax,dword ptr ss:[esp+0x4]
 *  004201fa  |. c600 00        mov byte ptr ds:[eax],0x0
 *  004201fd  \. c3             retn
 *
 *  Sample registers:
 *  EAX 0012F998
 *  ECX 000000DB
 *  EDX 00000059
 *  EBX 00000000    ; ebx is zero for name/scenario thread
 *  ESP 0012F96C
 *  EBP 00000003
 *  ESI 00000025
 *  EDI 000000DB
 *  EIP 022C0000
 *
 *  EAX 0012F174
 *  ECX 0012F7CC
 *  EDX FDFBF80C
 *  EBX 0012F6CC
 *  ESP 0012F15C
 *  EBP 0012F5CC
 *  ESI 800000DB
 *  EDI 00000001
 *  EIP 00420460 .00420460
 *
 *  EAX 0012F174
 *  ECX 0012F7CC
 *  EDX FDFBF7DF
 *  EBX 0012F6CC
 *  ESP 0012F15C
 *  EBP 0012F5CC
 *  ESI 00000108
 *  EDI 00000001
 *  EIP 00420460 .00420460
 *
 *  0042DC5D   52               PUSH EDX
 *  0042DC5E   68 E038AC00      PUSH .00AC38E0                           ; ASCII "Ami"
 *  0042DC63   E8 F827FFFF      CALL .00420460  ; jichi: name thread
 *  0042DC68   83C4 08          ADD ESP,0x8
 *  0042DC6B   E9 48000000      JMP .0042DCB8
 *  0042DC70   83FD 58          CMP EBP,0x58
 *  0042DC73   74 07            JE SHORT .0042DC7C
 *  0042DC75   C605 E038AC00 00 MOV BYTE PTR DS:[0xAC38E0],0x0
 *  0042DC7C   8D4424 20        LEA EAX,DWORD PTR SS:[ESP+0x20]
 *  0042DC80   50               PUSH EAX
 *  0042DC81   68 0808AF00      PUSH .00AF0808
 *  0042DC86   E8 D527FFFF      CALL .00420460 ; jichi: scenario thread
 *  0042DC8B   83C4 08          ADD ESP,0x8
 *  0042DC8E   33C0             XOR EAX,EAX
 *  0042DC90   C705 D0DF4700 FF>MOV DWORD PTR DS:[0x47DFD0],-0x1
 *  0042DC9A   A3 0CE04700      MOV DWORD PTR DS:[0x47E00C],EAX
 *  0042DC9F   A3 940EB200      MOV DWORD PTR DS:[0xB20E94],EAX
 *  0042DCA4   A3 2C65AC00      MOV DWORD PTR DS:[0xAC652C],EAX
 *  0042DCA9   C705 50F9AC00 59>MOV DWORD PTR DS:[0xACF950],0x59
 *  0042DCB3   A3 3C70AE00      MOV DWORD PTR DS:[0xAE703C],EAX
 */
bool attach()
{
  ulong startAddress, stopAddress;
  if (!Engine::getProcessMemoryRange(&startAddress, &stopAddress))
    return 0;
  ulong addr = Private::findFunctionAddress(startAddress, stopAddress);
  if (!addr)
    return false;
  // Find the nearest two callers (distance within 100)
  ulong lastCall = 0;
  auto fun = [&lastCall](ulong call) -> bool {
    if (call - lastCall < 100) {
      Private::scenarioReturnAddress_ = call + 5;
      Private::nameReturnAddress_ = lastCall + 5;
      DOUT("found scenario and name calls");
      return false; // found target
    }
    lastCall = call;
    return true; // replace all functions
  };
  MemDbg::iterNearCallAddress(fun, addr, startAddress, stopAddress);
  return winhook::hook_before(addr, Private::hookBefore);
}

} // namespace ScenarioHook

} // unnamed namespace

/** Public class */

bool CircusEngine::attach()
{
  if (!ScenarioHook::attach())
    return false;
  //HijackManager::instance()->attachFunction((ulong)::GetGlyphOutlineA); // for special symbol and ruby texts
  return true;
}

/**
 *  Get rid of ruby. Examples:
 *  ｛くらき／蔵木｝
 *  ｛・・・・／いいから｝この私に、紅茶を淹れなさい」
 */
QString CircusEngine::textFilter(const QString &text, int role)
{
  const wchar_t
    w_open = 0xff5b    /* ｛ */
    , w_close = 0xff5d /* ｝ */
    , w_split = 0xff0f /* ／ */
  ;
  if (role != Engine::ScenarioRole || !text.contains(w_open))
    return text;
  QString ret = text;
  //ret.remove("@K");
  for (int pos = ret.indexOf(w_open); pos != -1; pos = ret.indexOf(w_open, pos)) {
    int split_pos = ret.indexOf(w_split, pos);
    if (split_pos == -1)
      return ret;
    int close_pos = ret.indexOf(w_close, split_pos);
    if (close_pos == -1)
      return ret;
    ret.remove(close_pos, 1);
    ret.remove(pos, split_pos - pos + 1);
    pos += close_pos - split_pos - 1;
  }
  return ret;
}

// EOF

/*
// Remove "\n" for scenario text
QString CircusEngine::textFilter(const QString &text, int role)
{
  QString ret = text;
  if (role == Engine::ScenarioRole)
    ret.remove('\n');
  return ret;
}

// Insert "\n"
QString CircusEngine::translationFilter(const QString &text, int role)
{
  if (role != Engine::ScenarioRole)
    return text;
  enum { MaxLineWitdth = 61 }; // estimated in DC3. FIXME: This might be different for different game

  QString ret;
  int lineWidth = 0, // estimated line width
      lineCount = 0; // number of characters
  foreach (const QChar &c, text) {
    int w = c.unicode() <= 255 ? 1 : 2;
    if (lineWidth + w <= MaxLineWitdth) {
      lineCount ++;
      lineWidth += w;
    } else {
      bool wordWrap = false;
      if (lineCount)
        for (int i = 0; i < lineCount; i++)
          if (ret[ret.size() - i - 1].isSpace()) {
            wordWrap = true;
            ret[ret.size() - i - 1] = '\n';
            lineCount = i;
            lineWidth = 0;
            for (int i = 0; i < lineCount; i++)
              lineWidth +=  ret[ret.size() - i - 2].unicode() < 255 ? 1 : 2;
            break;
          }
      if (!wordWrap) {
        ret.append('\n');
        lineWidth = lineCount = 0;
      }
    }
    ret.append(c);
  }
  return ret;
}
*/
