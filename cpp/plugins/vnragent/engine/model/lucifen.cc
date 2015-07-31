// lucifen.cc
// 7/26/2015 jichi
#include "engine/model/lucifen.h"
#include "engine/enginecontroller.h"
#include "engine/enginedef.h"
#include "engine/enginehash.h"
#include "engine/engineutil.h"
#include "hijack/hijackmanager.h"
#include "dyncodec/dynsjis.h"
#include "memdbg/memsearch.h"
#include "winhook/hookcode.h"
#include <qt_windows.h>
#include <cstdint>
#include <unordered_set>

#define DEBUG "model/lucifen"
#include "sakurakit/skdebug.h"

#ifdef _MSC_VER
# pragma warning(disable:4018) // C4018: signed/unsigned mismatch
#endif // _MSC_VER

namespace { // unnamed
namespace ScenarioHook {

std::unordered_set<qint64> textHashes_;

namespace Private {

  ulong scenarioOffset_,
        nameOffset_;

  QByteArray replaceNewLines(const QByteArray &data)
  {
    QByteArray ret;
    //ret.replace("\n", 1, "\x00\x5b\x0c\x00\x00\x00\x0e\x00\x00\x00\x00\x00\x00\x00", 0xc + 2);
    for (auto p = data.constData(); *p;)
      if (*p == '\n') {
        ret.append("\x00\x5b\x0c\x00\x00\x00\x0e\x00\x00\x00\x00\x00\x00\x00", 0xc + 2);
        p++;
      } else {
        ret.push_back(*p++);
        if (*p && dynsjis::isleadbyte(p[-1]))
          ret.push_back(*p++);
      }

    //QByteArray ret;
    //do {
    //  ret.append(start, p - start);
    //  if (dynsjis::prevchar(p, start) == p - 1) {
    //    ret.append("\x00\x5b\x0c\x00\x00\x00\x0e\x00\x00\x00\x00\x00\x00\x00", 0xc + 2);
    //    p++;
    //  } else {
    //    start = p;
    //    p = ::strchr(p, '\n');
    //  }
    //} while (p && *p);
    return ret;
  }

  /**
   *  Sample game: 猫撫ディストーション
   *
   *  0x5b is the text to skip next character
   *
   *  Ruby:
   *  014BB52C  81 77 8C F5 00 5B 1C 00 00 00 1B 00 00 00 01 00  『光.[.......
   *  014BB53C  00 00 03 0B 00 00 00 83 72 83 62 83 4F 83 6F 83  .....ビッグバ・
   *  014BB54C  93 00 81 78 82 CC 91 4F 81 5C 81 5C 00 5B 0C 00  ・』の前――.[..
   *  014BB55C  00 00 0E 00 00 00 00 00 00 00 82 C2 82 DC 82 E8  .........つまり
   *  014BB56C  81 41 89 46 92 88 82 AA 90 B6 82 DC 82 EA 82 E9  、宇宙が生まれる
   *  014BB57C  91 4F 82 A9 82 E7 82 A0 82 C1 82 BD 82 E0 82 CC  前からあったもの
   *  014BB58C  81 42 00 00 00 00 00 00 00 00 00 00 00 00 00 00  。..............
   *  014BB59C  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *
   *  No ruby:
   *  014BB52C  82 B6 82 E1 82 A0 81 41 81 77 8C BE 97 74 81 78  じゃあ、『言葉』
   *  014BB53C  82 C1 82 C4 89 BD 82 C8 82 F1 82 BE 81 48 6F 83  って何なんだ？o・
   *  014BB54C  93 00 81 78 82 CC 91 4F 81 5C 81 5C 00 5B 0C 00  ・』の前――.[..
   *  014BB55C  00 00 0E 00 00 00 00 00 00 00 82 C2 82 DC 82 E8  .........つまり
   *  014BB56C  81 41 89 46 92 88 82 AA 90 B6 82 DC 82 EA 82 E9  、宇宙が生まれる
   *  014BB57C  91 4F 82 A9 82 E7 82 A0 82 C1 82 BD 82 E0 82 CC  前からあったもの
   *  014BB58C  81 42 00 00 00 00 00 00 00 00 00 00 00 00 00 00  。..............
   *  014BB59C  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  014BB5AC  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  014BB5BC  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  014BB5CC  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *
   *  014BB52C  96 85 82 CC 8B D5 00 5B 16 00 00 00 1B 00 00 00  妹の琴.[......
   *  014BB53C  01 00 00 00 03 05 00 00 00 82 B1 82 C6 00 8E 71  ......こと.子
   *  014BB54C  00 5B 14 00 00 00 1B 00 00 00 01 00 00 00 03 03  .[.........
   *  014BB55C  00 00 00 82 B1 00 82 CD 82 BB 82 A4 8C BE 82 C1  ...こ.はそう言っ
   *  014BB56C  82 BD 81 42 82 C6 82 A2 82 A4 88 D3 96 A1 82 F0  た。という意味を
   *  014BB57C  97 5E 82 A6 82 BD 82 CC 81 76 82 BD 82 E0 82 CC  与えたの」たもの
   *  014BB58C  81 42 00 00 00 00 00 00 00 00 00 00 00 00 00 00  。..............
   *  014BB59C  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  014BB5AC  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  014BB5BC  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  014BB5CC  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *
   *  New line:
   *  014D7D39  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  014D7D49  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  014D7D59  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  014D7D69  00 00 00 00 00 01 00 E6 01 00 00 54 01 00 00 00  ......・..T...
   *  014D7D79  00 00 00 B0 11 52 00 D8 CD 4D 01 44 EE E9 07 D8  ...ｰR.ﾘﾍMD鵫ﾘ
   *  014D7D89  CD 4D 01 00 00 00 00 00 00 00 00 00 00 00 00 00  ﾍM.............
   *  014D7D99  00 00 00 F0 50 4E 01 0C 53 4E 01 F0 54 4E 01 10  ...N.SNN
   *  014D7DA9  00 00 00 00 00 00 00 82 BB 82 B5 82 C4 89 B4 82  .......そして俺・
   *  014D7DB9  C9 82 E0 81 41 00 5B 0C 00 00 00 0E 00 00 00 00  ﾉも、.[........
   *  014D7DC9  00 00 00 90 7E 96 5B 82 CC 82 B1 82 EB 82 A9 82  ...厨房のころか・
   *  014D7DD9  E7 8E 6C 94 4E 8A D4 81 41 96 88 93 FA 91 B1 82  邇l年間、毎日続・
   *  014D7DE9  AF 82 C4 82 A2 82 E9 82 B1 82 C6 82 AA 82 A0 82  ｯていることがあ・
   *  014D7DF9  E9 81 42 00 00 00 00 00 00 00 00 00 00 00 00 00  驕B.............
   *  014D7E09  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  014D7E19  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  014D7E29  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  014D7E39  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  014D7E49  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  014D7E59  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  014D7E69  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  014D7E79  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  014D7E89  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   *  014D7E99  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00  ................
   */
  template <typename strT>
  strT ltrimScenarioText(strT p)
  {
    while (p[0] == 0 && p[1] == 0x5b && p[2] > 0)
      p += p[2] + 2;
    return p;
  }
  QByteArray parseScenarioText(const char *p, const char *end)
  {
    int size = ::strlen(p);
    if (end > p && end - p < size)
      size = end - p;
    QByteArray ret;
    if (size)
      ret.setRawData(p, size);
    //if ((uint8_t)p[ret.size() - 1] == 0x93 && (uint8_t)p[ret.size() - 1] == 0x83)// trim encindg \x83\x93
    //  return ret.left(ret.size() - 2);
    for (p += ret.size(); (!end || p < end) && p[1] == 0x5b && p[2] > 0; p += ret.size()) {
      //if (p[2] == 0xc && p[6] == 0xe) {
      //  ret.push_back('\n');
      //  ret.push_back('\n'); // insert double new lines
      //}
      p += p[2] + 2;
      size = ::strlen(p);
      if (end > p && end - p < size)
        size = end - p;
      ret.append(p, size);
    }
    return ret;
  }

  void dispatchNameText(char *text, ulong split)
  {
    enum { capacity = 0x10 }; // excluding '\0'
    enum { role = Engine::NameRole };

    if (!*text)
      return;

    auto sig = Engine::hashThreadSignature(role, split);
    QByteArray oldData = text,
               newData = EngineController::instance()->dispatchTextA(oldData, role, sig);
    if (newData == oldData)
      return;

    int size = newData.size();
    if (size > capacity)
      size = capacity;
    else if (size < oldData.size())
      ::memset(text + size, 0, oldData.size() - size);

    ::memcpy(text, newData.constData(), size);
    text[size] = 0;
    textHashes_.insert(Engine::hashCharArray(text));
  }

