// unicorn.cc
// 6/22/2015 jichi
#include "engine/model/unicorn.h"
#include "engine/enginecontroller.h"
#include "engine/enginedef.h"
#include "engine/engineutil.h"
#include "hijack/hijackmanager.h"
#include "util/textutil.h"
#include "winhook/hookcode.h"
#include "memdbg/memsearch.h"
#include <qt_windows.h>
#include <QtCore/QHash>
#include <cstdint>

#define DEBUG "unicorn"
#include "sakurakit/skdebug.h"

#pragma intrinsic(_ReturnAddress)

namespace { // unnamed
namespace ScenarioHook {
namespace Private {

  class TextStorage
  {
    LPSTR text_;
    QByteArray oldData_,
               newData_;
    int lineCount_;
    bool saved_;
  public:
    TextStorage()
      : text_(nullptr), lineCount_(0), saved_(false) {}

    bool isEmpty() const
    { return lineCount_ == 0; }

    void clear()
    {
      text_ = nullptr;
      lineCount_ = 0;
      saved_ = false;
      oldData_.clear();
      newData_.clear();
    }

    QByteArray load(char *textAddress);
    void save();
    bool restore(); // recover old text
  } textStorage_;

  // Hook

  ulong textOffset_; // = 0x114;

  QByteArray sourceData_;
  LPSTR targetText_;
  bool hookBefore(winhook::hook_stack *s)
  {
    if (!textStorage_.isEmpty()) {
      textStorage_.restore();
      textStorage_.clear();
    }

    // Sample game:  三極姫4 ～天華繚乱 天命の恋絵巻～
    // 004B76BB   51               PUSH ECX
    // 004B76BC   8BCB             MOV ECX,EBX
    // 004B76BE   894424 14        MOV DWORD PTR SS:[ESP+0x14],EAX
    // 004B76C2   E8 89A5FFFF      CALL Sangokuh.004B1C50	; jichi: name caller
    // 004B76C7   E8 44A5FFFF      CALL Sangokuh.004B1C10
    // 004B76CC   85C0             TEST EAX,EAX
    // 004B76CE   0F8E F6000000    JLE Sangokuh.004B77CA
    // 004B76D4   8BF8             MOV EDI,EAX
    // 004B76D6   EB 08            JMP SHORT Sangokuh.004B76E0
    // 004B76D8   8DA424 00000000  LEA ESP,DWORD PTR SS:[ESP]
    // 004B76DF   90               NOP
    // 004B76E0   33C0             XOR EAX,EAX
    // 004B76E2   B9 0F000000      MOV ECX,0xF
    // 004B76E7   898C24 FC000000  MOV DWORD PTR SS:[ESP+0xFC],ECX
    // 004B76EE   898424 F8000000  MOV DWORD PTR SS:[ESP+0xF8],EAX
    // 004B76F5   888424 E8000000  MOV BYTE PTR SS:[ESP+0xE8],AL
    // 004B76FC   898C24 18010000  MOV DWORD PTR SS:[ESP+0x118],ECX
    // 004B7703   898424 14010000  MOV DWORD PTR SS:[ESP+0x114],EAX
    // 004B770A   888424 04010000  MOV BYTE PTR SS:[ESP+0x104],AL
    // 004B7711   8D9424 84040000  LEA EDX,DWORD PTR SS:[ESP+0x484]
    // 004B7718   52               PUSH EDX
    // 004B7719   8BCB             MOV ECX,EBX
    // 004B771B   C68424 AC060000 01 MOV BYTE PTR SS:[ESP+0x6AC],0x1
    // 004B7723   E8 28A5FFFF      CALL Sangokuh.004B1C50	; jichi: scenario caller
    // 004B7728   8D8424 84040000  LEA EAX,DWORD PTR SS:[ESP+0x484]
    // 004B772F   50               PUSH EAX
    // 004B7730   8D8C24 E8000000  LEA ECX,DWORD PTR SS:[ESP+0xE8]
    //
    // Sample game: 天極姫 ～新世大乱･双界の覇者達～
    // Name caller:
    // 0049A83B   E8 D0AFFFFF      CALL .00495810
    // 0049A840   894424 14        MOV DWORD PTR SS:[ESP+0x14],EAX
    // 0049A844   8D8424 EC010000  LEA EAX,DWORD PTR SS:[ESP+0x1EC]
    // 0049A84B   50               PUSH EAX
    // 0049A84C   E8 DFAFFFFF      CALL .00495830 ; jichi: name caller
    // 0049A851   E8 9AAFFFFF      CALL .004957F0
    // 0049A856   BD 0F000000      MOV EBP,0xF
    // 0049A85B   85C0             TEST EAX,EAX
    // 0049A85D   0F8E E3000000    JLE .0049A946

    auto retaddr = s->stack[0];
    auto role = Engine::OtherRole;
    //if (retaddr == 0x4b7728)
    if ((*(DWORD *)(retaddr - 5 - 8) & 0x00ffffff) == 0x2484c6) // 004B771B   C68424 AC060000 01 MOV BYTE PTR SS:[ESP+0x6AC],0x1
      role = Engine::ScenarioRole;
    //else if (retaddr == 0x4b76c7)
    else if ((*(DWORD *)(retaddr - 5 - 8) & 0x00ffffff) == 0x0024848d   // 0049A844   8D8424 EC010000  LEA EAX,DWORD PTR SS:[ESP+0x1EC]
          || (*(DWORD *)(retaddr - 5 - 4) & 0x00ffffff) == 0x00244489)  // 004B76BE   894424 14        MOV DWORD PTR SS:[ESP+0x14],EAX
      role = Engine::NameRole;
    else
      return true;

    auto text = (LPSTR)*(DWORD *)(s->ecx + textOffset_); // [ecx+0x114]
    if (!*text || Util::allAscii(text)) // allspaces is only needed when textstorage is enabled though
      return true;

    bool textStorageEnabled = role == Engine::ScenarioRole && Engine::isAddressWritable(text);
    QByteArray oldData;
    if (textStorageEnabled)
      oldData = textStorage_.load(text);
    else
      oldData = text;

    if (role == Engine::NameRole)
      oldData.replace("\x81\x40", ""); // remove spaces in the middle of names

    auto sig = retaddr;
    QByteArray newData = EngineController::instance()->dispatchTextA(oldData, role, sig);
    if (oldData == newData) {
      if (textStorageEnabled)
        textStorage_.clear();
      return true;
    }
    if (textStorageEnabled)
      textStorage_.save();

    //newData.replace(UNICORN_BR, "\n");

    sourceData_ = newData;
    targetText_ = (LPSTR)s->stack[1]; // arg1
    return true;
  }
  bool hookAfter(winhook::hook_stack *)
  {
    if (targetText_) {
      ::strcpy(targetText_, sourceData_.constData());
      targetText_ = nullptr;
    }
    return true;
  }

} // namespace Private

/**
 *  Sample text
 *
 *  Sample game:  三極姫4 ～天華繚乱 天命の恋絵巻～
 *
 *  01FE881C  81 40 92 6A 81 40 00 01 81 75 82 BB 81 41 82 BB  　男　.「そ、そ
 *  01FE882C  82 F1 82 C8 81 63 81 63 82 BB 82 EA 82 AA 8D C5  んな……それが最
 *  01FE883C  8C E3 82 CC 90 48 97 BF 82 C8 82 CC 82 C9 81 63  後の食料なのに…
 *  01FE884C  81 63 81 49 81 76 00 00 00 00 FF FF FF FF FF FF  …！」....
 *  01FE885C  FF FF 11 19 00 1B 00 0F 19 00 1D 00 03 00 00 00  .......
 *  01FE886C  03 00 00 00 00 01 97 AA 92 44 81 5C 81 5C 00 00  ....略奪――..
 *
 *  01FE8758  01 00 00 00 01 00 00 00 93 90 81 40 91 AF 00 02  ......盗　賊.
 *  01FE8768  81 75 82 C7 82 A4 82 B9 82 B1 82 EA 82 C1 82 DB  「どうせこれっぽ
 *  01FE8778  82 C1 82 BF 82 CC 90 48 97 BF 82 AA 82 A0 82 C1  っちの食料があっ
 *  01FE8788  82 BD 82 C6 82 B1 82 EB 82 C5 81 41 8B 51 82 A6  たところで、飢え
 *  01FE8798  82 C4 8E 80 00 00 00 00 FF FF FF FF FF FF FF FF  て死....
 *  01FE87A8  0A 82 CA 82 CC 82 CD 93 AF 82 B6 82 BE 82 EB 81  .ぬのは同じだろ・
 *  01FE87B8  49 81 40 82 D9 82 E7 91 53 95 94 82 E6 82 B1 82  I　ほら全部よこ・
 *  01FE87C8  B9 82 C1 81 49 81 76 00 00 00 00 FF FF FF FF FF  ｹっ！」....
 *  01FE87D8  FF FF FF 11 19 00 16 00 19 19 00 18 00 32 00 00  ....2..
 *  01FE87E8  00 44 61 74 61 5C 76 6F 69 63 65 5C 65 74 63 5C  .Data\voice\etc\
 *  01FE87F8  65 74 63 4A 5F 70 63 41 5F 30 30 30 31 2E 76 6F  etcJ_pcA_0001.vo
 *  01FE8808  69 00 00 00 00 00 00 0F 19 00 19 00 02 00 00 00  i...........
 *
 *  Sample game: 戦極姫6
 *
 *  023AF0E8  82 BB 82 CC 90 BA 82 F0 95 B7 82 AB 81 41 90 B0  その声を聞き、晴
 *  023AF0F8  90 4D 82 CD 82 B7 82 C1 82 C6 95 5C 8F EE 82 F0  信はすっと表情を
 *  023AF108  88 F8 82 AB 92 F7 82 DF 82 BD 81 42 00 00 00 00  引き締めた。....
 *  023AF118  BE BE BE FF FF FF FF FF 11 0E 00 1E 00 0F 0E 00  ｾｾｾ...
 *  023AF128  20 00 03 00 00 00 03 00 00 00 95 90 93 63 90 4D   .......武田信
 *  023AF138  94 C9 00 01 81 75 90 4D 8C D5 97 6C 82 CD 81 41  繁.「信虎様は、
 *  023AF148  97 5C 92 E8 82 C7 82 A8 82 E8 82 BE 82 BB 82 A4  予定どおりだそう
 *  023AF158  82 BE 81 76 00 00 00 00 BE BE BE FF FF FF FF FF  だ」....ｾｾｾ
 *  023AF168  11 0E 00 22 00 0F 0E 00 24 00 04 00 00 00 04 00  ."..$.....
 *  023AF178  00 00 00 02 95 94 89 AE 82 C9 82 CD 82 A2 82 C1  ...部屋にはいっ
 *  023AF188  82 C4 82 AB 82 BD 90 4D 94 C9 82 CD 81 41 90 B0  てきた信繁は、晴
 *  023AF198  90 4D 82 CC 91 4F 82 D6 82 C6 8D 98 82 F0 82 A8  信の前へと腰をお
 *  023AF1A8  82 EB 82 B5 8C FC 82 A9 00 00 00 00 BE BE BE FF  ろし向か....ｾｾｾ
 *  023AF1B8  FF FF FF FF 0A 82 A2 82 A0 82 A4 81 42 00 00 00  .いあう。...
 *  023AF1C8  00 BE BE BE FF FF FF FF FF 11 0E 00 27 00 01 0E  .ｾｾｾ.'.
 *  023AF1D8  00 2A 00 84 D9 07 00 02 00 00 00 E8 18 00 00 01  .*.・....・..
 *  023AF1E8  60 00 00 00 E9 18 00 00 01 5B 00 00 00 19 0E 00  `...・..[....
 *  023AF1F8  2C 00 06 00 00 00 44 61 74 61 5C 76 6F 69 63 65  ,....Data\voice
 *  023AF208  5C 73 69 6E 67 65 6E 5C 73 69 6E 67 65 6E 5F 30  \singen\singen_0
 *  023AF218  30 34 33 2E 76 6F 69 00 00 00 00 00 00 0F 0E 00  043.voi.......
 *
 *  Sample game: 天極姫 ～新世大乱･双界の覇者達～
 *  0211F8AA  82 91 80 82 BD 82 BF 82 CD 82 B1 82 CC 90 A2 8A  ｑたちはこの世・
 *  0211F8BA  45 82 C9 93 CB 91 52 8C BB 82 EA 82 BD 81 42 82  Eに突然現れた。・
 *  0211F8CA  BB 82 B5 82 C4 82 B1 82 B1 82 CC 96 AF 82 BD 82  ｻしてここの民た・
 *  0211F8DA  BF 82 CD 00 00 00 00 BE BE BE FF FF FF FF FF 0A  ｿは....ｾｾｾ.
 *  0211F8EA  91 82 91 80 82 BD 82 BF 82 F0 81 41 92 B7 82 AD  曹操たちを、長く
 *  0211F8FA  91 B1 82 A2 82 BD 90 ED 97 90 82 F0 8F 49 82 ED  続いた戦乱を終わ
 *  0211F90A  82 E7 82 B9 82 E9 89 70 97 59 82 C6 81 41 96 7B  らせる英雄と、本
 *  0211F91A  8B 43 82 C5 00 00 00 00 BE BE BE FF FF FF FF FF  気で....ｾｾｾ
 *  0211F92A  0A 90 4D 82 B6 82 C4 82 A2 82 E9 82 C6 82 A2 82  .信じているとい・
 *  0211F93A  A4 82 B1 82 C6 82 BE 82 C1 82 BD 81 42 00 00 00  ､ことだった。...
 */
// 三極姫4: 00 00 00 00 ff ff ff ff ff ff ff ff 0a
// 戦極姫6: 00 00 00 00 be be be ff ff ff ff ff 0a
static inline bool isTextSeparator(LPCSTR text)
{
  //return 0 == ::memcmp(p, "\x00\x00\x00\x00\xff\xff\xff\xff\xff\xff\xff\xff\x0a", 13);
  return 0 == ::memcmp(text, "\x00\x00\x00\x00", 4)
      && 0 == ::memcmp(text + 8, "\xff\xff\xff\xff\x0a", 5);
}
QByteArray Private::TextStorage::load(char *text)
{
  text_ = text;
  QByteArray data = text;
  lineCount_ = 1;
  LPCSTR p = text + ::strlen(text);
  for (; isTextSeparator(p); p += ::strlen(p)) {
    lineCount_++;
    p += 12;
    data.append(p);
  }
  oldData_ = QByteArray(text, p - text);
  return data;
}

void Private::TextStorage::save()
{
  if (lineCount_ <= 1)
    return;
  LPSTR p = text_ + ::strlen(text_);
  while (isTextSeparator(p)) {
    p += 13;
    if (size_t size = ::strlen(p)) {
      ::memset(p, ' ', size);
      p += size;
    }
  }
  newData_ = QByteArray(text_, p - text_);
}

bool Private::TextStorage::restore()
{
  if (!saved_
      || !Engine::isAddressWritable(text_, oldData_.size())
      || ::memcmp(text_, newData_.constData(), newData_.size()))
    return false;
  if (::memcmp(text_, oldData_.constData(), oldData_.size()))
    ::memcpy(text_, oldData_.constData(), oldData_.size());
  saved_ = false;
  return true;
}

/**
 *  Sample game:  三極姫4 ～天華繚乱 天命の恋絵巻～
 *
 *  Function found by hardware breakpoint scenario text.
 *
 *  The memory copy function:
 *  004B1C4D   CC               INT3
 *  004B1C4E   CC               INT3
 *  004B1C4F   CC               INT3
 *  004B1C50   8B81 14010000    MOV EAX,DWORD PTR DS:[ECX+0x114]	; jichi: source text in eax, beforeAddress
 *  004B1C56   8B5424 04        MOV EDX,DWORD PTR SS:[ESP+0x4]		; jichi: target address in edx
 *  004B1C5A   56               PUSH ESI
 *  004B1C5B   33F6             XOR ESI,ESI
 *  004B1C5D   8038 00          CMP BYTE PTR DS:[EAX],0x0
 *  004B1C60   74 1D            JE SHORT Sangokuh.004B1C7F
 *  004B1C62   8B81 14010000    MOV EAX,DWORD PTR DS:[ECX+0x114]
 *  004B1C68   8A00             MOV AL,BYTE PTR DS:[EAX]
 *  004B1C6A   8802             MOV BYTE PTR DS:[EDX],AL
 *  004B1C6C   FF81 14010000    INC DWORD PTR DS:[ECX+0x114]
 *  004B1C72   8B81 14010000    MOV EAX,DWORD PTR DS:[ECX+0x114]
 *  004B1C78   42               INC EDX
 *  004B1C79   46               INC ESI
 *  004B1C7A   8038 00          CMP BYTE PTR DS:[EAX],0x0
 *  004B1C7D  ^75 E3            JNZ SHORT Sangokuh.004B1C62
 *  004B1C7F   8B81 14010000    MOV EAX,DWORD PTR DS:[ECX+0x114]
 *  004B1C85   8A00             MOV AL,BYTE PTR DS:[EAX]
 *  004B1C87   8802             MOV BYTE PTR DS:[EDX],AL
 *  004B1C89   FF81 14010000    INC DWORD PTR DS:[ECX+0x114]
 *  004B1C8F   8BC6             MOV EAX,ESI ; jichi: copied count
 *  004B1C91   5E               POP ESI
 *  004B1C92   C2 0400          RETN 0x4 ; jichi: afterAddress
 *  004B1C95   CC               INT3
 *  004B1C96   CC               INT3
 *  004B1C97   CC               INT3
 *
 *  The very large caller function:
 *
 *  004B76AB   894424 1C        MOV DWORD PTR SS:[ESP+0x1C],EAX
 *  004B76AF   E8 7CA5FFFF      CALL Sangokuh.004B1C30
 *  004B76B4   8D8C24 7C030000  LEA ECX,DWORD PTR SS:[ESP+0x37C]
 *  004B76BB   51               PUSH ECX
 *  004B76BC   8BCB             MOV ECX,EBX
 *  004B76BE   894424 14        MOV DWORD PTR SS:[ESP+0x14],EAX
 *  004B76C2   E8 89A5FFFF      CALL Sangokuh.004B1C50	; jichi: name caller
 *  004B76C7   E8 44A5FFFF      CALL Sangokuh.004B1C10
 *  004B76CC   85C0             TEST EAX,EAX
 *  004B76CE   0F8E F6000000    JLE Sangokuh.004B77CA
 *  004B76D4   8BF8             MOV EDI,EAX
 *  004B76D6   EB 08            JMP SHORT Sangokuh.004B76E0
 *  004B76D8   8DA424 00000000  LEA ESP,DWORD PTR SS:[ESP]
 *  004B76DF   90               NOP
 *  004B76E0   33C0             XOR EAX,EAX
 *  004B76E2   B9 0F000000      MOV ECX,0xF
 *  004B76E7   898C24 FC000000  MOV DWORD PTR SS:[ESP+0xFC],ECX
 *  004B76EE   898424 F8000000  MOV DWORD PTR SS:[ESP+0xF8],EAX
 *  004B76F5   888424 E8000000  MOV BYTE PTR SS:[ESP+0xE8],AL
 *  004B76FC   898C24 18010000  MOV DWORD PTR SS:[ESP+0x118],ECX
 *  004B7703   898424 14010000  MOV DWORD PTR SS:[ESP+0x114],EAX
 *  004B770A   888424 04010000  MOV BYTE PTR SS:[ESP+0x104],AL
 *  004B7711   8D9424 84040000  LEA EDX,DWORD PTR SS:[ESP+0x484]
 *  004B7718   52               PUSH EDX
 *  004B7719   8BCB             MOV ECX,EBX
 *  004B771B   C68424 AC060000 01 MOV BYTE PTR SS:[ESP+0x6AC],0x1
 *  004B7723   E8 28A5FFFF      CALL Sangokuh.004B1C50	; jichi: scenario caller
 *  004B7728   8D8424 84040000  LEA EAX,DWORD PTR SS:[ESP+0x484]
 *  004B772F   50               PUSH EAX
 *  004B7730   8D8C24 E8000000  LEA ECX,DWORD PTR SS:[ESP+0xE8]
 *
 *  Sample game: 戦極姫6
 *  004A6C88   CC               INT3
 *  004A6C89   CC               INT3
 *  004A6C8A   CC               INT3
 *  004A6C8B   CC               INT3
 *  004A6C8C   CC               INT3
 *  004A6C8D   CC               INT3
 *  004A6C8E   CC               INT3
 *  004A6C8F   CC               INT3
 *  004A6C90   8B81 14010000    MOV EAX,DWORD PTR DS:[ECX+0x114]
 *  004A6C96   8B5424 04        MOV EDX,DWORD PTR SS:[ESP+0x4]
 *  004A6C9A   56               PUSH ESI
 *  004A6C9B   33F6             XOR ESI,ESI
 *  004A6C9D   8038 00          CMP BYTE PTR DS:[EAX],0x0
 *  004A6CA0   74 1D            JE SHORT .004A6CBF
 *  004A6CA2   8B81 14010000    MOV EAX,DWORD PTR DS:[ECX+0x114]
 *  004A6CA8   8A00             MOV AL,BYTE PTR DS:[EAX]
 *  004A6CAA   8802             MOV BYTE PTR DS:[EDX],AL
 *  004A6CAC   FF81 14010000    INC DWORD PTR DS:[ECX+0x114]
 *  004A6CB2   8B81 14010000    MOV EAX,DWORD PTR DS:[ECX+0x114]
 *  004A6CB8   42               INC EDX
 *  004A6CB9   46               INC ESI
 *  004A6CBA   8038 00          CMP BYTE PTR DS:[EAX],0x0
 *  004A6CBD  ^75 E3            JNZ SHORT .004A6CA2
 *  004A6CBF   8B81 14010000    MOV EAX,DWORD PTR DS:[ECX+0x114]
 *  004A6CC5   8A00             MOV AL,BYTE PTR DS:[EAX]
 *  004A6CC7   8802             MOV BYTE PTR DS:[EDX],AL
 *  004A6CC9   FF81 14010000    INC DWORD PTR DS:[ECX+0x114]
 *  004A6CCF   8BC6             MOV EAX,ESI
 *  004A6CD1   5E               POP ESI
 *  004A6CD2   C2 0400          RETN 0x4
 *  004A6CD5   CC               INT3
 *  004A6CD6   CC               INT3
 *  004A6CD7   CC               INT3
 *  004A6CD8   CC               INT3
 *  004A6CD9   CC               INT3
 */
bool attach(ulong startAddress, ulong stopAddress)
{
  ulong beforeAddress;
  {
    const uint8_t bytes[] = {
      0x8b,0x81, XX4,         // 004b1c50   8b81 14010000    mov eax,dword ptr ds:[ecx+0x114]	; jichi: source text in eax
      0x8b,0x54,0x24, 0x04,   // 004b1c56   8b5424 04        mov edx,dword ptr ss:[esp+0x4]		; jichi: target address in edx
      0x56,                   // 004b1c5a   56               push esi
      0x33,0xf6,              // 004b1c5b   33f6             xor esi,esi
      0x80,0x38, 0x00         // 004b1c5d   8038 00          cmp byte ptr ds:[eax],0x0
    };
    beforeAddress = MemDbg::matchBytes(bytes, sizeof(bytes), startAddress, stopAddress);
    if (!beforeAddress)
      return true;
  }

  ulong afterAddress;
  {
    // 004B1C92   C2 0400          RETN 0x4 ; jichi: afterAddress
    // 004B1C95   CC               INT3
    DWORD bytes = 0xcc0004c2;
    afterAddress = MemDbg::findBytes(&bytes, sizeof(bytes), beforeAddress, stopAddress);
    if (!afterAddress || afterAddress - beforeAddress > 0x200) // should within 0x42
      return true;
  }

  // 004b1c50   8b81 14010000    mov eax,dword ptr ds:[ecx+0x114]	; jichi: source text in eax
  Private::textOffset_ = *(DWORD *)(beforeAddress + 2); // 0x114

  return winhook::hook_before(beforeAddress, Private::hookBefore)
      && winhook::hook_before(afterAddress, Private::hookAfter);
}

} // namespace ScenarioHook
} // unnamed namespace

bool UnicornEngine::attach()
{
  ulong startAddress, stopAddress;
  if (!Engine::getProcessMemoryRange(&startAddress, &stopAddress))
    return false;
  if (!ScenarioHook::attach(startAddress, stopAddress))
    return false;
  HijackManager::instance()->attachFunction((ulong)::GetGlyphOutlineA);
  return true;
}

// EOF
