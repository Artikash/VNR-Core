// horkeye.cc
// 6/24/2015 jichi
#include "engine/model/horkeye.h"
#include "engine/enginecontroller.h"
#include "engine/enginedef.h"
#include "engine/enginehash.h"
#include "engine/engineutil.h"
//#include "hijack/hijackmanager.h"
#include "memdbg/memsearch.h"
#include "winhook/hookcode.h"
#include <qt_windows.h>
#include <QtCore/QTextCodec>

#define DEBUG "horkeye"
#include "sakurakit/skdebug.h"

namespace { // unnamed
namespace ScenarioHook {
namespace Private {

  // CharNextA not used in order to ustilize sjis prefix
  QByteArray ltrim(const QByteArray &data, QByteArray &prefix, QTextCodec *codec)
  {
    QString text = codec->toUnicode(data);
    if (text.isEmpty())
      return data;
    const wchar_t w_open = 0x3010,
                  w_close = 0x3011;
    if (text[0].unicode() != w_open)
      return data;
    int pos = text.indexOf(w_close);
    if (pos == -1)
      return data;
    pos++;
    QString left = text.left(pos),
            right = text.mid(pos);
    prefix = codec->fromUnicode(left);
    return codec->fromUnicode(right);
  }

  /**
   *  Example text:
   *  【直子,S001_A1_0027】「ほら犬がなにか悪さをすると、しらばっくれるけどばればれな表情するでしょ？　今のシンクローにはそういうの全然ないもの」
   *
   *  Sample argument:
   *  00F44148  58 3B F4 00 59 20 39 2C 20 35 30 30 2C 20 34 30
   *  00F44158  1E 00 00 00 1F 00 00 00 38 30 40 31 39 30 40 00
   *  00F44168  5A D8 83 49 00 00 00 88 20 53 30 30 31 5F 31 42
   *  00F44178  41 32 42 41 5F 30 30 30 5F 4C 2C 20 32 35 30 00
   *  00F44188  24 65 78 74 2C 25 30 00 45 D8 83 49 00 00 00 88
   *  00F44198  20 30 30 31 2C 20 35 30 30 40 30 40 2D 31 2C 20
   *  00F441A8  31 30 2C 20 31 00 00 00 00 00 00 00 00 00 00 00
   *  00F441B8  40 D8 83 49 00 00 00 8C 00 01 E3 00 20 2A 65 78
   *  00F441C8  6D 6F 64 65 00 00 00 00 00 00 00 00 0F 00 00 00
   *  00F441D8  00 00 00 00 00 00 00 00 4B D8 83 49 00 00 00 8C
   *
   *  00F43B58  90 5F 93 DE 90 EC 8C A7 81 40 90 BC 8F C3 92 6E  神奈川県　西湘地
   *  00F43B68  88 E6 81 40 8F AC 98 61 93 63 8E 73 81 42 00 00  域　小和田市。..
   */
  struct HookArgument
  {
    LPCSTR text;  // 0x0
    DWORD unknown[3];
    int size,     // 0x10
        capacity; // 0x14
  };
  HookArgument *arg_;
  bool hookBefore(winhook::hook_stack *s)
  {
    static QList<QByteArray> data_;
    auto q = EngineController::instance();
    QTextCodec *codec = q->decoder(); // decoder have the sjis
    if (!codec)
      return true;
    //auto text = (LPCSTR)s->stack[1]; // text in arg1
    auto arg = (HookArgument *)s->eax;
    auto text = arg->text;
    enum { role = Engine::ScenarioRole, sig = Engine::ScenarioThreadSignature };
    QByteArray prefix,
               oldData = ltrim(text, prefix, codec),
               newData = q->dispatchTextA(oldData, sig, role);
    if (newData.isEmpty() || newData == oldData)
      return true;
    if (!prefix.isEmpty())
      newData.prepend(prefix);
    data_.append(newData);
    arg->text = newData.constData();
    s->ecx= arg->size = newData.size(); // size in arg2
    arg->capacity = arg->size + 1;

    //s->stack[1] = (ulong)data_.constData(); // text in arg1
    //s->stack[2] = data_.size(); // size in arg2
    //::strcpy(text, data_.constData());
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
 *  The first one is used. Text in arg1, and size in arg2.
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
 *  The second called function is here that is not used
 *  013CDC90   8B15 C4709901    MOV EDX,DWORD PTR DS:[0x19970C4]
 *  013CDC96   8A041E           MOV AL,BYTE PTR DS:[ESI+EBX]	; jichi: here
 *  013CDC99   884424 24        MOV BYTE PTR SS:[ESP+0x24],AL
 *  013CDC9D   0FB6C0           MOVZX EAX,AL
 *
 *  1/15/2015
 *  Alternative hook that might not need a text filter:
 *  http://www.hongfire.com/forum/showthread.php/36807-AGTH-text-extraction-tool-for-games-translation/page753
 *  /HA-4@552B5:姉小路直子と銀色の死神.exe
 *  If this hook no longer works, try that one instead.
 *
 *  0013F7B4   010E4826  RETURN to .010E4826 from .010DD980
 *  0013F7B8   00F83B58 ; jichi: arg1 text
 *  0013F7BC   0000001E ; jichi: arg2 size excluding count
 *  0013F7C0   0013F7DC
 *  0013F7C4   0013F7E4
 *  0013F7C8   FAB63973
 *
 *  姉小路直子と銀色の死神
 *  001BD979   81C4 AC000000    ADD ESP,0xAC
 *  001BD97F   C3               RETN
 *  001BD980   83EC 40          SUB ESP,0x40    ; jichi: hook here
 *  001BD983   A1 24803C00      MOV EAX,DWORD PTR DS:[0x3C8024]
 *  001BD988   8B15 C4707800    MOV EDX,DWORD PTR DS:[0x7870C4]
 *  001BD98E   8D0C00           LEA ECX,DWORD PTR DS:[EAX+EAX]
 *  001BD991   A1 9C504A00      MOV EAX,DWORD PTR DS:[0x4A509C]
 *  001BD996   0305 18803C00    ADD EAX,DWORD PTR DS:[0x3C8018]
 *  001BD99C   53               PUSH EBX
 *  001BD99D   8B5C24 48        MOV EBX,DWORD PTR SS:[ESP+0x48]
 *  001BD9A1   55               PUSH EBP
 *  001BD9A2   8B6C24 50        MOV EBP,DWORD PTR SS:[ESP+0x50]
 *  001BD9A6   894C24 34        MOV DWORD PTR SS:[ESP+0x34],ECX
 *  001BD9AA   8B0D 20803C00    MOV ECX,DWORD PTR DS:[0x3C8020]
 *  001BD9B0   894424 18        MOV DWORD PTR SS:[ESP+0x18],EAX
 *  001BD9B4   A1 1C803C00      MOV EAX,DWORD PTR DS:[0x3C801C]
 *  001BD9B9   03C8             ADD ECX,EAX
 *
 *  The only caller of this function
 *  009347A1   8B7424 38        MOV ESI,DWORD PTR SS:[ESP+0x38]
 *  009347A5   8B7431 10        MOV ESI,DWORD PTR DS:[ECX+ESI+0x10]
 *  009347A9   8B76 10          MOV ESI,DWORD PTR DS:[ESI+0x10]
 *  009347AC   03C0             ADD EAX,EAX
 *  009347AE   8B04C1           MOV EAX,DWORD PTR DS:[ECX+EAX*8]
 *  009347B1   8B48 10          MOV ECX,DWORD PTR DS:[EAX+0x10]
 *  009347B4   897424 14        MOV DWORD PTR SS:[ESP+0x14],ESI
 *  009347B8   894C24 1C        MOV DWORD PTR SS:[ESP+0x1C],ECX
 *  009347BC   EB 3B            JMP SHORT .009347F9
 *  009347BE   42               INC EDX
 *  009347BF   3BD1             CMP EDX,ECX
 *  009347C1   8B15 D0A91801    MOV EDX,DWORD PTR DS:[0x118A9D0]
 *  009347C7   7D 26            JGE SHORT .009347EF
 *  009347C9   8B0D AC51C100    MOV ECX,DWORD PTR DS:[0xC151AC]
 *  009347CF   8BC2             MOV EAX,EDX
 *  009347D1   03C0             ADD EAX,EAX
 *  009347D3   8B04C1           MOV EAX,DWORD PTR DS:[ECX+EAX*8]
 *  009347D6   8B4C24 38        MOV ECX,DWORD PTR SS:[ESP+0x38]
 *  009347DA   8B4408 10        MOV EAX,DWORD PTR DS:[EAX+ECX+0x10]
 *  009347DE   8B48 10          MOV ECX,DWORD PTR DS:[EAX+0x10]
 *  009347E1   894C24 14        MOV DWORD PTR SS:[ESP+0x14],ECX
 *  009347E5   C74424 1C 000000>MOV DWORD PTR SS:[ESP+0x1C],0x0
 *  009347ED   EB 0A            JMP SHORT .009347F9
 *  009347EF   33C0             XOR EAX,EAX
 *  009347F1   894424 14        MOV DWORD PTR SS:[ESP+0x14],EAX
 *  009347F5   894424 1C        MOV DWORD PTR SS:[ESP+0x1C],EAX
 *  009347F9   A1 AC51C100      MOV EAX,DWORD PTR DS:[0xC151AC]
 *  009347FE   03D2             ADD EDX,EDX
 *  00934800   8B0CD0           MOV ECX,DWORD PTR DS:[EAX+EDX*8]
 *  00934803   8B5424 38        MOV EDX,DWORD PTR SS:[ESP+0x38]
 *  00934807   8B040A           MOV EAX,DWORD PTR DS:[EDX+ECX]
 *  0093480A   8378 14 10       CMP DWORD PTR DS:[EAX+0x14],0x10
 *  0093480E   8B48 10          MOV ECX,DWORD PTR DS:[EAX+0x10]
 *  00934811   72 02            JB SHORT .00934815
 *  00934813   8B00             MOV EAX,DWORD PTR DS:[EAX]  ; jichi: hook here
 *  00934815   8D5424 1C        LEA EDX,DWORD PTR SS:[ESP+0x1C]
 *  00934819   52               PUSH EDX
 *  0093481A   8D5424 18        LEA EDX,DWORD PTR SS:[ESP+0x18]
 *  0093481E   52               PUSH EDX
 *  0093481F   51               PUSH ECX        ; jichi: count in ecx
 *  00934820   50               PUSH EAX        ; jichi: text in eax
 *  00934821   E8 5A91FFFF      CALL .0092D980  ; jichi: paint function called here
 *  00934826   83C4 10          ADD ESP,0x10    ; jichi: there are four parameters
 *  00934829   85C0             TEST EAX,EAX
 *  0093482B   0F84 2E010000    JE .0093495F
 *  00934831   A1 C070EF00      MOV EAX,DWORD PTR DS:[0xEF70C0]
 *  00934836   85C0             TEST EAX,EAX
 *  00934838   0F84 EF000000    JE .0093492D
 *  0093483E   833D C05BBD00 00 CMP DWORD PTR DS:[0xBD5BC0],0x0
 *  00934845   0F84 E2000000    JE .0093492D
 *  0093484B   833D C470EF00 00 CMP DWORD PTR DS:[0xEF70C4],0x0
 *  00934852   74 06            JE SHORT .0093485A
 *  00934854   FF0D D4A91801    DEC DWORD PTR DS:[0x118A9D4]
 *  0093485A   50               PUSH EAX
 *  0093485B   E8 80740900      CALL .009CBCE0
 *  00934860   83C4 04          ADD ESP,0x4
 *  00934863   A1 B0A11801      MOV EAX,DWORD PTR DS:[0x118A1B0]
 *  00934868   33FF             XOR EDI,EDI
 *  0093486A   3BC7             CMP EAX,EDI
 *  0093486C   0F84 B3000000    JE .00934925
 *  00934872   8D77 02          LEA ESI,DWORD PTR DS:[EDI+0x2]
 *  00934875   83F8 04          CMP EAX,0x4
 *  00934878   75 3B            JNZ SHORT .009348B5
 *  0093487A   8B0D AC51C100    MOV ECX,DWORD PTR DS:[0xC151AC]
 *  00934880   83C1 20          ADD ECX,0x20
 *  00934883   8935 D0A91801    MOV DWORD PTR DS:[0x118A9D0],ESI
 *  00934889   E8 E2ABFFFF      CALL .0092F470
 *  0093488E   A1 D0A91801      MOV EAX,DWORD PTR DS:[0x118A9D0]
 *  00934893   8B0D F4AC1801    MOV ECX,DWORD PTR DS:[0x118ACF4]
 *  00934899   8B15 F0AC1801    MOV EDX,DWORD PTR DS:[0x118ACF0]
 *  0093489F   C1E0 04          SHL EAX,0x4
 *  009348A2   0305 AC51C100    ADD EAX,DWORD PTR DS:[0xC151AC]
 *  009348A8   50               PUSH EAX
 *  009348A9   51               PUSH ECX
 *  009348AA   52               PUSH EDX
 *  009348AB   E8 40140A00      CALL .009D5CF0
 *  009348B0   83C4 0C          ADD ESP,0xC
 *  009348B3   EB 06            JMP SHORT .009348BB
 *  009348B5   48               DEC EAX
 *  009348B6   A3 D0A91801      MOV DWORD PTR DS:[0x118A9D0],EAX
 *  009348BB   A1 D0A91801      MOV EAX,DWORD PTR DS:[0x118A9D0]
 *  009348C0   8B0485 508FB300  MOV EAX,DWORD PTR DS:[EAX*4+0xB38F50]
 *  009348C7   50               PUSH EAX
 *  009348C8   B9 A4AD1801      MOV ECX,.0118ADA4
 *  009348CD   E8 9EA5FFFF      CALL .0092EE70
 *  009348D2   8B48 0C          MOV ECX,DWORD PTR DS:[EAX+0xC]
 *  009348D5   83C0 08          ADD EAX,0x8
 *  009348D8   894424 48        MOV DWORD PTR SS:[ESP+0x48],EAX
 *  009348DC   A1 D0A91801      MOV EAX,DWORD PTR DS:[0x118A9D0]
 *  009348E1   C1E0 04          SHL EAX,0x4
 *  009348E4   0305 AC51C100    ADD EAX,DWORD PTR DS:[0xC151AC]
 *  009348EA   894C24 18        MOV DWORD PTR SS:[ESP+0x18],ECX
 *  009348EE   893D B0A11801    MOV DWORD PTR DS:[0x118A1B0],EDI
 *  009348F4   8B48 04          MOV ECX,DWORD PTR DS:[EAX+0x4]
 *  009348F7   2B08             SUB ECX,DWORD PTR DS:[EAX]
 *  009348F9   A1 D870EF00      MOV EAX,DWORD PTR DS:[0xEF70D8]
 *  009348FE   C1F9 04          SAR ECX,0x4
 *  00934901   894C24 34        MOV DWORD PTR SS:[ESP+0x34],ECX
 *  00934905   3BC7             CMP EAX,EDI
 *  00934907   74 1C            JE SHORT .00934925
 *  00934909   50               PUSH EAX
 *  0093490A   E8 D1730900      CALL .009CBCE0
 *  0093490F   A1 B0A11801      MOV EAX,DWORD PTR DS:[0x118A1B0]
 *  00934914   83C4 04          ADD ESP,0x4
 *  00934917   893D D870EF00    MOV DWORD PTR DS:[0xEF70D8],EDI
 *  0093491D   3BC7             CMP EAX,EDI
 *  0093491F  ^0F85 50FFFFFF    JNZ .00934875
 *  00934925   393D 1CAD1801    CMP DWORD PTR DS:[0x118AD1C],EDI
 *  0093492B   75 1B            JNZ SHORT .00934948
 *  0093492D   8B0D AC9D1801    MOV ECX,DWORD PTR DS:[0x1189DAC]
 *  00934933   83C1 02          ADD ECX,0x2
 *  00934936   890D AC9D1801    MOV DWORD PTR DS:[0x1189DAC],ECX
 *  0093493C   3B0D C0A71801    CMP ECX,DWORD PTR DS:[0x118A7C0]
 *  00934942  ^0F8C 3BF8FFFF    JL .00934183
 *  00934948   8B7C24 34        MOV EDI,DWORD PTR SS:[ESP+0x34]
 *  0093494C   A1 D4A91801      MOV EAX,DWORD PTR DS:[0x118A9D4]
 *  00934951   40               INC EAX
 *  00934952   A3 D4A91801      MOV DWORD PTR DS:[0x118A9D4],EAX
 *  00934957   3BC7             CMP EAX,EDI
 *  00934959  ^0F8C B0F7FFFF    JL .0093410F
 *  0093495F   803D A850C100 00 CMP BYTE PTR DS:[0xC150A8],0x0
 *  00934966   74 05            JE SHORT .0093496D
 *  00934968   E8 4330FFFF      CALL .009279B0
 *  0093496D   E8 DE340400      CALL .00977E50
 *  00934972   A1 AC51C100      MOV EAX,DWORD PTR DS:[0xC151AC]
 *  00934977   8B10             MOV EDX,DWORD PTR DS:[EAX]
 *  00934979   8B48 04          MOV ECX,DWORD PTR DS:[EAX+0x4]
 *  0093497C   8D70 04          LEA ESI,DWORD PTR DS:[EAX+0x4]
 *  0093497F   8BE8             MOV EBP,EAX
 *  00934981   3BD1             CMP EDX,ECX
 *  00934983   74 2B            JE SHORT .009349B0
 *  00934985   8B4424 20        MOV EAX,DWORD PTR SS:[ESP+0x20]
 *  00934989   50               PUSH EAX
 *  0093498A   52               PUSH EDX
 *  0093498B   51               PUSH ECX
 *  0093498C   51               PUSH ECX
 *  0093498D   E8 3EA3FFFF      CALL .0092ECD0
 *  00934992   8B4C24 30        MOV ECX,DWORD PTR SS:[ESP+0x30]
 *  00934996   8B16             MOV EDX,DWORD PTR DS:[ESI]
 *  00934998   51               PUSH ECX
 *  00934999   83C5 0C          ADD EBP,0xC
 *  0093499C   55               PUSH EBP
 *  0093499D   8BF8             MOV EDI,EAX
 *  0093499F   52               PUSH EDX
 *  009349A0   57               PUSH EDI
 *  009349A1   E8 0A9BFFFF      CALL .0092E4B0
 *  009349A6   893E             MOV DWORD PTR DS:[ESI],EDI
 *  009349A8   A1 AC51C100      MOV EAX,DWORD PTR DS:[0xC151AC]
 *  009349AD   83C4 20          ADD ESP,0x20
 *  009349B0   8B50 10          MOV EDX,DWORD PTR DS:[EAX+0x10]
 *  009349B3   8B48 14          MOV ECX,DWORD PTR DS:[EAX+0x14]
 *  009349B6   8D70 10          LEA ESI,DWORD PTR DS:[EAX+0x10]
 *  009349B9   3BD1             CMP EDX,ECX
 *  009349BB   74 2D            JE SHORT .009349EA
 *  009349BD   8B4424 20        MOV EAX,DWORD PTR SS:[ESP+0x20]
 *  009349C1   50               PUSH EAX
 *  009349C2   52               PUSH EDX
 *  009349C3   51               PUSH ECX
 *  009349C4   51               PUSH ECX
 *  009349C5   E8 06A3FFFF      CALL .0092ECD0
 */
bool attach() // attach scenario
{
  ulong startAddress, stopAddress;
  if (!Engine::getProcessMemoryRange(&startAddress, &stopAddress))
    return false;

  //ulong addr = Private::findFunction(startAddress, stopAddress);
  //if (!addr)
  //  return false;

  const BYTE bytes[] = {
    //0x8b,0x48, 0x10,      // 0093480e   8b48 10          mov ecx,dword ptr ds:[eax+0x10]
    //0x72, 0x0,0x02,       // 00934811   72 02            jb short .00934815
    0x8b,0x00,            // 00934813   8b00             mov eax,dword ptr ds:[eax]  ; jichi: hook here
    0x8d,0x54,0x24, 0x1c, // 00934815   8d5424 1c        lea edx,dword ptr ss:[esp+0x1c]
    0x52,                 // 00934819   52               push edx
    0x8d,0x54,0x24, 0x18, // 0093481a   8d5424 18        lea edx,dword ptr ss:[esp+0x18]
    0x52,                 // 0093481e   52               push edx
    0x51,                 // 0093481f   51               push ecx
    0x50                  // 00934820   50               push eax
  };
  //enum { addr_offset = 0x00934813 - 0x0093480e }; // = 5
  ulong addr = MemDbg::findBytes(bytes, sizeof(bytes), startAddress, stopAddress);
  if (!addr)
    return 0;
  ///addr += addr_offset;
  return winhook::hook_before(addr, Private::hookBefore);

  //int count = 0;
  //auto fun = [&count](ulong call) {
  //  count += winhook::hook_both(call, Private::hookBefore, Private::hookAfter);
  //};
  //MemDbg::iterNearCallAddress(fun, addr, startAddress, stopAddress);
  //DOUT("call number =" << count);
  //return count;
}

} // namespace ScenarioHook

} // unnamed namespace

/** Public class */

bool HorkEyeEngine::attach() { return ScenarioHook::attach(); }

// EOF

/*
  ulong findFunction(ulong startAddress, ulong stopAddress)
  {
    const quint8 bytes[] = {
      0x89,0x6c,0x24, 0x24,   // 013cdb01   896c24 24        mov dword ptr ss:[esp+0x24],ebp
      0x89,0x74,0x24, 0x0c,   // 013cdb05   897424 0c        mov dword ptr ss:[esp+0xc],esi
      0x89,0x4c,0x24, 0x18,   // 013cdb09   894c24 18        mov dword ptr ss:[esp+0x18],ecx
      0x8a,0x0c,0x1a          // 013cdb0d   8a0c1a           mov cl,byte ptr ds:[edx+ebx]        jichi: text is here
    };
    ulong addr = MemDbg::findBytes(bytes, sizeof(bytes), startAddress, stopAddress);
    if (!addr)
      return 0;

    // 013cdabf   c3               retn
    // 013cdac0   83ec 40          sub esp,0x40    ; jichi: text here in arg1
    //
    // 0x013cdb0d - 0x013cdabf = 78
    for (DWORD i = addr; i > addr - 0x100; i--)
      if (*(DWORD *)i == 0x40ec83c3)
        return i + 1;
    return 0;
  }
*/