  void dispatchScenarioText(char *text, ulong split)
  {
    // text[0] could be \0
    enum { role = Engine::ScenarioRole };
    auto scenarioEndAddress = (LPSTR *)(text + 0x1000);
    auto scenarioEnd = *scenarioEndAddress;
    if (!Engine::isAddressReadable(scenarioEnd))
      scenarioEnd = nullptr;
      //DOUT("warning: scenario end NOT FOUND");

    auto sig = Engine::hashThreadSignature(role, split);

    text = ltrimScenarioText(text);
    if (!*text)
      return;
    QByteArray oldData = parseScenarioText(text, scenarioEnd),
               newData = EngineController::instance()->dispatchTextA(oldData, role, sig);
    if (newData.isEmpty() || newData == oldData)
      return;

    if (newData.contains('\n'))
      newData = replaceNewLines(newData);

    if (scenarioEnd > text && scenarioEnd - text > newData.size())
      ::memset(text + newData.size(), 0, scenarioEnd - text - newData.size());
    else if (oldData.size() > newData.size())
      ::memset(text + newData.size(), 0, oldData.size() - newData.size());

    //::strcpy(text, newData.constData());
    ::memcpy(text, newData.constData(), newData.size() + 1);

    *scenarioEndAddress = text + newData.size(); // FIXME: THis sometimes does not work
  }

