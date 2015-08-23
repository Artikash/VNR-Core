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

namespace OtherHook {
namespace Private {

  bool isSkippedText(LPCSTR text)
  {
    return 0 == ::strcmp(text, "\x82\x6c\x82\x72\x20\x83\x53\x83\x56\x83\x62\x83\x4e"); // "ＭＳ ゴシック"
  }

  /**
   *  Sample game:  戦極姫6
   *
   *  Scenario caller:
   *
   *  0052FDC0   72 11            JB SHORT .0052FDD3
   *  0052FDC2   56               PUSH ESI
   *  0052FDC3   FF75 10          PUSH DWORD PTR SS:[EBP+0x10]
   *  0052FDC6   FF75 08          PUSH DWORD PTR SS:[EBP+0x8]
   *  0052FDC9   E8 12F5FFFF      CALL .0052F2E0    ; jichi: scenario called
   *  0052FDCE   83C4 0C          ADD ESP,0xC
   *  0052FDD1  ^EB C1            JMP SHORT .0052FD94
   *  0052FDD3   FF75 0C          PUSH DWORD PTR SS:[EBP+0xC]
   *  0052FDD6   57               PUSH EDI
   *
   *  08C4E39C   08962521
   *  08C4E3A0   0895C921  ; jichi: source text
   *  08C4E3A4   00000D29
   *  08C4E3A8   00000D29
   *  08C4E3AC   005A24A0  .005A24A0
   *  08C4E3B0  /08C4E3E4
   *  08C4E3B4  |00532435  RETURN to .00532435 from .0052FD84
   *  08C4E3B8  |08962521
   *  08C4E3BC  |FFFFFFFE
   *  08C4E3C0  |0895C921  ; jichi: source text
   *  08C4E3C4  |00000D29
   *  08C4E3C8  |000004B2
   *  08C4E3CC  |00000000   ; jichi: this value is always 0
   *  08C4E3D0  |0896A920  ASCII "SFP"
   *  08C4E3D4  |00000D2A
   *  08C4E3D8  |00001000
   *
   *  0052FD7C   59               POP ECX
   *  0052FD7D   8BC6             MOV EAX,ESI
   *  0052FD7F   5E               POP ESI
   *  0052FD80   5D               POP EBP
   *  0052FD81   C2 0400          RETN 0x4
   *
   *  0052FD84   8BFF             MOV EDI,EDI   ; jichi: caller of the encodable text
   *  0052FD86   55               PUSH EBP
   *  0052FD87   8BEC             MOV EBP,ESP
   *  0052FD89   56               PUSH ESI
   *  0052FD8A   8B75 14          MOV ESI,DWORD PTR SS:[EBP+0x14]
   *  0052FD8D   57               PUSH EDI
   *  0052FD8E   33FF             XOR EDI,EDI
   *  0052FD90   3BF7             CMP ESI,EDI
   *  0052FD92   75 04            JNZ SHORT .0052FD98
   *  0052FD94   33C0             XOR EAX,EAX
   *  0052FD96   EB 65            JMP SHORT .0052FDFD
   *  0052FD98   397D 08          CMP DWORD PTR SS:[EBP+0x8],EDI
   *  0052FD9B   75 1B            JNZ SHORT .0052FDB8
   *  0052FD9D   E8 46370000      CALL .005334E8
   *  0052FDA2   6A 16            PUSH 0x16
   *  0052FDA4   5E               POP ESI
   *  0052FDA5   8930             MOV DWORD PTR DS:[EAX],ESI
   *  0052FDA7   57               PUSH EDI
   *  0052FDA8   57               PUSH EDI
   *  0052FDA9   57               PUSH EDI
   *  0052FDAA   57               PUSH EDI
   *  0052FDAB   57               PUSH EDI
   *  0052FDAC   E8 E4010000      CALL .0052FF95
   *  0052FDB1   83C4 14          ADD ESP,0x14
   *  0052FDB4   8BC6             MOV EAX,ESI
   *  0052FDB6   EB 45            JMP SHORT .0052FDFD
   *  0052FDB8   397D 10          CMP DWORD PTR SS:[EBP+0x10],EDI
   *  0052FDBB   74 16            JE SHORT .0052FDD3
   *  0052FDBD   3975 0C          CMP DWORD PTR SS:[EBP+0xC],ESI
   *  0052FDC0   72 11            JB SHORT .0052FDD3
   *  0052FDC2   56               PUSH ESI
   *  0052FDC3   FF75 10          PUSH DWORD PTR SS:[EBP+0x10]
   *  0052FDC6   FF75 08          PUSH DWORD PTR SS:[EBP+0x8]
   *  0052FDC9   E8 12F5FFFF      CALL .0052F2E0
   *  0052FDCE   83C4 0C          ADD ESP,0xC
   *  0052FDD1  ^EB C1            JMP SHORT .0052FD94
   *  0052FDD3   FF75 0C          PUSH DWORD PTR SS:[EBP+0xC]
   *  0052FDD6   57               PUSH EDI
   *  0052FDD7   FF75 08          PUSH DWORD PTR SS:[EBP+0x8]
   *  0052FDDA   E8 81070000      CALL .00530560
   *  0052FDDF   83C4 0C          ADD ESP,0xC
   *  0052FDE2   397D 10          CMP DWORD PTR SS:[EBP+0x10],EDI
   *  0052FDE5  ^74 B6            JE SHORT .0052FD9D
   *  0052FDE7   3975 0C          CMP DWORD PTR SS:[EBP+0xC],ESI
   *  0052FDEA   73 0E            JNB SHORT .0052FDFA
   *  0052FDEC   E8 F7360000      CALL .005334E8
   *  0052FDF1   6A 22            PUSH 0x22
   *  0052FDF3   59               POP ECX
   *  0052FDF4   8908             MOV DWORD PTR DS:[EAX],ECX
   *  0052FDF6   8BF1             MOV ESI,ECX
   *  0052FDF8  ^EB AD            JMP SHORT .0052FDA7
   *  0052FDFA   6A 16            PUSH 0x16
   *  0052FDFC   58               POP EAX
   *  0052FDFD   5F               POP EDI
   *  0052FDFE   5E               POP ESI
   *  0052FDFF   5D               POP EBP
   *  0052FE00   C3               RETN
   *  0052FE01   8BFF             MOV EDI,EDI
   *  0052FE03   55               PUSH EBP
   *  0052FE04   8BEC             MOV EBP,ESP
   *  0052FE06   8B45 14          MOV EAX,DWORD PTR SS:[EBP+0x14]
   *  0052FE09   56               PUSH ESI
   *  0052FE0A   57               PUSH EDI
   *  0052FE0B   33FF             XOR EDI,EDI
   *  0052FE0D   3BC7             CMP EAX,EDI
   *  0052FE0F   74 47            JE SHORT .0052FE58
   *  0052FE11   397D 08          CMP DWORD PTR SS:[EBP+0x8],EDI
   *  0052FE14   75 1B            JNZ SHORT .0052FE31
   *  0052FE16   E8 CD360000      CALL .005334E8
   *  0052FE1B   6A 16            PUSH 0x16
   *  0052FE1D   5E               POP ESI
   *  0052FE1E   8930             MOV DWORD PTR DS:[EAX],ESI
   *  0052FE20   57               PUSH EDI
   *  0052FE21   57               PUSH EDI
   *  0052FE22   57               PUSH EDI
   *  0052FE23   57               PUSH EDI
   *  0052FE24   57               PUSH EDI
   *  0052FE25   E8 6B010000      CALL .0052FF95
   *  0052FE2A   83C4 14          ADD ESP,0x14
   *  0052FE2D   8BC6             MOV EAX,ESI
   *  0052FE2F   EB 29            JMP SHORT .0052FE5A
   *  0052FE31   397D 10          CMP DWORD PTR SS:[EBP+0x10],EDI
   *  0052FE34  ^74 E0            JE SHORT .0052FE16
   *  0052FE36   3945 0C          CMP DWORD PTR SS:[EBP+0xC],EAX
   *  0052FE39   73 0E            JNB SHORT .0052FE49
   *  0052FE3B   E8 A8360000      CALL .005334E8
   *  0052FE40   6A 22            PUSH 0x22
   *  0052FE42   59               POP ECX
   *  0052FE43   8908             MOV DWORD PTR DS:[EAX],ECX
   *  0052FE45   8BF1             MOV ESI,ECX
   *  0052FE47  ^EB D7            JMP SHORT .0052FE20
   *  0052FE49   50               PUSH EAX
   *  0052FE4A   FF75 10          PUSH DWORD PTR SS:[EBP+0x10]
   *  0052FE4D   FF75 08          PUSH DWORD PTR SS:[EBP+0x8]
   *  0052FE50   E8 3B310000      CALL .00532F90
   *  0052FE55   83C4 0C          ADD ESP,0xC
   *  0052FE58   33C0             XOR EAX,EAX
   *  0052FE5A   5F               POP EDI
   *  0052FE5B   5E               POP ESI
   *  0052FE5C   5D               POP EBP
   *  0052FE5D   C3               RETN
   *  0052FE5E   8BFF             MOV EDI,EDI
   *  0052FE60   55               PUSH EBP
   *  0052FE61   8BEC             MOV EBP,ESP
   *  0052FE63   8B45 08          MOV EAX,DWORD PTR SS:[EBP+0x8]
   *  0052FE66   A3 5466D800      MOV DWORD PTR DS:[0xD86654],EAX
   *  0052FE6B   5D               POP EBP
   *  0052FE6C   C3               RETN
   *  0052FE6D   8BFF             MOV EDI,EDI
   *  0052FE6F   55               PUSH EBP
   *  0052FE70   8BEC             MOV EBP,ESP
   *  0052FE72   81EC 28030000    SUB ESP,0x328
   *  0052FE78   A1 E0225A00      MOV EAX,DWORD PTR DS:[0x5A22E0]
   *  0052FE7D   33C5             XOR EAX,EBP
   *  0052FE7F   8945 FC          MOV DWORD PTR SS:[EBP-0x4],EAX
   *  0052FE82   83A5 D8FCFFFF 00 AND DWORD PTR SS:[EBP-0x328],0x0
   *  0052FE89   53               PUSH EBX
   *  0052FE8A   6A 4C            PUSH 0x4C
   *  0052FE8C   8D85 DCFCFFFF    LEA EAX,DWORD PTR SS:[EBP-0x324]
   *  0052FE92   6A 00            PUSH 0x0
   *  0052FE94   50               PUSH EAX
   *  0052FE95   E8 C6060000      CALL .00530560
   *  0052FE9A   8D85 D8FCFFFF    LEA EAX,DWORD PTR SS:[EBP-0x328]
   */
  bool hookBefore(winhook::hook_stack *s)
  {
    static QByteArray data_;
    auto retaddr = s->stack[0];
    // 0052FDCE   83C4 0C          ADD ESP,0xC
    // 0052FDD1  ^EB C1            JMP SHORT .0052FD94
    //if (*(DWORD *)retaddr != 0xeb0cc483)
    //  return true;
    //retaddr = s->stack[7]; // parent caller
    auto q = EngineController::instance();
    auto text = (LPCSTR)s->stack[3]; // arg2
    if (!text || !*text
        || Engine::getTextLength(text) > 0x100
        || ::strlen(text) <= 2
        || Util::allAscii(text)
        || isascii(text[::strlen(text) - 2])
        || !q->isTextDecodable(text)
        || isSkippedText(text))
      return true;
    enum { role = Engine::OtherRole };
    QByteArray oldData = text;
    oldData.replace("\\n", ""); // Remove new line. FIXME: automatically adjust line width
    QByteArray newData = q->dispatchTextA(oldData, role, retaddr);
    if (newData == oldData)
      return true;
    //newData.replace("\\n", "\n");
    //texts_.insert(text);
    data_ = newData;
    s->stack[3] = (ulong)data_.constData();
    return true;
  }
} // namespace Private

/**
 *  Sample game:  戦極姫6
 *  Function found by debugging caller of GetGlyphOutlineA.
 *  0052F2DC   CC               INT3
 *  0052F2DD   CC               INT3
 *  0052F2DE   CC               INT3
 *  0052F2DF   CC               INT3
 *  0052F2E0   55               PUSH EBP
 *  0052F2E1   8BEC             MOV EBP,ESP
 *  0052F2E3   57               PUSH EDI
 *  0052F2E4   56               PUSH ESI
 *  0052F2E5   8B75 0C          MOV ESI,DWORD PTR SS:[EBP+0xC]
 *  0052F2E8   8B4D 10          MOV ECX,DWORD PTR SS:[EBP+0x10]
 *  0052F2EB   8B7D 08          MOV EDI,DWORD PTR SS:[EBP+0x8]
 *  0052F2EE   8BC1             MOV EAX,ECX
 *  0052F2F0   8BD1             MOV EDX,ECX
 *  0052F2F2   03C6             ADD EAX,ESI
 *  0052F2F4   3BFE             CMP EDI,ESI
 *  0052F2F6   76 08            JBE SHORT .0052F300
 *  0052F2F8   3BF8             CMP EDI,EAX
 *  0052F2FA   0F82 A4010000    JB .0052F4A4
 *  0052F300   81F9 00010000    CMP ECX,0x100
 *  0052F306   72 1F            JB SHORT .0052F327
 *  0052F308   833D 6472D800 00 CMP DWORD PTR DS:[0xD87264],0x0
 *  0052F30F   74 16            JE SHORT .0052F327
 *  0052F311   57               PUSH EDI
 *  0052F312   56               PUSH ESI
 *  0052F313   83E7 0F          AND EDI,0xF
 *  0052F316   83E6 0F          AND ESI,0xF
 *  0052F319   3BFE             CMP EDI,ESI
 *  0052F31B   5E               POP ESI
 *  0052F31C   5F               POP EDI
 *  0052F31D   75 08            JNZ SHORT .0052F327
 *  0052F31F   5E               POP ESI
 *  0052F320   5F               POP EDI
 *  0052F321   5D               POP EBP
 *  0052F322   E9 7C5F0000      JMP .005352A3
 *  0052F327   F7C7 03000000    TEST EDI,0x3
 *  0052F32D   75 15            JNZ SHORT .0052F344
 *  0052F32F   C1E9 02          SHR ECX,0x2
 *  0052F332   83E2 03          AND EDX,0x3
 *  0052F335   83F9 08          CMP ECX,0x8
 *  0052F338   72 2A            JB SHORT .0052F364
 *  0052F33A   F3:A5            REP MOVS DWORD PTR ES:[EDI],DWORD PTR DS>
 *  0052F33C   FF2495 54F45200  JMP DWORD PTR DS:[EDX*4+0x52F454]
 *  0052F343   90               NOP
 */
bool attach(ulong startAddress, ulong stopAddress)
{
  const uint8_t bytes[] = {
    0x74, 0x16,         // 0052f30f   74 16            je short .0052f327
    0x57,               // 0052f311   57               push edi
    0x56,               // 0052f312   56               push esi
    0x83,0xe7, 0x0f,    // 0052f313   83e7 0f          and edi,0xf
    0x83,0xe6, 0x0f,    // 0052f316   83e6 0f          and esi,0xf
    0x3b,0xfe,          // 0052f319   3bfe             cmp edi,esi
    0x5e,               // 0052f31b   5e               pop esi
    0x5f                // 0052f31c   5f               pop edi
  };
  ulong addr = MemDbg::findBytes(bytes, sizeof(bytes), startAddress, stopAddress);
  if (!addr)
    return false;
  addr = MemDbg::findEnclosingAlignedFunction(addr);
  if (!addr)
    return false;
  //ulong addr = 0x0052F2E0;
  addr = 0x0052FD84;
  return winhook::hook_before(addr, Private::hookBefore);
}

} // namespace OtherHook
} // unnamed namespace

