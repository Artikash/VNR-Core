// pensil.cc
// 8/1/2015 jichi
#include "engine/model/pensil.h"
#include "engine/enginecontroller.h"
#include "engine/enginedef.h"
#include "engine/enginehash.h"
#include "engine/engineutil.h"
#include "hijack/hijackmanager.h"
#include "winhook/hookcode.h"
#include "memdbg/memsearch.h"
#include "winasm/winasmdef.h"
#include <qt_windows.h>
#include <QtCore/QRegExp>
#include <cstdint>

#define DEBUG "model/pensil"
#include "sakurakit/skdebug.h"

namespace { // unnamed
namespace ScenarioHook {
namespace Private {

  /**
   *  Sample game: はにつま
   *
   *  Scenario:
   *  0012FA1C   004139FA  RETURN to .004139FA from .0046AFF0
   *  0012FA20   0027D528
   *  0012FA24   001FD576
   *  0012FA28   00000146
   *  0012FA2C   11407B21
   *  0012FA30   75802EB2  user32.PeekMessageA
   *  0012FA34   00170566
   *  0012FA38   75803569  user32.DispatchMessageA
   *  0012FA3C   7580910F  user32.TranslateMessage
   *  0012FA40   00000023
   *  0012FA44   0000001C
   *  0012FA48   00000146
   *  0012FA4C   00000004
   *  0012FA50   00000001
   *  0012FA54   0000104C
   *  0012FA58   00030000
   *  0012FA5C   004B5F78  .004B5F78
   *  0012FA60   006C737C  .006C737C
   *  0012FA64   004B5F8C  .004B5F8C
   *  0012FA68   00000000
   *
   *  Name:
   *  0012FA18   004139B0  RETURN to .004139B0 from .0046AFF0
   *  0012FA1C   0023DA90
   *  0012FA20   004B6038  .004B6038
   *  0012FA24   001FD44C
   *  0012FA28   00000001
   *  0012FA2C   11407B21
   *  0012FA30   75802EB2  user32.PeekMessageA
   *  0012FA34   00170566
   *  0012FA38   75803569  user32.DispatchMessageA
   *  0012FA3C   7580910F  user32.TranslateMessage
   *  0012FA40   00000024
   *  0012FA44   0000001C
   *  0012FA48   0000017A
   *  0012FA4C   00000005
   *  0012FA50   00000001
   *  0012FA54   000010B0
   *  0012FA58   00030000
   *  0012FA5C   004B5F78  .004B5F78
   *  0012FA60   006C737C  .006C737C
   *  0012FA64   004B5F8C  .004B5F8C
   *  0012FA68   00000000
   *  0012FA6C   00000000
   *  0012FA70   00000000
   *
   *  Config:
   *  0012FA20   0040CE86  RETURN to .0040CE86 from .0046AFF0
   *  0012FA24   0D183308
   *  0012FA28   009549BF
   *  0012FA2C   32C69D99
   *  0012FA30   75802EB2  user32.PeekMessageA
   *  0012FA34   0050020E  .0050020E
   *  0012FA38   75803569  user32.DispatchMessageA
   *  0012FA3C   7580910F  user32.TranslateMessage
   *  0012FA40   00000006
   *  0012FA44   00000000
   *  0012FA48   00000000
   *  0012FA4C   00000000
   *  0012FA50   00839880
   *  0012FA54   0002E468
   *  0012FA58   00000000
   *  0012FA5C   004B5F78  .004B5F78
   *  0012FA60   01E9DE60
   *  0012FA64   004B5F8C  .004B5F8C
   *  0012FA68   00000000
   *  0012FA6C   00000000
   *
   *  Sample game: 大正×対称アリス
   *  Name:
   *  0012F9EC   00414628  RETURN to .00414628 from .0046C970
   *  0012F9F0   04641998
   *  0012F9F4   004B7078  .004B7078
   *  0012F9F8   00000001
   *  0012F9FC   B85ABBDB
   *  0012FA00   75802EB2  user32.PeekMessageA
   *  0012FA04   001F0BA4
   *  0012FA08   75803569  user32.DispatchMessageA
   *  0012FA0C   7580910F  user32.TranslateMessage
   *  0012FA10   00000000
   *  0012FA14   00000615
   *  0012FA18   00000665
   *  0012FA1C   00000010
   *  0012FA20   00000001
   *  0012FA24   740C6765  RETURN to uxtheme.740C6765 from user32.PtInRect
   *  0012FA28   00030000
   *  0012FA2C   004B6FB8  .004B6FB8
   *  0012FA30   000001A3
   *  0012FA34   004B6FCC  .004B6FCC
   *  0012FA38   740C672B  RETURN to uxtheme.740C672B from uxtheme.740C6742
   *
   *  Scenario:
   *  0012F9EC   00414677  RETURN to .00414677 from .0046C970
   *  0012F9F0   043DB388
   *  0012F9F4   00243E15
   *  0012F9F8   00000001
   *  0012F9FC   B85ABBDB
   *  0012FA00   75802EB2  user32.PeekMessageA
   *  0012FA04   001F0BA4
   *  0012FA08   75803569  user32.DispatchMessageA
   *  0012FA0C   7580910F  user32.TranslateMessage
   *  0012FA10   00000000
   *  0012FA14   00000615
   *  0012FA18   00000665
   *  0012FA1C   00000010
   *  0012FA20   00000001
   *  0012FA24   740C6765  RETURN to uxtheme.740C6765 from user32.PtInRect
   *  0012FA28   00030000
   */
  bool hookBefore(winhook::hook_stack *s)
  {
    static QByteArray data_;
    auto text = (LPCSTR)s->stack[2]; // arg2
    size_t size = ::strlen(text);

    auto role = Engine::OtherRole;
    if (text[size + 1] == 0 && text[size + 2] == 0) { //|| s->stack[4] == 1)
      //if (size <= 0x10) // name should not be very long
      role = Engine::NameRole;
    } else if (s->stack[3] < 0xfffff) // this value seems to be a counter for scenario, and pointer for other tet
      role = Engine::ScenarioRole;

    auto retaddr = s->stack[0];
    auto sig = Engine::hashThreadSignature(role, retaddr);
    data_ = EngineController::instance()->dispatchTextA(text, role, sig);
    s->stack[2] = (ulong)data_.constData();
    return true;
  }

} // namespace Private

/**
 *  Sample game: はにつま
 *
 *  Debugging method:
 *  1. Hook to GetGlyphOutlineA
 *  2. Find text in memory
 *     There are three matches. The static scenario text is found
 *  3. Looking for text on the stack
 *
 *  Name/Scenario/Other texts can be translated.
 *  History cannot be translated.
 *
 *  Text in arg2.
 *
 *  0046AFE8   CC               INT3
 *  0046AFE9   CC               INT3
 *  0046AFEA   CC               INT3
 *  0046AFEB   CC               INT3
 *  0046AFEC   CC               INT3
 *  0046AFED   CC               INT3
 *  0046AFEE   CC               INT3
 *  0046AFEF   CC               INT3
 *  0046AFF0   83EC 10          SUB ESP,0x10
 *  0046AFF3   56               PUSH ESI
 *  0046AFF4   57               PUSH EDI
 *  0046AFF5   8B7C24 1C        MOV EDI,DWORD PTR SS:[ESP+0x1C]
 *  0046AFF9   85FF             TEST EDI,EDI
 *  0046AFFB   0F84 D6020000    JE .0046B2D7
 *  0046B001   8B7424 20        MOV ESI,DWORD PTR SS:[ESP+0x20]
 *  0046B005   85F6             TEST ESI,ESI
 *  0046B007   0F84 CA020000    JE .0046B2D7
 *  0046B00D   55               PUSH EBP
 *  0046B00E   33ED             XOR EBP,EBP
 *  0046B010   392D A8766C00    CMP DWORD PTR DS:[0x6C76A8],EBP
 *  0046B016   75 09            JNZ SHORT .0046B021
 *  0046B018   5D               POP EBP
 *  0046B019   5F               POP EDI
 *  0046B01A   33C0             XOR EAX,EAX
 *  0046B01C   5E               POP ESI
 *  0046B01D   83C4 10          ADD ESP,0x10
 *  0046B020   C3               RETN
 *  0046B021   8B47 24          MOV EAX,DWORD PTR DS:[EDI+0x24]
 *  0046B024   8B4F 28          MOV ECX,DWORD PTR DS:[EDI+0x28]
 *  0046B027   8B57 2C          MOV EDX,DWORD PTR DS:[EDI+0x2C]
 *  0046B02A   894424 0C        MOV DWORD PTR SS:[ESP+0xC],EAX
 *  0046B02E   8B47 30          MOV EAX,DWORD PTR DS:[EDI+0x30]
 *  0046B031   53               PUSH EBX
 *  0046B032   894C24 14        MOV DWORD PTR SS:[ESP+0x14],ECX
 *  0046B036   895424 18        MOV DWORD PTR SS:[ESP+0x18],EDX
 *  0046B03A   894424 1C        MOV DWORD PTR SS:[ESP+0x1C],EAX
 *  0046B03E   8A1E             MOV BL,BYTE PTR DS:[ESI]
 *  0046B040   84DB             TEST BL,BL
 *  0046B042   0F84 95000000    JE .0046B0DD
 *  0046B048   EB 06            JMP SHORT .0046B050
 *  0046B04A   8D9B 00000000    LEA EBX,DWORD PTR DS:[EBX]
 *  0046B050   0FB716           MOVZX EDX,WORD PTR DS:[ESI]
 *  0046B053   0FB7C2           MOVZX EAX,DX
 *  0046B056   3D 5C630000      CMP EAX,0x635C
 *  0046B05B   0F8F 93010000    JG .0046B1F4
 *  0046B061   0F84 2B010000    JE .0046B192
 *  0046B067   3D 5C4E0000      CMP EAX,0x4E5C
 *  0046B06C   0F8F DF000000    JG .0046B151
 *  0046B072   0F84 9E010000    JE .0046B216
 *  0046B078   3D 5C430000      CMP EAX,0x435C
 *  0046B07D   0F84 0F010000    JE .0046B192
 *  0046B083   3D 5C460000      CMP EAX,0x465C
 *  0046B088   0F84 80000000    JE .0046B10E
 *  0046B08E   3D 5C470000      CMP EAX,0x475C
 *  0046B093   0F85 CA010000    JNZ .0046B263
 *  0046B099   8A46 02          MOV AL,BYTE PTR DS:[ESI+0x2]
 *  0046B09C   83C6 02          ADD ESI,0x2
 *  0046B09F   33C9             XOR ECX,ECX
 *  0046B0A1   3C 39            CMP AL,0x39
 *  0046B0A3   77 17            JA SHORT .0046B0BC
 *  0046B0A5   3C 30            CMP AL,0x30
 *  0046B0A7   72 13            JB SHORT .0046B0BC
 *  0046B0A9   83C6 01          ADD ESI,0x1
 *  0046B0AC   0FB6D0           MOVZX EDX,AL
 *  0046B0AF   8A06             MOV AL,BYTE PTR DS:[ESI]
 *  0046B0B1   3C 39            CMP AL,0x39
 *  0046B0B3   8D0C89           LEA ECX,DWORD PTR DS:[ECX+ECX*4]
 *  0046B0B6   8D4C4A D0        LEA ECX,DWORD PTR DS:[EDX+ECX*2-0x30]
 *  0046B0BA  ^76 E9            JBE SHORT .0046B0A5
 *  0046B0BC   8D4424 10        LEA EAX,DWORD PTR SS:[ESP+0x10]
 *  0046B0C0   50               PUSH EAX
 *  0046B0C1   81C1 00FFFFFF    ADD ECX,-0x100
 *  0046B0C7   51               PUSH ECX
 *  0046B0C8   57               PUSH EDI
 *  0046B0C9   E8 92F1FFFF      CALL .0046A260
 *  0046B0CE   83C4 0C          ADD ESP,0xC
 *  0046B0D1   03E8             ADD EBP,EAX
 *  0046B0D3   8A1E             MOV BL,BYTE PTR DS:[ESI]
 *  0046B0D5   84DB             TEST BL,BL
 *  0046B0D7  ^0F85 73FFFFFF    JNZ .0046B050
 *  0046B0DD   F647 10 01       TEST BYTE PTR DS:[EDI+0x10],0x1
 *  0046B0E1   74 09            JE SHORT .0046B0EC
 *  0046B0E3   57               PUSH EDI
 *  0046B0E4   E8 F7DDFFFF      CALL .00468EE0
 *  0046B0E9   83C4 04          ADD ESP,0x4
 *  0046B0EC   F647 10 08       TEST BYTE PTR DS:[EDI+0x10],0x8
 *  0046B0F0   74 12            JE SHORT .0046B104
 *  0046B0F2   833D 98026C00 00 CMP DWORD PTR DS:[0x6C0298],0x0
 *  0046B0F9   74 09            JE SHORT .0046B104
 *  0046B0FB   57               PUSH EDI
 *  0046B0FC   E8 6FE4FFFF      CALL .00469570
 *  0046B101   83C4 04          ADD ESP,0x4
 *  0046B104   5B               POP EBX
 *  0046B105   8BC5             MOV EAX,EBP
 *  0046B107   5D               POP EBP
 *  0046B108   5F               POP EDI
 *  0046B109   5E               POP ESI
 *  0046B10A   83C4 10          ADD ESP,0x10
 *  0046B10D   C3               RETN
 *  0046B10E   8A46 02          MOV AL,BYTE PTR DS:[ESI+0x2]
 *  0046B111   83C6 02          ADD ESI,0x2
 *  0046B114   33C9             XOR ECX,ECX
 *  0046B116   3C 39            CMP AL,0x39
 *  0046B118   77 1D            JA SHORT .0046B137
 *  0046B11A   8D9B 00000000    LEA EBX,DWORD PTR DS:[EBX]
 *  0046B120   3C 30            CMP AL,0x30
 *  0046B122   72 13            JB SHORT .0046B137
 *  0046B124   83C6 01          ADD ESI,0x1
 *  0046B127   0FB6D0           MOVZX EDX,AL
 *  0046B12A   8A06             MOV AL,BYTE PTR DS:[ESI]
 *  0046B12C   3C 39            CMP AL,0x39
 *  0046B12E   8D0C89           LEA ECX,DWORD PTR DS:[ECX+ECX*4]
 *  0046B131   8D4C4A D0        LEA ECX,DWORD PTR DS:[EDX+ECX*2-0x30]
 *  0046B135  ^76 E9            JBE SHORT .0046B120
 *  0046B137   6A 01            PUSH 0x1
 *  0046B139   8B0C8D 580D6C00  MOV ECX,DWORD PTR DS:[ECX*4+0x6C0D58]
 *  0046B140   8D4424 14        LEA EAX,DWORD PTR SS:[ESP+0x14]
 *  0046B144   50               PUSH EAX
 *  0046B145   51               PUSH ECX
 *  0046B146   57               PUSH EDI
 *  0046B147   E8 84FBFFFF      CALL .0046ACD0
 *  0046B14C   83C4 10          ADD ESP,0x10
 *  0046B14F  ^EB 80            JMP SHORT .0046B0D1
 *  0046B151   3D 5C520000      CMP EAX,0x525C
 *  0046B156   0F84 BA000000    JE .0046B216
 *  0046B15C   3D 5C530000      CMP EAX,0x535C
 *  0046B161  ^0F84 32FFFFFF    JE .0046B099
 *  0046B167   3D 5C5C0000      CMP EAX,0x5C5C
 *  0046B16C   0F85 F1000000    JNZ .0046B263
 *  0046B172   8D5424 10        LEA EDX,DWORD PTR SS:[ESP+0x10]
 *  0046B176   52               PUSH EDX
 *  0046B177   6A 5C            PUSH 0x5C
 *  0046B179   57               PUSH EDI
 *  0046B17A   E8 81F3FFFF      CALL .0046A500
 *  0046B17F   83C4 0C          ADD ESP,0xC
 *  0046B182   85C0             TEST EAX,EAX
 *  0046B184   0F84 43010000    JE .0046B2CD
 *  0046B18A   83C6 01          ADD ESI,0x1
 *  0046B18D  ^E9 41FFFFFF      JMP .0046B0D3
 *  0046B192   33C9             XOR ECX,ECX
 *  0046B194   83C6 02          ADD ESI,0x2
 *  0046B197   8A06             MOV AL,BYTE PTR DS:[ESI]
 *  0046B199   3C 39            CMP AL,0x39
 *  0046B19B   77 14            JA SHORT .0046B1B1
 *  0046B19D   3C 30            CMP AL,0x30
 *  0046B19F   72 10            JB SHORT .0046B1B1
 *  0046B1A1   83C1 FD          ADD ECX,-0x3
 *  0046B1A4   0FB6C0           MOVZX EAX,AL
 *  0046B1A7   C1E1 04          SHL ECX,0x4
 *  0046B1AA   03C8             ADD ECX,EAX
 *  0046B1AC   83C6 01          ADD ESI,0x1
 *  0046B1AF  ^EB E6            JMP SHORT .0046B197
 *  0046B1B1   3C 46            CMP AL,0x46
 *  0046B1B3   77 13            JA SHORT .0046B1C8
 *  0046B1B5   3C 41            CMP AL,0x41
 *  0046B1B7   72 0F            JB SHORT .0046B1C8
 *  0046B1B9   0FB6D0           MOVZX EDX,AL
 *  0046B1BC   C1E1 04          SHL ECX,0x4
 *  0046B1BF   8D4C11 C9        LEA ECX,DWORD PTR DS:[ECX+EDX-0x37]
 *  0046B1C3   83C6 01          ADD ESI,0x1
 *  0046B1C6  ^EB CF            JMP SHORT .0046B197
 *  0046B1C8   3C 66            CMP AL,0x66
 *  0046B1CA   77 13            JA SHORT .0046B1DF
 *  0046B1CC   3C 61            CMP AL,0x61
 *  0046B1CE   72 0F            JB SHORT .0046B1DF
 *  0046B1D0   0FB6C0           MOVZX EAX,AL
 *  0046B1D3   C1E1 04          SHL ECX,0x4
 *  0046B1D6   8D4C01 A9        LEA ECX,DWORD PTR DS:[ECX+EAX-0x57]
 *  0046B1DA   83C6 01          ADD ESI,0x1
 *  0046B1DD  ^EB B8            JMP SHORT .0046B197
 *  0046B1DF   894C24 1C        MOV DWORD PTR SS:[ESP+0x1C],ECX
 *  0046B1E3   894C24 18        MOV DWORD PTR SS:[ESP+0x18],ECX
 *  0046B1E7   894C24 14        MOV DWORD PTR SS:[ESP+0x14],ECX
 *  0046B1EB   894C24 10        MOV DWORD PTR SS:[ESP+0x10],ECX
 *  0046B1EF  ^E9 DFFEFFFF      JMP .0046B0D3
 *  0046B1F4   3D 5C720000      CMP EAX,0x725C
 *  0046B1F9   7F 56            JG SHORT .0046B251
 *  0046B1FB   74 19            JE SHORT .0046B216
 *  0046B1FD   3D 5C660000      CMP EAX,0x665C
 *  0046B202   74 23            JE SHORT .0046B227
 *  0046B204   3D 5C670000      CMP EAX,0x675C
 *  0046B209  ^0F84 8AFEFFFF    JE .0046B099
 *  0046B20F   3D 5C6E0000      CMP EAX,0x6E5C
 *  0046B214   75 4D            JNZ SHORT .0046B263
 *  0046B216   57               PUSH EDI
 *  0046B217   E8 54DBFFFF      CALL .00468D70
 *  0046B21C   83C4 04          ADD ESP,0x4
 *  0046B21F   83C6 02          ADD ESI,0x2
 *  0046B222  ^E9 ACFEFFFF      JMP .0046B0D3
 *  0046B227   8A46 02          MOV AL,BYTE PTR DS:[ESI+0x2]
 *  0046B22A   83C6 02          ADD ESI,0x2
 *  0046B22D   33C9             XOR ECX,ECX
 *  0046B22F   3C 39            CMP AL,0x39
 *  0046B231   77 17            JA SHORT .0046B24A
 *  0046B233   3C 30            CMP AL,0x30
 *  0046B235   72 13            JB SHORT .0046B24A
 *  0046B237   83C6 01          ADD ESI,0x1
 *  0046B23A   0FB6D0           MOVZX EDX,AL
 *  0046B23D   8A06             MOV AL,BYTE PTR DS:[ESI]
 *  0046B23F   3C 39            CMP AL,0x39
 *  0046B241   8D0C89           LEA ECX,DWORD PTR DS:[ECX+ECX*4]
 *  0046B244   8D4C4A D0        LEA ECX,DWORD PTR DS:[EDX+ECX*2-0x30]
 *  0046B248  ^76 E9            JBE SHORT .0046B233
 *  0046B24A   6A 00            PUSH 0x0
 *  0046B24C  ^E9 E8FEFFFF      JMP .0046B139
 *  0046B251   3D 5C730000      CMP EAX,0x735C
 *  0046B256  ^0F84 3DFEFFFF    JE .0046B099
 *  0046B25C   3D 5C7B0000      CMP EAX,0x7B5C
 *  0046B261   74 49            JE SHORT .0046B2AC
 *  0046B263   52               PUSH EDX
 *  0046B264   E8 C7D5FFFF      CALL .00468830
 *  0046B269   83C4 04          ADD ESP,0x4
 *  0046B26C   85C0             TEST EAX,EAX
 *  0046B26E   74 1E            JE SHORT .0046B28E
 *  0046B270   8D4424 10        LEA EAX,DWORD PTR SS:[ESP+0x10]
 *  0046B274   50               PUSH EAX
 *  0046B275   52               PUSH EDX
 *  0046B276   57               PUSH EDI
 *  0046B277   E8 E4EFFFFF      CALL .0046A260
 *  0046B27C   83C4 0C          ADD ESP,0xC
 *  0046B27F   85C0             TEST EAX,EAX
 *  0046B281   74 4A            JE SHORT .0046B2CD
 *  0046B283   83C6 02          ADD ESI,0x2
 *  0046B286   83C5 01          ADD EBP,0x1
 *  0046B289  ^E9 45FEFFFF      JMP .0046B0D3
 *  0046B28E   8D4C24 10        LEA ECX,DWORD PTR SS:[ESP+0x10]
 *  0046B292   51               PUSH ECX
 *  0046B293   53               PUSH EBX
 *  0046B294   57               PUSH EDI
 *  0046B295   E8 66F2FFFF      CALL .0046A500
 *  0046B29A   83C4 0C          ADD ESP,0xC
 *  0046B29D   85C0             TEST EAX,EAX
 *  0046B29F   74 2C            JE SHORT .0046B2CD
 *  0046B2A1   83C6 01          ADD ESI,0x1
 *  0046B2A4   83C5 01          ADD EBP,0x1
 *  0046B2A7  ^E9 27FEFFFF      JMP .0046B0D3
 *  0046B2AC   8D5424 24        LEA EDX,DWORD PTR SS:[ESP+0x24]
 *  0046B2B0   52               PUSH EDX
 *  0046B2B1   83C6 02          ADD ESI,0x2
 *  0046B2B4   56               PUSH ESI
 *  0046B2B5   57               PUSH EDI
 *  0046B2B6   E8 F5F4FFFF      CALL .0046A7B0
 *  0046B2BB   8BF0             MOV ESI,EAX
 *  0046B2BD   83C4 0C          ADD ESP,0xC
 *  0046B2C0   85F6             TEST ESI,ESI
 *  0046B2C2   74 09            JE SHORT .0046B2CD
 *  0046B2C4   036C24 24        ADD EBP,DWORD PTR SS:[ESP+0x24]
 *  0046B2C8  ^E9 06FEFFFF      JMP .0046B0D3
 *  0046B2CD   5B               POP EBX
 *  0046B2CE   5D               POP EBP
 *  0046B2CF   5F               POP EDI
 *  0046B2D0   33C0             XOR EAX,EAX
 *  0046B2D2   5E               POP ESI
 *  0046B2D3   83C4 10          ADD ESP,0x10
 *  0046B2D6   C3               RETN
 *  0046B2D7   5F               POP EDI
 *  0046B2D8   33C0             XOR EAX,EAX
 *  0046B2DA   5E               POP ESI
 *  0046B2DB   83C4 10          ADD ESP,0x10
 *  0046B2DE   C3               RETN
 *  0046B2DF   CC               INT3
 */
bool attach(ulong startAddress, ulong stopAddress)
{
  const uint8_t bytes[] = {
    0x8b,0x47, 0x24,        // 0046b021   8b47 24          mov eax,dword ptr ds:[edi+0x24]
    0x8b,0x4f, 0x28,        // 0046b024   8b4f 28          mov ecx,dword ptr ds:[edi+0x28]
    0x8b,0x57, 0x2c,        // 0046b027   8b57 2c          mov edx,dword ptr ds:[edi+0x2c]
    0x89,0x44,0x24, 0x0c,   // 0046b02a   894424 0c        mov dword ptr ss:[esp+0xc],eax
    0x8b,0x47, 0x30         // 0046b02e   8b47 30          mov eax,dword ptr ds:[edi+0x30]
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

bool PensilEngine::attach()
{
  ulong startAddress, stopAddress;
  if (!Engine::getProcessMemoryRange(&startAddress, &stopAddress))
    return false;
  if (!ScenarioHook::attach(startAddress, stopAddress))
    return false;
  // Unfortunately, GetGlyphOutlineA is not invoked for all Pensil games
  HijackManager::instance()->attachFunction((ulong)::GetGlyphOutlineA);
  return true;
}

/**
 *  Sample ruby: 凄い綺麗な\{女性|ひと}だ。
 */
QString PensilEngine::rubyCreate(const QString &rb, const QString &rt)
{
  static QString fmt = "\\{%1|%2}";
  return fmt.arg(rb, rt);
}

// Remove furigana in scenario thread.
QString PensilEngine::rubyRemove(const QString &text)
{
  if (!text.contains('|'))
    return text;
  static QRegExp rx("\\\\\\{(.*)\\|.*\\}");
  if (!rx.isMinimal())
    rx.setMinimal(true);
  return QString(text).replace(rx, "\\1");
}

// EOF