  bool hookBefore(winhook::hook_stack *s)
  {
    auto self = (LPSTR)s->ecx;
    ulong retaddr = s->stack[0];
    dispatchNameText(self + nameOffset_, retaddr);
    dispatchScenarioText(self + scenarioOffset_, retaddr);
    return true;
  }

} // namespace Private

/**
 *  Debugging method:
 *  - Hijack GetGlyphOutlineA
 *    There is only one GetGlyphOutlineA
 *  - Find all text in memory
 *    There are two matches.
 *    One is current text with fixed address
 *    One is all text with fixed address
 *  - Find all text address on the stack
 *    There is one function use it as arg1 and as future text
 *    ecx is the current text instead
 *
 *  Sample game: プリズム・プリンセス
 *  name = ecx + 0xadd1
 *  scenario = ecx + 0xae48
 *  scenario end = ecx + 0xbe48
 *
 *  00441E3F   90               NOP
 *  00441E40   83EC 1C          SUB ESP,0x1C
 *  00441E43   53               PUSH EBX
 *  00441E44   56               PUSH ESI
 *  00441E45   8BF1             MOV ESI,ECX
 *  00441E47   8B9E 48BE0000    MOV EBX,DWORD PTR DS:[ESI+0xBE48]
 *  00441E4D   2BDE             SUB EBX,ESI
 *  00441E4F   81EB 48AE0000    SUB EBX,0xAE48
 *  00441E55   75 0B            JNZ SHORT .00441E62
 *  00441E57   5E               POP ESI
 *  00441E58   B8 01000000      MOV EAX,0x1
 *  00441E5D   5B               POP EBX
 *  00441E5E   83C4 1C          ADD ESP,0x1C
 *  00441E61   C3               RETN
 *  00441E62   8B86 AC040000    MOV EAX,DWORD PTR DS:[ESI+0x4AC]
 *  00441E68   55               PUSH EBP
 *  00441E69   57               PUSH EDI
 *  00441E6A   50               PUSH EAX
 *  00441E6B   8BCE             MOV ECX,ESI
 *  00441E6D   E8 9E6CFFFF      CALL .00438B10
 *  00441E72   8A96 DE050000    MOV DL,BYTE PTR DS:[ESI+0x5DE]
 *  00441E78   8B8E 909E0000    MOV ECX,DWORD PTR DS:[ESI+0x9E90]
 *  00441E7E   8BBE 489E0000    MOV EDI,DWORD PTR DS:[ESI+0x9E48]
 *  00441E84   84D2             TEST DL,DL
 *  00441E86   0F94C0           SETE AL
 *  00441E89   84C0             TEST AL,AL
 *  00441E8B   884424 13        MOV BYTE PTR SS:[ESP+0x13],AL
 *  00441E8F   C741 20 00000000 MOV DWORD PTR DS:[ECX+0x20],0x0
 *  00441E96   74 0D            JE SHORT .00441EA5
 *  00441E98   8BCE             MOV ECX,ESI
 *
 *  00441E9A   E8 4136FFFF      CALL .004354E0
 *  00441E9F   8987 A8030000    MOV DWORD PTR DS:[EDI+0x3A8],EAX
 *  00441EA5   8D86 48AE0000    LEA EAX,DWORD PTR DS:[ESI+0xAE48]	; jichi: this is the scenari text
 *  00441EAB   53               PUSH EBX
 *  00441EAC   50               PUSH EAX
 *  00441EAD   8BCF             MOV ECX,EDI
 *  00441EAF   E8 EC6B0000      CALL .00448AA0
 *  00441EB4   8D9E E2AD0000    LEA EBX,DWORD PTR DS:[ESI+0xADE2]	; jichi: this is the character name
 *  00441EBA   8D86 D1AD0000    LEA EAX,DWORD PTR DS:[ESI+0xADD1]	; jichi: this is the name text
 *  00441EC0   53               PUSH EBX
 *  00441EC1   50               PUSH EAX
 *  00441EC2   8BCF             MOV ECX,EDI
 *  00441EC4   894424 1C        MOV DWORD PTR SS:[ESP+0x1C],EAX
 *  00441EC8   E8 836B0000      CALL .00448A50
 *
 *  00441ECD   8A4424 13        MOV AL,BYTE PTR SS:[ESP+0x13]
 *  00441ED1   84C0             TEST AL,AL
 *  00441ED3   74 30            JE SHORT .00441F05
 *  00441ED5   6A 01            PUSH 0x1
 *  00441ED7   8BCF             MOV ECX,EDI
 *  00441ED9   E8 726D0000      CALL .00448C50
 *  00441EDE   803B 00          CMP BYTE PTR DS:[EBX],0x0
 *  00441EE1   74 22            JE SHORT .00441F05
 *  00441EE3   8B86 00AE0000    MOV EAX,DWORD PTR DS:[ESI+0xAE00]
 *  00441EE9   85C0             TEST EAX,EAX
 *  00441EEB   75 18            JNZ SHORT .00441F05
 *  00441EED   8B86 AC040000    MOV EAX,DWORD PTR DS:[ESI+0x4AC]
 *  00441EF3   8D97 D1030000    LEA EDX,DWORD PTR DS:[EDI+0x3D1]
 *  00441EF9   8996 00AE0000    MOV DWORD PTR DS:[ESI+0xAE00],EDX
 *  00441EFF   8986 C0040000    MOV DWORD PTR DS:[ESI+0x4C0],EAX
 *  00441F05   8A86 30A60000    MOV AL,BYTE PTR DS:[ESI+0xA630]
 *  00441F0B   84C0             TEST AL,AL
 *  00441F0D   0F84 DB000000    JE .00441FEE
 *  00441F13   8B86 C0A00000    MOV EAX,DWORD PTR DS:[ESI+0xA0C0]
 *  00441F19   85C0             TEST EAX,EAX
 *  00441F1B   0F84 CD000000    JE .00441FEE
 *  00441F21   8B96 E0A00000    MOV EDX,DWORD PTR DS:[ESI+0xA0E0]
 *  00441F27   8DAE E0A00000    LEA EBP,DWORD PTR DS:[ESI+0xA0E0]
 *  00441F2D   6A 00            PUSH 0x0
 *  00441F2F   8BCD             MOV ECX,EBP
 *  00441F31   FF92 B4000000    CALL DWORD PTR DS:[EDX+0xB4]
 *  00441F37   8B86 489E0000    MOV EAX,DWORD PTR DS:[ESI+0x9E48]
 *  00441F3D   8D8E 5C470000    LEA ECX,DWORD PTR DS:[ESI+0x475C]
 *  00441F43   8D96 14680000    LEA EDX,DWORD PTR DS:[ESI+0x6814]
 *  00441F49   898E E4050000    MOV DWORD PTR DS:[ESI+0x5E4],ECX
 *  00441F4F   894424 18        MOV DWORD PTR SS:[ESP+0x18],EAX
 *  00441F53   89AE 489E0000    MOV DWORD PTR DS:[ESI+0x9E48],EBP
 *  00441F59   C686 D8A00000 01 MOV BYTE PTR DS:[ESI+0xA0D8],0x1
 *  00441F60   8996 E8050000    MOV DWORD PTR DS:[ESI+0x5E8],EDX
 *  00441F66   8B87 B4030000    MOV EAX,DWORD PTR DS:[EDI+0x3B4]
 *  00441F6C   6A 01            PUSH 0x1
 *  00441F6E   8D4C24 20        LEA ECX,DWORD PTR SS:[ESP+0x20]
 *  00441F72   6A 01            PUSH 0x1
 *  00441F74   51               PUSH ECX
 *  00441F75   50               PUSH EAX
 *  00441F76   8BCD             MOV ECX,EBP
 *  00441F78   E8 935D0000      CALL .00447D10
 *  00441F7D   8B5424 18        MOV EDX,DWORD PTR SS:[ESP+0x18]
 *  00441F81   8D8E EC050000    LEA ECX,DWORD PTR DS:[ESI+0x5EC]
 *  00441F87   8996 489E0000    MOV DWORD PTR DS:[ESI+0x9E48],EDX
 *  00441F8D   8D96 A4260000    LEA EDX,DWORD PTR DS:[ESI+0x26A4]
 *  00441F93   85C0             TEST EAX,EAX
 *  00441F95   C686 D8A00000 00 MOV BYTE PTR DS:[ESI+0xA0D8],0x0
 *  00441F9C   898E E4050000    MOV DWORD PTR DS:[ESI+0x5E4],ECX
 *  00441FA2   8996 E8050000    MOV DWORD PTR DS:[ESI+0x5E8],EDX
 *  00441FA8   7E 44            JLE SHORT .00441FEE
 *  00441FAA   8A86 31A60000    MOV AL,BYTE PTR DS:[ESI+0xA631]
 *  00441FB0   84C0             TEST AL,AL
 *  00441FB2   74 0A            JE SHORT .00441FBE
 *  00441FB4   33C0             XOR EAX,EAX
 *  00441FB6   8A86 32A60000    MOV AL,BYTE PTR DS:[ESI+0xA632]
 *  00441FBC   EB 02            JMP SHORT .00441FC0
 *  00441FBE   33C0             XOR EAX,EAX
 *  00441FC0   8B4C24 28        MOV ECX,DWORD PTR SS:[ESP+0x28]
 *  00441FC4   8B6C24 20        MOV EBP,DWORD PTR SS:[ESP+0x20]
 *  00441FC8   8B97 B8030000    MOV EDX,DWORD PTR DS:[EDI+0x3B8]
 *  00441FCE   50               PUSH EAX
 *  00441FCF   8B4424 18        MOV EAX,DWORD PTR SS:[ESP+0x18]
 *  00441FD3   2BCD             SUB ECX,EBP
 *  00441FD5   53               PUSH EBX
 *  00441FD6   83C1 04          ADD ECX,0x4
 *  00441FD9   50               PUSH EAX
 *  00441FDA   8B87 B4030000    MOV EAX,DWORD PTR DS:[EDI+0x3B4]
 *  00441FE0   51               PUSH ECX
 *  00441FE1   52               PUSH EDX
 *  00441FE2   50               PUSH EAX
 *  00441FE3   8D8E B8A00000    LEA ECX,DWORD PTR DS:[ESI+0xA0B8]
 *  00441FE9   E8 72290000      CALL .00444960
 *  00441FEE   8B4C24 14        MOV ECX,DWORD PTR SS:[ESP+0x14]
 *  00441FF2   8D86 48AE0000    LEA EAX,DWORD PTR DS:[ESI+0xAE48]
 *  00441FF8   5F               POP EDI
 *  00441FF9   8986 48BE0000    MOV DWORD PTR DS:[ESI+0xBE48],EAX
 *  00441FFF   5D               POP EBP
 *  00442000   C603 00          MOV BYTE PTR DS:[EBX],0x0
 *  00442003   5E               POP ESI
 *  00442004   C601 00          MOV BYTE PTR DS:[ECX],0x0
 *  00442007   33C0             XOR EAX,EAX
 *  00442009   5B               POP EBX
 *  0044200A   83C4 1C          ADD ESP,0x1C
 *  0044200D   C3               RETN
 *  0044200E   90               NOP
 *  0044200F   90               NOP
 *
 *  Sample game: 猫撫ディストーション
 *  name = ecx + 0xc60f
 *  scenario = ecx + 0xc684
 *  scenario end = ecx + 0xd684
 *
 *  0043E11E   90               NOP
 *  0043E11F   90               NOP
 *  0043E120   83EC 18          SUB ESP,0x18
 *  0043E123   53               PUSH EBX
 *  0043E124   55               PUSH EBP
 *  0043E125   56               PUSH ESI
 *  0043E126   8BF1             MOV ESI,ECX
 *  0043E128   57               PUSH EDI
 *  0043E129   8BAE 84D60000    MOV EBP,DWORD PTR DS:[ESI+0xD684]	; jichi: overall offset is around 0xD684
 *  0043E12F   2BEE             SUB EBP,ESI
 *  0043E131   81ED 84C60000    SUB EBP,0xC684
 *  0043E137   896C24 10        MOV DWORD PTR SS:[ESP+0x10],EBP
 *  0043E13B   75 0D            JNZ SHORT .0043E14A
 *  0043E13D   5F               POP EDI
 *  0043E13E   5E               POP ESI
 *  0043E13F   5D               POP EBP
 *  0043E140   B8 01000000      MOV EAX,0x1
 *  0043E145   5B               POP EBX
 *  0043E146   83C4 18          ADD ESP,0x18
 *  0043E149   C3               RETN
 *  0043E14A   8B86 A8040000    MOV EAX,DWORD PTR DS:[ESI+0x4A8]
 *  0043E150   8BCE             MOV ECX,ESI
 *  0043E152   50               PUSH EAX
 *  0043E153   E8 3875FFFF      CALL .00435690
 *  0043E158   8B9E F4B20000    MOV EBX,DWORD PTR DS:[ESI+0xB2F4]
 *  0043E15E   8BBE D8B10000    MOV EDI,DWORD PTR DS:[ESI+0xB1D8]
 *  0043E164   8B43 14          MOV EAX,DWORD PTR DS:[EBX+0x14]
 *  0043E167   85C0             TEST EAX,EAX
 *  0043E169   7D 7C            JGE SHORT .0043E1E7
 *  0043E16B   8B8E 70040000    MOV ECX,DWORD PTR DS:[ESI+0x470]
 *  0043E171   6A 00            PUSH 0x0
 *  0043E173   8D96 20C60000    LEA EDX,DWORD PTR DS:[ESI+0xC620]	; jichi: 0xc620 is the nearest position
 *  0043E179   6A 00            PUSH 0x0
 *  0043E17B   52               PUSH EDX
 *  0043E17C   6A FE            PUSH -0x2
 *  0043E17E   E8 ED93FEFF      CALL .00427570
 *  0043E183   8BE8             MOV EBP,EAX
 *  0043E185   85ED             TEST EBP,EBP
 *  0043E187   7C 0D            JL SHORT .0043E196
 *  0043E189   45               INC EBP
 *  0043E18A   83FD 08          CMP EBP,0x8
 *  0043E18D   7C 09            JL SHORT .0043E198
 *  0043E18F   BD 07000000      MOV EBP,0x7
 *  0043E194   EB 02            JMP SHORT .0043E198
 *  0043E196   33ED             XOR EBP,EBP
 *  0043E198   396B 1C          CMP DWORD PTR DS:[EBX+0x1C],EBP
 *  0043E19B   74 46            JE SHORT .0043E1E3
 *  0043E19D   8B8F 4C020000    MOV ECX,DWORD PTR DS:[EDI+0x24C]
 *  0043E1A3   85C9             TEST ECX,ECX
 *  0043E1A5   75 0D            JNZ SHORT .0043E1B4
 *  0043E1A7   5F               POP EDI
 *  0043E1A8   5E               POP ESI
 *  0043E1A9   5D               POP EBP
 *  0043E1AA   B8 02000000      MOV EAX,0x2
 *  0043E1AF   5B               POP EBX
 *  0043E1B0   83C4 18          ADD ESP,0x18
 *  0043E1B3   C3               RETN
 *  0043E1B4   8BC5             MOV EAX,EBP
 *  0043E1B6   6A 00            PUSH 0x0
 *  0043E1B8   C1E0 04          SHL EAX,0x4
 *  0043E1BB   03C5             ADD EAX,EBP
 *  0043E1BD   6A 00            PUSH 0x0
 *  0043E1BF   6A 00            PUSH 0x0
 *  0043E1C1   6A 00            PUSH 0x0
 *  0043E1C3   8D94C6 48BA0000  LEA EDX,DWORD PTR DS:[ESI+EAX*8+0xBA48]
 *  0043E1CA   52               PUSH EDX
 *  0043E1CB   E8 E0DD0200      CALL .0046BFB0
 *  0043E1D0   896B 1C          MOV DWORD PTR DS:[EBX+0x1C],EBP
 *  0043E1D3   8B07             MOV EAX,DWORD PTR DS:[EDI]
 *  0043E1D5   6A 01            PUSH 0x1
 *  0043E1D7   6A 01            PUSH 0x1
 *  0043E1D9   6A 01            PUSH 0x1
 *  0043E1DB   8BCF             MOV ECX,EDI
 *  0043E1DD   FF90 4C010000    CALL DWORD PTR DS:[EAX+0x14C]
 *  0043E1E3   8B6C24 10        MOV EBP,DWORD PTR SS:[ESP+0x10]
 *  0043E1E7   8BCE             MOV ECX,ESI
 *  0043E1E9   C743 20 00000000 MOV DWORD PTR DS:[EBX+0x20],0x0
 *
 *  0043E1F0   E8 3B46FFFF      CALL .00432830
 *  0043E1F5   8987 A0030000    MOV DWORD PTR DS:[EDI+0x3A0],EAX
 *  0043E1FB   8D86 84C60000    LEA EAX,DWORD PTR DS:[ESI+0xC684]	; jichi: this is scenario
 *  0043E201   55               PUSH EBP
 *  0043E202   50               PUSH EAX
 *  0043E203   8BCF             MOV ECX,EDI
 *  0043E205   E8 765F0000      CALL .00444180
 *  0043E20A   8D9E 20C60000    LEA EBX,DWORD PTR DS:[ESI+0xC620]	; jichi: this is the chara name, such as KOT0
 *  0043E210   8D86 0FC60000    LEA EAX,DWORD PTR DS:[ESI+0xC60F]	; jichi: this is the name address
 *  0043E216   53               PUSH EBX
 *  0043E217   50               PUSH EAX
 *  0043E218   8BCF             MOV ECX,EDI
 *  0043E21A   894424 18        MOV DWORD PTR SS:[ESP+0x18],EAX
 *  0043E21E   E8 0D5F0000      CALL .00444130
 *
 *  0043E223   6A 01            PUSH 0x1
 *  0043E225   8BCF             MOV ECX,EDI
 *  0043E227   E8 04600000      CALL .00444230
 *  0043E22C   8A86 40BA0000    MOV AL,BYTE PTR DS:[ESI+0xBA40]
 *  0043E232   84C0             TEST AL,AL
 *  0043E234   0F84 DB000000    JE .0043E315
 *  0043E23A   8B86 18B50000    MOV EAX,DWORD PTR DS:[ESI+0xB518]
 *  0043E240   85C0             TEST EAX,EAX
 *  0043E242   0F84 CD000000    JE .0043E315
 *  0043E248   8B96 38B50000    MOV EDX,DWORD PTR DS:[ESI+0xB538]
 *  0043E24E   8DAE 38B50000    LEA EBP,DWORD PTR DS:[ESI+0xB538]
 *  0043E254   6A 00            PUSH 0x0
 *  0043E256   8BCD             MOV ECX,EBP
 *  0043E258   FF92 B4000000    CALL DWORD PTR DS:[EDX+0xB4]
 *  0043E25E   8B86 D8B10000    MOV EAX,DWORD PTR DS:[ESI+0xB1D8]
 *  0043E264   8D8E 70460000    LEA ECX,DWORD PTR DS:[ESI+0x4670]
 *  0043E26A   8D96 28670000    LEA EDX,DWORD PTR DS:[ESI+0x6728]
 *  0043E270   898E F8040000    MOV DWORD PTR DS:[ESI+0x4F8],ECX
 *  0043E276   894424 14        MOV DWORD PTR SS:[ESP+0x14],EAX
 *  0043E27A   89AE D8B10000    MOV DWORD PTR DS:[ESI+0xB1D8],EBP
 *  0043E280   C686 30B50000 01 MOV BYTE PTR DS:[ESI+0xB530],0x1
 *  0043E287   8996 FC040000    MOV DWORD PTR DS:[ESI+0x4FC],EDX
 *  0043E28D   8B87 AC030000    MOV EAX,DWORD PTR DS:[EDI+0x3AC]
 *  0043E293   6A 01            PUSH 0x1
 *  0043E295   8D4C24 1C        LEA ECX,DWORD PTR SS:[ESP+0x1C]
 *  0043E299   6A 01            PUSH 0x1
 *  0043E29B   51               PUSH ECX
 *  0043E29C   50               PUSH EAX
 *  0043E29D   8BCD             MOV ECX,EBP
 *  0043E29F   E8 DC570000      CALL .00443A80
 *  0043E2A4   8B5424 14        MOV EDX,DWORD PTR SS:[ESP+0x14]
 *  0043E2A8   8D8E 00050000    LEA ECX,DWORD PTR DS:[ESI+0x500]
 *  0043E2AE   8996 D8B10000    MOV DWORD PTR DS:[ESI+0xB1D8],EDX
 *  0043E2B4   8D96 B8250000    LEA EDX,DWORD PTR DS:[ESI+0x25B8]
 *  0043E2BA   85C0             TEST EAX,EAX
 *  0043E2BC   C686 30B50000 00 MOV BYTE PTR DS:[ESI+0xB530],0x0
 *  0043E2C3   898E F8040000    MOV DWORD PTR DS:[ESI+0x4F8],ECX
 *  0043E2C9   8996 FC040000    MOV DWORD PTR DS:[ESI+0x4FC],EDX
 *  0043E2CF   7E 44            JLE SHORT .0043E315
 *  0043E2D1   8A86 41BA0000    MOV AL,BYTE PTR DS:[ESI+0xBA41]
 *  0043E2D7   84C0             TEST AL,AL
 *  0043E2D9   74 0A            JE SHORT .0043E2E5
 *  0043E2DB   33C0             XOR EAX,EAX
 *  0043E2DD   8A86 42BA0000    MOV AL,BYTE PTR DS:[ESI+0xBA42]
 *  0043E2E3   EB 02            JMP SHORT .0043E2E7
 *  0043E2E5   33C0             XOR EAX,EAX
 *  0043E2E7   8B4C24 24        MOV ECX,DWORD PTR SS:[ESP+0x24]
 *  0043E2EB   8B6C24 1C        MOV EBP,DWORD PTR SS:[ESP+0x1C]
 *  0043E2EF   8B97 B0030000    MOV EDX,DWORD PTR DS:[EDI+0x3B0]
 *  0043E2F5   50               PUSH EAX
 *  0043E2F6   8B4424 14        MOV EAX,DWORD PTR SS:[ESP+0x14]
 *  0043E2FA   2BCD             SUB ECX,EBP
 *  0043E2FC   53               PUSH EBX
 *  0043E2FD   83C1 04          ADD ECX,0x4
 *  0043E300   50               PUSH EAX
 *  0043E301   8B87 AC030000    MOV EAX,DWORD PTR DS:[EDI+0x3AC]
 *  0043E307   51               PUSH ECX
 *  0043E308   52               PUSH EDX
 *  0043E309   50               PUSH EAX
 *  0043E30A   8D8E 10B50000    LEA ECX,DWORD PTR DS:[ESI+0xB510]
 *  0043E310   E8 7B270000      CALL .00440A90
 *  0043E315   803B 00          CMP BYTE PTR DS:[EBX],0x0
 *  0043E318   74 0C            JE SHORT .0043E326
 *  0043E31A   81C7 C9030000    ADD EDI,0x3C9
 *  0043E320   89BE 3CC60000    MOV DWORD PTR DS:[ESI+0xC63C],EDI
 *  0043E326   8B4C24 10        MOV ECX,DWORD PTR SS:[ESP+0x10]
 *  0043E32A   8D86 84C60000    LEA EAX,DWORD PTR DS:[ESI+0xC684]
 *  0043E330   8986 84D60000    MOV DWORD PTR DS:[ESI+0xD684],EAX
 *  0043E336   5F               POP EDI
 *  0043E337   5E               POP ESI
 *  0043E338   C603 00          MOV BYTE PTR DS:[EBX],0x0
 *  0043E33B   5D               POP EBP
 *  0043E33C   C601 00          MOV BYTE PTR DS:[ECX],0x0
 *  0043E33F   33C0             XOR EAX,EAX
 *  0043E341   5B               POP EBX
 *  0043E342   83C4 18          ADD ESP,0x18
 *  0043E345   C3               RETN
 *  0043E346   90               NOP
 *  0043E347   90               NOP
 *  0043E348   90               NOP
 *  0043E349   90               NOP
 *  0043E34A   90               NOP
 *  0043E34B   90               NOP
 */
bool attach(ulong startAddress, ulong stopAddress) // attach scenario
{
  const uint8_t bytes[] = {
    0xe8, XX4,              // 0043e1f0   e8 3b46ffff      call .00432830
    0x89,0x87, XX4,         // 0043e1f5   8987 a0030000    mov dword ptr ds:[edi+0x3a0],eax
    0x8d,0x86, XX4,         // 0043e1fb   8d86 84c60000    lea eax,dword ptr ds:[esi+0xc684]	; jichi: this is scenario
                            // 0043e201   55               push ebp
                            // 0043e202   50               push eax
    XX4,                    // 0043e203   8bcf             mov ecx,edi
    0xe8, XX4,              // 0043e205   e8 765f0000      call .00444180
    0x8d,0x9e, XX4,         // 0043e20a   8d9e 20c60000    lea ebx,dword ptr ds:[esi+0xc620]	; jichi: this is the chara name, such as kot0
    0x8d,0x86, XX4,         // 0043e210   8d86 0fc60000    lea eax,dword ptr ds:[esi+0xc60f]	; jichi: this is the name address
    0x53,                   // 0043e216   53               push ebx
    0x50,                   // 0043e217   50               push eax
    0x8b,0xcf,              // 0043e218   8bcf             mov ecx,edi
    0x89,0x44,0x24, XX,     // 0043e21a   894424 18        mov dword ptr ss:[esp+0x18],eax
    0xe8 //, XX4            // 0043e21e   e8 0d5f0000      call .00444130
  };
  ulong addr = MemDbg::matchBytes(bytes, sizeof(bytes), startAddress, stopAddress);
  if (!addr)
    return false;

  Private::scenarioOffset_ = *(DWORD *)(addr + 2 + 0x0043e1fb - 0x0043e1f0);
  Private::nameOffset_ = *(DWORD *)(addr + 2 + 0x0043e210 - 0x0043e1f0);
  if ((Private::scenarioOffset_ >> 16) || // offset high bits are zero
      (Private::nameOffset_ >> 16))
    return false;

  addr = MemDbg::findEnclosingAlignedFunction(addr);
  if (!addr)
    return false;
  return winhook::hook_before(addr, Private::hookBefore);
}
} // namespace ScenarioHook

namespace ChoiceHook {
namespace Private {