bool UnicornEngine::attach()
{
  ulong startAddress, stopAddress;
  if (!Engine::getProcessMemoryRange(&startAddress, &stopAddress))
    return false;
  if (!ScenarioHook::attach(startAddress, stopAddress))
    return false;
  if (OtherHook::attach(startAddress, stopAddress))
    DOUT("other text found");
  else
    DOUT("other text NOT FOUND");
  HijackManager::instance()->attachFunction((ulong)::GetGlyphOutlineA);
  return true;
}

// EOF

#if 0
namespace OtherHook {
namespace Private {
  struct HookArgument
  {
    enum { ShortTextCapacity = 0x10 }; // 15, maximum number of characters for short text including '\0'
    DWORD type;       // 0x0, equal zero for scenario
    union {
      LPCSTR  text;       // 0x4, editable though
      char chars[ShortTextCapacity];
    };
    int size,     // 0x14, 0xf for short text
        capacity; // 0x18

    // Skip single character
    bool isValid() const
    {
      auto t = getText();
      return size > 2 && size <= capacity // skip individual kanji
          && Engine::isAddressWritable(t, size) && ::strlen(t) == size;
    }

    LPCSTR getText() const
    { return size < ShortTextCapacity ? chars : text; }

    void setText(LPCSTR _text, int _size)
    {
      if (_size < ShortTextCapacity)
        ::strcpy(chars, _text);
      else
        text = _text;
      capacity = size = _size;
    }
  };

  HookArgument *arg_,
               argValue_;
  bool hookBefore(winhook::hook_stack *s)
  {
    static QByteArray data_;
    auto arg = (HookArgument *)s->stack[1]; // arg1
    if (!arg || !arg->isValid())
      return true;
    // Skip translate font name: ＭＳ ゴシック
    auto text = arg->getText();
    if (!text || !*text || Util::allAscii(text))
      return true;
    enum { role = Engine::OtherRole };
    auto retaddr = s->stack[0];
    if (retaddr != 0x4053bb)
      return true;
    QByteArray oldData(text),
               newData = EngineController::instance()->dispatchTextA(oldData, role, retaddr);
    if (newData == oldData)
      return true;
    //texts_.insert(text);
    data_ = newData;
    arg_ = arg;
    argValue_ = *arg;
    arg->setText(data_.constData(), data_.size());
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
bool attach(ulong startAddress, ulong stopAddress)
{
  ulong addr = 0x00403AD0;
  return winhook::hook_before(addr, Private::hookBefore);
}

} // namespace OtherHook
#endif // 0