  bool hookBefore(winhook::hook_stack *s)
  {
    static QByteArray data_;
    auto text = (LPCSTR)s->stack[0]; // arg1 is text
    if (!text || !*text)
      return text;
    enum { role = Engine::ChoiceRole, sig = Engine::OtherThreadSignature };
    data_ = EngineController::instance()->dispatchTextA(text, role, sig);
    s->stack[0] = (ulong)data_.constData();
    return true;
  }

} // namespace Private

/**
 *  Debugging method:
 *  - Hijack GetGlyphOutlineA
 *  - Backtrack stack to find text that used as argument
 *
 *  Sample game: プリズム・プリンセス
 *
 *  Text in arg1.
 *
 *  The function is only called by one caller.
 *  I suspect it is a virtual function, and hence caller is hooked.
 *
 *  0044235E   90               NOP
 *  0044235F   90               NOP
 *  00442360   83EC 08          SUB ESP,0x8
 *  00442363   53               PUSH EBX
 *  00442364   56               PUSH ESI
 *  00442365   8BF1             MOV ESI,ECX
 *  00442367   BB 01000000      MOV EBX,0x1
 *  0044236C   8A86 E2050000    MOV AL,BYTE PTR DS:[ESI+0x5E2]
 *  00442372   84C0             TEST AL,AL
 *  00442374   75 14            JNZ SHORT .0044238A
 *  00442376   889E BD040000    MOV BYTE PTR DS:[ESI+0x4BD],BL
 *  0044237C   E8 BFFAFFFF      CALL .00441E40
 *  00442381   85C0             TEST EAX,EAX
 *  00442383   0F94C0           SETE AL
 *  00442386   84C0             TEST AL,AL
 *  00442388   74 16            JE SHORT .004423A0
 *  0044238A   53               PUSH EBX
 *  0044238B   6A 00            PUSH 0x0
 *  0044238D   8BCE             MOV ECX,ESI
 *  0044238F   E8 2C80FFFF      CALL .0043A3C0
 *  00442394   85C0             TEST EAX,EAX
 *  00442396   74 16            JE SHORT .004423AE
 *  00442398   5E               POP ESI
 *  00442399   5B               POP EBX
 *  0044239A   83C4 08          ADD ESP,0x8
 *  0044239D   C2 0400          RETN 0x4
 *  004423A0   8B86 88040000    MOV EAX,DWORD PTR DS:[ESI+0x488]
 *  004423A6   8BCE             MOV ECX,ESI
 *  004423A8   50               PUSH EAX
 *  004423A9   E8 32120700      CALL .004B35E0
 *  004423AE   8B96 949E0000    MOV EDX,DWORD PTR DS:[ESI+0x9E94]
 *  004423B4   55               PUSH EBP
 *  004423B5   8DAE 949E0000    LEA EBP,DWORD PTR DS:[ESI+0x9E94]
 *  004423BB   57               PUSH EDI
 *  004423BC   8BCD             MOV ECX,EBP
 *  004423BE   C686 BD040000 00 MOV BYTE PTR DS:[ESI+0x4BD],0x0
 *  004423C5   FF92 80000000    CALL DWORD PTR DS:[EDX+0x80]
 *  004423CB   8B86 44040000    MOV EAX,DWORD PTR DS:[ESI+0x444]
 *  004423D1   85C0             TEST EAX,EAX
 *  004423D3   74 05            JE SHORT .004423DA
 *  004423D5   83C0 18          ADD EAX,0x18
 *  004423D8   EB 02            JMP SHORT .004423DC
 *  004423DA   33C0             XOR EAX,EAX
 *  004423DC   8B8E A0A00000    MOV ECX,DWORD PTR DS:[ESI+0xA0A0]
 *  004423E2   8B7C24 1C        MOV EDI,DWORD PTR SS:[ESP+0x1C]
 *  004423E6   8B55 00          MOV EDX,DWORD PTR SS:[EBP]
 *  004423E9   51               PUSH ECX
 *  004423EA   8B4F 4C          MOV ECX,DWORD PTR DS:[EDI+0x4C]
 *  004423ED   51               PUSH ECX
 *  004423EE   50               PUSH EAX
 *  004423EF   8BCD             MOV ECX,EBP
 *  004423F1   FF92 AC000000    CALL DWORD PTR DS:[EDX+0xAC]
 *  004423F7   B8 02000000      MOV EAX,0x2
 *  004423FC   8D4F 08          LEA ECX,DWORD PTR DS:[EDI+0x8]
 *  004423FF   8339 00          CMP DWORD PTR DS:[ECX],0x0
 *  00442402   74 0B            JE SHORT .0044240F
 *  00442404   83C0 02          ADD EAX,0x2
 *  00442407   83C1 08          ADD ECX,0x8
 *  0044240A   83F8 12          CMP EAX,0x12
 *  0044240D  ^7C F0            JL SHORT .004423FF
 *  0044240F   D1F8             SAR EAX,1
 *  00442411   48               DEC EAX
 *  00442412   8BF8             MOV EDI,EAX
 *  00442414   8A86 30A60000    MOV AL,BYTE PTR DS:[ESI+0xA630]
 *  0044241A   84C0             TEST AL,AL
 *  0044241C   897C24 14        MOV DWORD PTR SS:[ESP+0x14],EDI
 *  00442420   89BE 9CA00000    MOV DWORD PTR DS:[ESI+0xA09C],EDI
 *  00442426   0F84 B9000000    JE .004424E5
 *  0044242C   8B86 C0A00000    MOV EAX,DWORD PTR DS:[ESI+0xA0C0]
 *  00442432   85C0             TEST EAX,EAX
 *  00442434   0F84 AB000000    JE .004424E5
 *  0044243A   57               PUSH EDI
 *  0044243B   8D8E B8A00000    LEA ECX,DWORD PTR DS:[ESI+0xA0B8]
 *  00442441   885C24 17        MOV BYTE PTR SS:[ESP+0x17],BL
 *  00442445   E8 46270000      CALL .00444B90
 *  0044244A   33DB             XOR EBX,EBX
 *  0044244C   85FF             TEST EDI,EDI
 *  0044244E   7E 64            JLE SHORT .004424B4
 *  00442450   8B5424 1C        MOV EDX,DWORD PTR SS:[ESP+0x1C]
 *  00442454   8D7A 0C          LEA EDI,DWORD PTR DS:[EDX+0xC]
 *  00442457   8A941E B8040000  MOV DL,BYTE PTR DS:[ESI+EBX+0x4B8]
 *  0044245E   8B45 00          MOV EAX,DWORD PTR SS:[EBP]
 *  00442461   6A 00            PUSH 0x0
 *  00442463   6A 00            PUSH 0x0
 *  00442465   84D2             TEST DL,DL
 *  00442467   8B17             MOV EDX,DWORD PTR DS:[EDI]
 *  00442469   6A 00            PUSH 0x0
 *  0044246B   0F954424 28      SETNE BYTE PTR SS:[ESP+0x28]
 *  00442470   8B4C24 28        MOV ECX,DWORD PTR SS:[ESP+0x28]
 *  00442474   6A 00            PUSH 0x0
 *  00442476   6A FF            PUSH -0x1
 *  00442478   6A 00            PUSH 0x0
 *  0044247A   6A FF            PUSH -0x1
 *  0044247C   51               PUSH ECX
 *  0044247D   6A 00            PUSH 0x0
 *  0044247F   52               PUSH EDX
 *  00442480   8BCD             MOV ECX,EBP
 *  00442482   FF90 84000000    CALL DWORD PTR DS:[EAX+0x84]             ; .004BBD00	; jichi: text called here, text on the top
 *  00442488   8A4424 13        MOV AL,BYTE PTR SS:[ESP+0x13]
 *  0044248C   84C0             TEST AL,AL
 *  0044248E   74 18            JE SHORT .004424A8
 *  00442490   8A5424 1C        MOV DL,BYTE PTR SS:[ESP+0x1C]
 *  00442494   8B0F             MOV ECX,DWORD PTR DS:[EDI]
 *  00442496   84D2             TEST DL,DL
 *  00442498   0F94C0           SETE AL
 *  0044249B   50               PUSH EAX
 *  0044249C   51               PUSH ECX
 *  0044249D   8D8E B8A00000    LEA ECX,DWORD PTR DS:[ESI+0xA0B8]
 *  004424A3   E8 48280000      CALL .00444CF0
 *  004424A8   8B4424 14        MOV EAX,DWORD PTR SS:[ESP+0x14]
 *  004424AC   83C7 08          ADD EDI,0x8
 *  004424AF   43               INC EBX
 *  004424B0   3BD8             CMP EBX,EAX
 *  004424B2  ^7C A3            JL SHORT .00442457
 *  004424B4   8A4424 13        MOV AL,BYTE PTR SS:[ESP+0x13]
 *  004424B8   5F               POP EDI
 *  004424B9   84C0             TEST AL,AL
 *  004424BB   5D               POP EBP
 *  004424BC   74 12            JE SHORT .004424D0
 *  004424BE   8D96 34A60000    LEA EDX,DWORD PTR DS:[ESI+0xA634]
 *  004424C4   8D8E B8A00000    LEA ECX,DWORD PTR DS:[ESI+0xA0B8]
 *  004424CA   52               PUSH EDX
 *  004424CB   E8 B0280000      CALL .00444D80
 *  004424D0   33C0             XOR EAX,EAX
 *  004424D2   81C6 B8040000    ADD ESI,0x4B8
 *  004424D8   8906             MOV DWORD PTR DS:[ESI],EAX
 *  004424DA   8846 04          MOV BYTE PTR DS:[ESI+0x4],AL
 *  004424DD   5E               POP ESI
 *  004424DE   5B               POP EBX
 *  004424DF   83C4 08          ADD ESP,0x8
 *  004424E2   C2 0400          RETN 0x4
 *  004424E5   C64424 13 00     MOV BYTE PTR SS:[ESP+0x13],0x0
 *  004424EA  ^E9 5BFFFFFF      JMP .0044244A
 *  004424EF   90               NOP
 *  004424F0   8B4424 04        MOV EAX,DWORD PTR SS:[ESP+0x4]
 *  004424F4   8B40 04          MOV EAX,DWORD PTR DS:[EAX+0x4]
 *  004424F7   85C0             TEST EAX,EAX
 *  004424F9   7C 0D            JL SHORT .00442508
 *  004424FB   83F8 05          CMP EAX,0x5
 *  004424FE   7D 08            JGE SHORT .00442508
 *  00442500   C68408 B8040000 >MOV BYTE PTR DS:[EAX+ECX+0x4B8],0x1
 *  00442508   33C0             XOR EAX,EAX
 *  0044250A   C2 0400          RETN 0x4
 *  0044250D   90               NOP
 *  0044250E   90               NOP
 */
bool attach(ulong startAddress, ulong stopAddress) // attach scenario
{
  const uint8_t bytes[] = {
    0xff,0x90, 0x84,0x00,0x00,0x00, // 00442482   ff90 84000000    call dword ptr ds:[eax+0x84]             ; .004bbd00	; jichi: text called here, text on the top
    0x8a,0x44,0x24, 0x13            // 00442488   8a4424 13        mov al,byte ptr ss:[esp+0x13]
  };
  ulong addr = MemDbg::findBytes(bytes, sizeof(bytes), startAddress, stopAddress);
  return addr && winhook::hook_before(addr, Private::hookBefore);
}
} // namespace ChoiceHook

namespace OtherHook {
namespace Private {

  bool hookBefore(winhook::hook_stack *s)
  {
    auto text = (LPSTR)s->stack[1]; // arg1 is text
    if (!text || ::strlen(text) <= 2
        || ScenarioHook::textHashes_.find(Engine::hashCharArray(text)) != ScenarioHook::textHashes_.end())
      return text;

    enum { role = Engine::OtherRole };
    auto split = s->stack[0];
    auto sig = Engine::hashThreadSignature(role, split);
    QByteArray oldData = text,
               newData = EngineController::instance()->dispatchTextA(oldData, role, sig);
    if (newData == oldData)
      return true;
    size_t capacity = Engine::countZero(text + oldData.size());
    if (!capacity)
      return true;
    capacity += oldData.size() - 1;
    if (newData.size() > capacity)
      newData = newData.left(capacity);
   if (newData.size() < oldData.size())
     ::memset(text + newData.size(), 0, oldData.size() - newData.size());
    ::strcpy(text, newData.constData());
    ScenarioHook::textHashes_.insert(Engine::hashCharArray(text));
    return true;
  }

} // namespace Private

/**
 *  Debugging method:
 *  - Hijack GetTextExtentPoint32A
 *  - Backtrack stack to find text that used as argument
 *
 *  Sample game: プリズム・プリンセス
 *
 *  Text in arg1, that could be accessed either character-by-character or entire text
 *
 *  It will capture save message, name, and ruby.
 *
 *  00461C9E   90               NOP
 *  00461C9F   90               NOP
 *  00461CA0   83EC 14          SUB ESP,0x14
 *  00461CA3   33D2             XOR EDX,EDX
 *  00461CA5   55               PUSH EBP
 *  00461CA6   56               PUSH ESI
 *  00461CA7   8B7424 20        MOV ESI,DWORD PTR SS:[ESP+0x20]
 *  00461CAB   8BE9             MOV EBP,ECX
 *  00461CAD   3BF2             CMP ESI,EDX
 *  00461CAF   0F84 55020000    JE .00461F0A
 *  00461CB5   3955 08          CMP DWORD PTR SS:[EBP+0x8],EDX
 *  00461CB8   0F84 4C020000    JE .00461F0A
 *  00461CBE   8B85 74200000    MOV EAX,DWORD PTR SS:[EBP+0x2074]
 *  00461CC4   53               PUSH EBX
 *  00461CC5   57               PUSH EDI
 *  00461CC6   8B7D 1C          MOV EDI,DWORD PTR SS:[EBP+0x1C]
 *  00461CC9   81FF BC020000    CMP EDI,0x2BC
 *  00461CCF   895424 18        MOV DWORD PTR SS:[ESP+0x18],EDX
 *  00461CD3   0F9D4424 28      SETGE BYTE PTR SS:[ESP+0x28]
 *  00461CD8   3BC2             CMP EAX,EDX
 *  00461CDA   74 07            JE SHORT .00461CE3
 *  00461CDC   8B40 14          MOV EAX,DWORD PTR DS:[EAX+0x14]
 *  00461CDF   894424 18        MOV DWORD PTR SS:[ESP+0x18],EAX
 *  00461CE3   8B7C24 2C        MOV EDI,DWORD PTR SS:[ESP+0x2C]
 *  00461CE7   33DB             XOR EBX,EBX
 *  00461CE9   C74424 14 010000>MOV DWORD PTR SS:[ESP+0x14],0x1
 *  00461CF1   895424 10        MOV DWORD PTR SS:[ESP+0x10],EDX
 *  00461CF5   8917             MOV DWORD PTR DS:[EDI],EDX
 *  00461CF7   8957 04          MOV DWORD PTR DS:[EDI+0x4],EDX
 *  00461CFA   803E 00          CMP BYTE PTR DS:[ESI],0x0
 *  00461CFD   0F84 07010000    JE .00461E0A
 *  00461D03   8A06             MOV AL,BYTE PTR DS:[ESI]
 *  00461D05   895424 1C        MOV DWORD PTR SS:[ESP+0x1C],EDX
 *  00461D09   3C 81            CMP AL,0x81
 *  00461D0B   895424 20        MOV DWORD PTR SS:[ESP+0x20],EDX
 *  00461D0F   72 04            JB SHORT .00461D15
 *  00461D11   3C 9F            CMP AL,0x9F
 *  00461D13   76 08            JBE SHORT .00461D1D
 *  00461D15   3C E0            CMP AL,0xE0
 *  00461D17   72 15            JB SHORT .00461D2E
 *  00461D19   3C FC            CMP AL,0xFC
 *  00461D1B   77 11            JA SHORT .00461D2E
 *  00461D1D   66:8B0E          MOV CX,WORD PTR DS:[ESI]
 *  00461D20   B8 02000000      MOV EAX,0x2
 *  00461D25   66:894C24 2C     MOV WORD PTR SS:[ESP+0x2C],CX
 *  00461D2A   03F0             ADD ESI,EAX
 *  00461D2C   EB 72            JMP SHORT .00461DA0
 *  00461D2E   3C 0A            CMP AL,0xA
 *  00461D30   75 2D            JNZ SHORT .00461D5F
 *  00461D32   3B1F             CMP EBX,DWORD PTR DS:[EDI]
 *  00461D34   7E 02            JLE SHORT .00461D38
 *  00461D36   891F             MOV DWORD PTR DS:[EDI],EBX
 *  00461D38   8B45 0C          MOV EAX,DWORD PTR SS:[EBP+0xC]
 *  00461D3B   33DB             XOR EBX,EBX
 *  00461D3D   3BC2             CMP EAX,EDX
 *  00461D3F   7D 02            JGE SHORT .00461D43
 *  00461D41   F7D8             NEG EAX
 *  00461D43   8B4D 58          MOV ECX,DWORD PTR SS:[EBP+0x58]
 *  00461D46   03C8             ADD ECX,EAX
 *  00461D48   8B47 04          MOV EAX,DWORD PTR DS:[EDI+0x4]
 *  00461D4B   03C1             ADD EAX,ECX
 *  00461D4D   46               INC ESI
 *  00461D4E   8947 04          MOV DWORD PTR DS:[EDI+0x4],EAX
 *  00461D51   8B4424 14        MOV EAX,DWORD PTR SS:[ESP+0x14]
 *  00461D55   40               INC EAX
 *  00461D56   894424 14        MOV DWORD PTR SS:[ESP+0x14],EAX
 *  00461D5A   E9 A2000000      JMP .00461E01
 *  00461D5F   3C 40            CMP AL,0x40
 *  00461D61   75 33            JNZ SHORT .00461D96
 *  00461D63   8A46 01          MOV AL,BYTE PTR DS:[ESI+0x1]
 *  00461D66   46               INC ESI
 *  00461D67   3C 40            CMP AL,0x40
 *  00461D69   74 2B            JE SHORT .00461D96
 *  00461D6B   33C9             XOR ECX,ECX
 *  00461D6D   3C 30            CMP AL,0x30
 *  00461D6F   7C 12            JL SHORT .00461D83
 *  00461D71   803C31 39        CMP BYTE PTR DS:[ECX+ESI],0x39
 *  00461D75   7F 0C            JG SHORT .00461D83
 *  00461D77   41               INC ECX
 *  00461D78   83F9 06          CMP ECX,0x6
 *  00461D7B   7F 06            JG SHORT .00461D83
 *  00461D7D   803C31 30        CMP BYTE PTR DS:[ECX+ESI],0x30
 *  00461D81  ^7D EE            JGE SHORT .00461D71
 *  00461D83   8B4424 10        MOV EAX,DWORD PTR SS:[ESP+0x10]
 *  00461D87   03F1             ADD ESI,ECX
 *  00461D89   8B4C24 18        MOV ECX,DWORD PTR SS:[ESP+0x18]
 *  00461D8D   03D9             ADD EBX,ECX
 *  00461D8F   40               INC EAX
 *  00461D90   894424 10        MOV DWORD PTR SS:[ESP+0x10],EAX
 *  00461D94   EB 6B            JMP SHORT .00461E01
 *  00461D96   884424 2C        MOV BYTE PTR SS:[ESP+0x2C],AL
 *  00461D9A   B8 01000000      MOV EAX,0x1
 *  00461D9F   46               INC ESI
 *  00461DA0   8A4C24 28        MOV CL,BYTE PTR SS:[ESP+0x28]
 *  00461DA4   84C9             TEST CL,CL
 *  00461DA6   74 33            JE SHORT .00461DDB
 *  00461DA8   8B8D A0200000    MOV ECX,DWORD PTR SS:[EBP+0x20A0]
 *  00461DAE   8D5424 1C        LEA EDX,DWORD PTR SS:[ESP+0x1C]
 *  00461DB2   52               PUSH EDX
 *  00461DB3   50               PUSH EAX
 *  00461DB4   8D4424 34        LEA EAX,DWORD PTR SS:[ESP+0x34]
 *  00461DB8   50               PUSH EAX
 *  00461DB9   51               PUSH ECX
 *  00461DBA   FF15 24E05100    CALL DWORD PTR DS:[0x51E024]             ; gdi32.GetTextExtentPoint32A
 *  00461DC0   8B4C24 1C        MOV ECX,DWORD PTR SS:[ESP+0x1C]
 *  00461DC4   B8 1F85EB51      MOV EAX,0x51EB851F
 *  00461DC9   F7E9             IMUL ECX
 *  00461DCB   C1FA 04          SAR EDX,0x4
 *  00461DCE   8BC2             MOV EAX,EDX
 *  00461DD0   C1E8 1F          SHR EAX,0x1F
 *  00461DD3   03D0             ADD EDX,EAX
 *  00461DD5   8D4411 01        LEA EAX,DWORD PTR DS:[ECX+EDX+0x1]
 *  00461DD9   EB 19            JMP SHORT .00461DF4
 *  00461DDB   8D4C24 1C        LEA ECX,DWORD PTR SS:[ESP+0x1C]
 *  00461DDF   8D5424 2C        LEA EDX,DWORD PTR SS:[ESP+0x2C]
 *  00461DE3   51               PUSH ECX
 *  00461DE4   50               PUSH EAX
 *  00461DE5   8B45 08          MOV EAX,DWORD PTR SS:[EBP+0x8]
 *  00461DE8   52               PUSH EDX
 *  00461DE9   50               PUSH EAX
 *  00461DEA   FF15 24E05100    CALL DWORD PTR DS:[0x51E024]             ; gdi32.GetTextExtentPoint32A
 *  00461DF0   8B4424 1C        MOV EAX,DWORD PTR SS:[ESP+0x1C]
 *  00461DF4   03D8             ADD EBX,EAX
 *  00461DF6   8B4424 10        MOV EAX,DWORD PTR SS:[ESP+0x10]
 *  00461DFA   40               INC EAX
 *  00461DFB   33D2             XOR EDX,EDX
 *  00461DFD   894424 10        MOV DWORD PTR SS:[ESP+0x10],EAX
 *  00461E01   803E 00          CMP BYTE PTR DS:[ESI],0x0
 *  00461E04  ^0F85 F9FEFFFF    JNZ .00461D03
 *  00461E0A   3B1F             CMP EBX,DWORD PTR DS:[EDI]
 *  00461E0C   7E 02            JLE SHORT .00461E10
 *  00461E0E   891F             MOV DWORD PTR DS:[EDI],EBX
 *  00461E10   8B45 0C          MOV EAX,DWORD PTR SS:[EBP+0xC]
 *  00461E13   3BC2             CMP EAX,EDX
 *  00461E15   7D 02            JGE SHORT .00461E19
 *  00461E17   F7D8             NEG EAX
 *  00461E19   8B4F 04          MOV ECX,DWORD PTR DS:[EDI+0x4]
 *  00461E1C   03C8             ADD ECX,EAX
 *  00461E1E   894F 04          MOV DWORD PTR DS:[EDI+0x4],ECX
 *  00461E21   8A45 20          MOV AL,BYTE PTR SS:[EBP+0x20]
 *  00461E24   84C0             TEST AL,AL
 *  00461E26   74 21            JE SHORT .00461E49
 *  00461E28   8B45 0C          MOV EAX,DWORD PTR SS:[EBP+0xC]
 *  00461E2B   3BC2             CMP EAX,EDX
 *  00461E2D   7D 02            JGE SHORT .00461E31
 *  00461E2F   F7D8             NEG EAX
 *  00461E31   8BC8             MOV ECX,EAX
 *  00461E33   B8 56555555      MOV EAX,0x55555556
 *  00461E38   F7E9             IMUL ECX
 *  00461E3A   8B07             MOV EAX,DWORD PTR DS:[EDI]
 *  00461E3C   8BCA             MOV ECX,EDX
 *  00461E3E   C1E9 1F          SHR ECX,0x1F
 *  00461E41   03D1             ADD EDX,ECX
 *  00461E43   03C2             ADD EAX,EDX
 *  00461E45   33D2             XOR EDX,EDX
 *  00461E47   8907             MOV DWORD PTR DS:[EDI],EAX
 *  00461E49   8B4C24 30        MOV ECX,DWORD PTR SS:[ESP+0x30]
 *  00461E4D   3BCA             CMP ECX,EDX
 *  00461E4F   74 02            JE SHORT .00461E53
 *  00461E51   8911             MOV DWORD PTR DS:[ECX],EDX
 *  00461E53   8B5424 34        MOV EDX,DWORD PTR SS:[ESP+0x34]
 *  00461E57   85D2             TEST EDX,EDX
 *  00461E59   74 06            JE SHORT .00461E61
 *  00461E5B   C702 00000000    MOV DWORD PTR DS:[EDX],0x0
 *  00461E61   8A4424 38        MOV AL,BYTE PTR SS:[ESP+0x38]
 *  00461E65   84C0             TEST AL,AL
 *  00461E67   0F84 81000000    JE .00461EEE
 *  00461E6D   8A45 4C          MOV AL,BYTE PTR SS:[EBP+0x4C]
 *  00461E70   3C 01            CMP AL,0x1
 *  00461E72   75 3C            JNZ SHORT .00461EB0
 *  00461E74   8B45 50          MOV EAX,DWORD PTR SS:[EBP+0x50]
 *  00461E77   85C0             TEST EAX,EAX
 *  00461E79   7D 02            JGE SHORT .00461E7D
 *  00461E7B   F7D8             NEG EAX
 *  00461E7D   8B1F             MOV EBX,DWORD PTR DS:[EDI]
 *  00461E7F   03D8             ADD EBX,EAX
 *  00461E81   891F             MOV DWORD PTR DS:[EDI],EBX
 *  00461E83   8B45 54          MOV EAX,DWORD PTR SS:[EBP+0x54]
 *  00461E86   85C0             TEST EAX,EAX
 *  00461E88   7D 02            JGE SHORT .00461E8C
 *  00461E8A   F7D8             NEG EAX
 *  00461E8C   8B77 04          MOV ESI,DWORD PTR DS:[EDI+0x4]
 *  00461E8F   03F0             ADD ESI,EAX
 *  00461E91   85C9             TEST ECX,ECX
 *  00461E93   8977 04          MOV DWORD PTR DS:[EDI+0x4],ESI
 *  00461E96   74 09            JE SHORT .00461EA1
 *  00461E98   8B45 50          MOV EAX,DWORD PTR SS:[EBP+0x50]
 *  00461E9B   85C0             TEST EAX,EAX
 *  00461E9D   7D 02            JGE SHORT .00461EA1
 *  00461E9F   8901             MOV DWORD PTR DS:[ECX],EAX
 *  00461EA1   85D2             TEST EDX,EDX
 *  00461EA3   74 49            JE SHORT .00461EEE
 *  00461EA5   8B45 54          MOV EAX,DWORD PTR SS:[EBP+0x54]
 *  00461EA8   85C0             TEST EAX,EAX
 *  00461EAA   7D 42            JGE SHORT .00461EEE
 *  00461EAC   8902             MOV DWORD PTR DS:[EDX],EAX
 *  00461EAE   EB 3E            JMP SHORT .00461EEE
 *  00461EB0   3C 02            CMP AL,0x2
 *  00461EB2   75 3A            JNZ SHORT .00461EEE
 *  00461EB4   8B85 A8200000    MOV EAX,DWORD PTR SS:[EBP+0x20A8]
 *  00461EBA   8B37             MOV ESI,DWORD PTR DS:[EDI]
 *  00461EBC   D1E0             SHL EAX,1
 *  00461EBE   03F0             ADD ESI,EAX
 *  00461EC0   8937             MOV DWORD PTR DS:[EDI],ESI
 *  00461EC2   8B85 A8200000    MOV EAX,DWORD PTR SS:[EBP+0x20A8]
 *  00461EC8   8B77 04          MOV ESI,DWORD PTR DS:[EDI+0x4]
 *  00461ECB   D1E0             SHL EAX,1
 *  00461ECD   03F0             ADD ESI,EAX
 *  00461ECF   85C9             TEST ECX,ECX
 *  00461ED1   8977 04          MOV DWORD PTR DS:[EDI+0x4],ESI
 *  00461ED4   74 0A            JE SHORT .00461EE0
 *  00461ED6   8B85 A8200000    MOV EAX,DWORD PTR SS:[EBP+0x20A8]
 *  00461EDC   F7D8             NEG EAX
 *  00461EDE   8901             MOV DWORD PTR DS:[ECX],EAX
 *  00461EE0   85D2             TEST EDX,EDX
 *  00461EE2   74 0A            JE SHORT .00461EEE
 *  00461EE4   8B8D A8200000    MOV ECX,DWORD PTR SS:[EBP+0x20A8]
 *  00461EEA   F7D9             NEG ECX
 *  00461EEC   890A             MOV DWORD PTR DS:[EDX],ECX
 *  00461EEE   8B4424 3C        MOV EAX,DWORD PTR SS:[ESP+0x3C]
 *  00461EF2   5F               POP EDI
 *  00461EF3   85C0             TEST EAX,EAX
 *  00461EF5   5B               POP EBX
 *  00461EF6   74 06            JE SHORT .00461EFE
 *  00461EF8   8B5424 08        MOV EDX,DWORD PTR SS:[ESP+0x8]
 *  00461EFC   8910             MOV DWORD PTR DS:[EAX],EDX
 *  00461EFE   8B4424 0C        MOV EAX,DWORD PTR SS:[ESP+0xC]
 *  00461F02   5E               POP ESI
 *  00461F03   5D               POP EBP
 *  00461F04   83C4 14          ADD ESP,0x14
 *  00461F07   C2 1800          RETN 0x18
 *  00461F0A   5E               POP ESI
 *  00461F0B   33C0             XOR EAX,EAX
 *  00461F0D   5D               POP EBP
 *  00461F0E   83C4 14          ADD ESP,0x14
 *  00461F11   C2 1800          RETN 0x18
 *  00461F14   90               NOP
 *  00461F15   90               NOP
 *  00461F16   90               NOP
 *  00461F17   90               NOP
 *  00461F18   90               NOP
 *  00461F19   90               NOP
 *  00461F1A   90               NOP
 *  00461F1B   90               NOP
 *  00461F1C   90               NOP
 *  00461F1D   90               NOP
 */
bool attach(ulong startAddress, ulong stopAddress) // attach scenario
{
  const uint8_t bytes[] = {
    0x83,0xec, 0x14,                // 00461ca0   83ec 14          sub esp,0x14
    0x33,0xd2,                      // 00461ca3   33d2             xor edx,edx
    0x55,                           // 00461ca5   55               push ebp
    0x56,                           // 00461ca6   56               push esi
    0x8b,0x74,0x24, 0x20,           // 00461ca7   8b7424 20        mov esi,dword ptr ss:[esp+0x20]
    0x8b,0xe9,                      // 00461cab   8be9             mov ebp,ecx
    0x3b,0xf2,                      // 00461cad   3bf2             cmp esi,edx
    0x0f,0x84, 0x55,0x02,0x00,0x00, // 00461caf   0f84 55020000    je .00461f0a
    0x39,0x55, 0x08,                // 00461cb5   3955 08          cmp dword ptr ss:[ebp+0x8],edx
    0x0f,0x84, 0x4c,0x02,0x00,0x00, // 00461cb8   0f84 4c020000    je .00461f0a
    0x8b,0x85, 0x74,0x20,0x00,0x00  // 00461cbe   8b85 74200000    mov eax,dword ptr ss:[ebp+0x2074]
  };
  ulong addr = MemDbg::findBytes(bytes, sizeof(bytes), startAddress, stopAddress);
  return addr && winhook::hook_before(addr, Private::hookBefore);
}
} // namespace OtherHook
} // unnamed namespace

/** Public class */

bool LucifenEngine::attach()
{
  ulong startAddress, stopAddress;
  if (!Engine::getProcessMemoryRange(&startAddress, &stopAddress))
    return false;
  if (!ScenarioHook::attach(startAddress, stopAddress))
    return false;

  if (ChoiceHook::attach(startAddress, stopAddress))
    DOUT("choice text found");
  else
    DOUT("choice text NOT FOUND");

  if (OtherHook::attach(startAddress, stopAddress))
    DOUT("other text found");
  else
    DOUT("other text NOT FOUND");

  auto h = HijackManager::instance();
  h->attachFunction((ulong)::GetGlyphOutlineA);
  h->attachFunction((ulong)::GetTextExtentPoint32A);
  //h->attachFunction((ulong)::CreateFontA);
  //h->attachFunction((ulong)::CreateFontIndirectA);
  return true;
}

// EOF

