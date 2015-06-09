// rgss.cc
// 6/7/2015 jichi
#include "engine/model/rgss.h"
#include "engine/enginecontroller.h"
#include "engine/enginedef.h"
#include "engine/enginehash.h"
#include "engine/engineutil.h"
#include "util/textutil.h"
#include "hijack/hijackmanager.h"
#include "memdbg/memsearch.h"
#include "winhook/hookcode.h"
#include <qt_windows.h>
//#include <QtCore/QRegExp>
#include <QtCore/QSet>
#include <boost/foreach.hpp>

#define DEBUG "rgss"
#include "sakurakit/skdebug.h"

//#pragma intrinsic(_ReturnAddress)

namespace { // unnamed

namespace RGSS3Hook {
namespace Private {

  QString getDllModuleName()
  {
    foreach (const QString &dll, Engine::glob("System/RGSS3*.dll"))
      if (::GetModuleHandleW((LPCWSTR)dll.utf16()))
        return dll;
    return QString();
  }

  enum { MaxTextSize = 0x1000 };
  //char oldText_[MaxTextSize + 1]; // 1 extra 0 that is always 0
  //size_t oldSize_;

  struct HookArgument
  {
    LPDWORD type;    // 0x0
    LPDWORD unknown; // 0x4
    size_t size;     // 0x8
    LPSTR text;      // 0xc

    bool isValid() const
    {
      return Engine::isAddressReadable(type) && *type
          && size && size < MaxTextSize
          && Engine::isAddressWritable(text, size + 1) && *text
          && text[size] == 0 && ::strlen(text) == size  // validate size
          //&& !::strchr(text, '/')
          && !Util::allAscii(text);
    }

    //int size() const { return (*type >> 0xe) & 0x1f; }
  };

  inline bool _trims(const QChar &ch)
  { return ch.unicode() <= 127 || ch.isSpace(); }

  QString trim(const QString &text, QString *prefix = nullptr, QString *suffix = nullptr)
  {
    if (text.isEmpty() ||
        !_trims(text[0]) && !_trims(text[text.size() - 1]))
      return text;
    QString ret = text;
    if (_trims(ret[0])) {
      int pos = 1;
      for (; pos < ret.size() && _trims(ret[pos]); pos++);
      if (prefix)
        *prefix = ret.left(pos);
      ret = ret.mid(pos);
    }
    if (!ret.isEmpty() && _trims(ret[ret.size() - 1])) {
      int pos = ret.size() - 2;
      for (; pos >= 0 && _trims(ret[pos]); pos--);
      if (suffix)
        *suffix = ret.mid(pos + 1);
      ret = ret.left(pos + 1);
    }
    return ret;
  }

  bool hookBefore(winhook::hook_stack *s)
  {
    static QSet<HookArgument *> args_;
    static QSet<QString> texts_;

    auto arg = (HookArgument *)s->stack[1]; // arg1
    if (!args_.contains(arg) && arg->isValid()) { // && (quint8)arg->text[0] > 127) { // skip translate text beginning with ascii character
      args_.insert(arg); // make sure it is only translated once
      QString oldText = QString::fromUtf8(arg->text, arg->size),
              prefix,
              suffix,
              trimmedText = trim(oldText, &prefix, &suffix);
      //trimmedText.remove('\n')  // remove Linux new line characters
      //           .remove('\r');
      if (!trimmedText.isEmpty() && !texts_.contains(trimmedText) && !texts_.contains(oldText)) { // skip text beginning with ascii character
        enum { role = Engine::ScenarioRole };
        //ulong split = arg->unknown2[0]; // always 2
        ulong split = s->stack[0]; // return address
        auto sig = Engine::hashThreadSignature(role, split);

        QString newText = EngineController::instance()->dispatchTextW(trimmedText, sig, role);
        if (newText != trimmedText) {
          texts_.insert(newText);

          if (!prefix.isEmpty())
            newText.prepend(prefix);
          if (!suffix.isEmpty())
            newText.append(suffix);

          QByteArray data = newText.toUtf8();
          //if (Engine::isAddressWritable(arg->text, data.size() + 1))

          arg->size = min(data.size(), MaxTextSize - 1);
          ::memcpy(arg->text, data.constData(), arg->size + 1);

          //static QByteArray data_;
          //data_ = data;
          //arg->text = const_cast<LPSTR>(data_.constData());

          //arg_ = arg;
          //oldSize_ = arg->size;
          //::memcpy(oldText_, arg->text, min(arg->size + 1, MaxTextSize));
        }
      }
    }
    return true;
  }

  //bool hookAfter(winhook::hook_stack *)
  //{
  //  if (arg_) {
  //    arg_->size = oldSize_;
  //    ::strcpy(arg_->text, oldText_);
  //    arg_ = nullptr;
  //  }
  //  return true;
  //}

} // namespace Private

/**
 *  Sample game: Mogeko Castle with RGSS 3.01
 *
 *  1004149D   CC               INT3
 *  1004149E   CC               INT3
 *  1004149F   CC               INT3
 *  100414A0   8B4C24 08        MOV ECX,DWORD PTR SS:[ESP+0x8]
 *  100414A4   8BC1             MOV EAX,ECX
 *  100414A6   E8 75030500      CALL RGSS301.10091820
 *  100414AB   83F8 05          CMP EAX,0x5
 *  100414AE   74 19            JE SHORT RGSS301.100414C9
 *  100414B0   68 649D1A10      PUSH RGSS301.101A9D64                    ; ASCII "to_str"
 *  100414B5   68 74931A10      PUSH RGSS301.101A9374                    ; ASCII "String"
 *  100414BA   6A 05            PUSH 0x5
 *  100414BC   51               PUSH ECX
 *  100414BD   E8 AE2FFFFF      CALL RGSS301.10034470
 *  100414C2   83C4 10          ADD ESP,0x10
 *  100414C5   894424 08        MOV DWORD PTR SS:[ESP+0x8],EAX
 *  100414C9   53               PUSH EBX
 *  100414CA   55               PUSH EBP
 *  100414CB   56               PUSH ESI
 *  100414CC   8B7424 10        MOV ESI,DWORD PTR SS:[ESP+0x10]
 *  100414D0   57               PUSH EDI
 *  100414D1   8B7C24 18        MOV EDI,DWORD PTR SS:[ESP+0x18]
 *  100414D5   57               PUSH EDI
 *  100414D6   56               PUSH ESI
 *  100414D7   E8 B4490100      CALL RGSS301.10055E90
 *  100414DC   8BE8             MOV EBP,EAX
 *  100414DE   8B06             MOV EAX,DWORD PTR DS:[ESI]
 *  100414E0   83C4 08          ADD ESP,0x8
 *  100414E3   A9 00200000      TEST EAX,0x2000
 *  100414E8   75 08            JNZ SHORT RGSS301.100414F2
 *  100414EA   C1E8 0E          SHR EAX,0xE
 *  100414ED   83E0 1F          AND EAX,0x1F
 *  100414F0   EB 03            JMP SHORT RGSS301.100414F5
 *  100414F2   8B46 08          MOV EAX,DWORD PTR DS:[ESI+0x8]
 *  100414F5   8B0F             MOV ECX,DWORD PTR DS:[EDI]
 *  100414F7   F7C1 00200000    TEST ECX,0x2000
 *  100414FD   75 08            JNZ SHORT RGSS301.10041507
 *  100414FF   C1E9 0E          SHR ECX,0xE
 *  10041502   83E1 1F          AND ECX,0x1F
 *  10041505   EB 03            JMP SHORT RGSS301.1004150A
 *  10041507   8B4F 08          MOV ECX,DWORD PTR DS:[EDI+0x8]
 *  1004150A   8D3401           LEA ESI,DWORD PTR DS:[ECX+EAX]
 *  1004150D   A1 70C02A10      MOV EAX,DWORD PTR DS:[0x102AC070]
 *  10041512   50               PUSH EAX
 *  10041513   33FF             XOR EDI,EDI
 *  10041515   E8 B64EFFFF      CALL RGSS301.100363D0
 *  1004151A   8B5424 18        MOV EDX,DWORD PTR SS:[ESP+0x18] ; jichi: edx = arg1 on the stack
 *  1004151E   8BD8             MOV EBX,EAX
 *  10041520   8B02             MOV EAX,DWORD PTR DS:[EDX]  ; jichi: eax = ecx = [arg1]
 *  10041522   8BC8             MOV ECX,EAX
 *  10041524   83C4 04          ADD ESP,0x4
 *  10041527   81E1 00200000    AND ECX,0x2000
 *  1004152D   75 08            JNZ SHORT RGSS301.10041537
 *  1004152F   C1E8 0E          SHR EAX,0xE
 *  10041532   83E0 1F          AND EAX,0x1F
 *  10041535   EB 03            JMP SHORT RGSS301.1004153A
 *  10041537   8B42 08          MOV EAX,DWORD PTR DS:[EDX+0x8] ; jichi: [edx+0x8] text length
 *  1004153A   85C9             TEST ECX,ECX
 *  1004153C   75 05            JNZ SHORT RGSS301.10041543
 *  1004153E   83C2 08          ADD EDX,0x8
 *  10041541   EB 03            JMP SHORT RGSS301.10041546
 *  10041543   8B52 0C          MOV EDX,DWORD PTR DS:[EDX+0xC] ; jichi: [edx + 0xc] could be the text address
 *  10041546   F703 00200000    TEST DWORD PTR DS:[EBX],0x2000
 *  1004154C   8D4B 08          LEA ECX,DWORD PTR DS:[EBX+0x8]
 *  1004154F   74 03            JE SHORT RGSS301.10041554
 *  10041551   8B4B 0C          MOV ECX,DWORD PTR DS:[EBX+0xC]
 *  10041554   50               PUSH EAX
 *  10041555   52               PUSH EDX
 *  10041556   51               PUSH ECX
 *  10041557   E8 E4F21300      CALL RGSS301.10180840   ; jichi: text is in edx
 *  1004155C   8B5424 24        MOV EDX,DWORD PTR SS:[ESP+0x24]
 *  10041560   8B02             MOV EAX,DWORD PTR DS:[EDX]
 *  10041562   8BC8             MOV ECX,EAX
 *  10041564   83C4 0C          ADD ESP,0xC
 *  10041567   81E1 00200000    AND ECX,0x2000
 *  1004156D   75 08            JNZ SHORT RGSS301.10041577
 *
 *  Stack:
 *  00828EB4   1002E5E6  RETURN to RGSS301.1002E5E6 from RGSS301.100414A0
 *  00828EB8   03F13B20
 *  00828EBC   069F42CC
 *  00828EC0   00000000
 *  00828EC4   01699298
 *  00828EC8   01699298
 *  00828ECC   03EB41B8
 *  00828ED0   01692A00
 *  00828ED4   06A34548
 *  00828ED8   00000000
 *  00828EDC   00000168
 *  00828EE0   00000280
 *  00828EE4   000001E0
 *  00828EE8   1019150F  RETURN to RGSS301.1019150F from RGSS301.1018DF45
 *
 *  Here's the strncpy-like function for UTF8 strings, which is found using hardware breakpoints
 *  Parameters:
 *  - arg1 char *dest
 *  - arg2 const char *src
 *  - arg3 size_t size  length of src excluding \0 at the end
 *
 *  1018083A   CC               INT3
 *  1018083B   CC               INT3
 *  1018083C   CC               INT3
 *  1018083D   CC               INT3
 *  1018083E   CC               INT3
 *  1018083F   CC               INT3
 *  10180840   55               PUSH EBP
 *  10180841   8BEC             MOV EBP,ESP
 *  10180843   57               PUSH EDI
 *  10180844   56               PUSH ESI
 *  10180845   8B75 0C          MOV ESI,DWORD PTR SS:[EBP+0xC]
 *  10180848   8B4D 10          MOV ECX,DWORD PTR SS:[EBP+0x10]
 *  1018084B   8B7D 08          MOV EDI,DWORD PTR SS:[EBP+0x8]
 *  1018084E   8BC1             MOV EAX,ECX
 *  10180850   8BD1             MOV EDX,ECX
 *  10180852   03C6             ADD EAX,ESI
 *  10180854   3BFE             CMP EDI,ESI
 *  10180856   76 08            JBE SHORT RGSS301.10180860
 *  10180858   3BF8             CMP EDI,EAX
 *  1018085A   0F82 A4010000    JB RGSS301.10180A04
 *  10180860   81F9 00010000    CMP ECX,0x100
 *  10180866   72 1F            JB SHORT RGSS301.10180887
 *  10180868   833D 4CC12A10 00 CMP DWORD PTR DS:[0x102AC14C],0x0
 *  1018086F   74 16            JE SHORT RGSS301.10180887
 *  10180871   57               PUSH EDI
 *  10180872   56               PUSH ESI
 *  10180873   83E7 0F          AND EDI,0xF
 *  10180876   83E6 0F          AND ESI,0xF
 *  10180879   3BFE             CMP EDI,ESI
 *  1018087B   5E               POP ESI
 *  1018087C   5F               POP EDI
 *  1018087D   75 08            JNZ SHORT RGSS301.10180887
 *  1018087F   5E               POP ESI
 *  10180880   5F               POP EDI
 *  10180881   5D               POP EBP
 *  10180882   E9 05F80000      JMP RGSS301.1019008C
 *  10180887   F7C7 03000000    TEST EDI,0x3
 *  1018088D   75 15            JNZ SHORT RGSS301.101808A4
 *  1018088F   C1E9 02          SHR ECX,0x2
 *  10180892   83E2 03          AND EDX,0x3
 *  10180895   83F9 08          CMP ECX,0x8
 *  10180898   72 2A            JB SHORT RGSS301.101808C4
 *  1018089A   F3:A5            REP MOVS DWORD PTR ES:[EDI],DWORD PTR DS>
 *  1018089C   FF2495 B4091810  JMP DWORD PTR DS:[EDX*4+0x101809B4]
 *  101808A3   90               NOP
 *  101808A4   8BC7             MOV EAX,EDI
 *  101808A6   BA 03000000      MOV EDX,0x3
 *  101808AB   83E9 04          SUB ECX,0x4
 *  101808AE   72 0C            JB SHORT RGSS301.101808BC
 *  101808B0   83E0 03          AND EAX,0x3
 *  101808B3   03C8             ADD ECX,EAX
 *  101808B5   FF2485 C8081810  JMP DWORD PTR DS:[EAX*4+0x101808C8]
 *  101808BC   FF248D C4091810  JMP DWORD PTR DS:[ECX*4+0x101809C4]
 *  101808C3   90               NOP
 *  101808C4   FF248D 48091810  JMP DWORD PTR DS:[ECX*4+0x10180948]
 *  101808CB   90               NOP
 *  101808CC   D808             FMUL DWORD PTR DS:[EAX]
 *  101808CE   1810             SBB BYTE PTR DS:[EAX],DL
 *  101808D0   04 09            ADD AL,0x9
 *  101808D2   1810             SBB BYTE PTR DS:[EAX],DL
 *  101808D4   2809             SUB BYTE PTR DS:[ECX],CL
 *  101808D6   1810             SBB BYTE PTR DS:[EAX],DL
 *  101808D8   23D1             AND EDX,ECX
 *  101808DA   8A06             MOV AL,BYTE PTR DS:[ESI]
 *  101808DC   8807             MOV BYTE PTR DS:[EDI],AL
 *  101808DE   8A46 01          MOV AL,BYTE PTR DS:[ESI+0x1]
 *  101808E1   8847 01          MOV BYTE PTR DS:[EDI+0x1],AL
 *  101808E4   8A46 02          MOV AL,BYTE PTR DS:[ESI+0x2]
 *  101808E7   C1E9 02          SHR ECX,0x2
 *  101808EA   8847 02          MOV BYTE PTR DS:[EDI+0x2],AL
 *  101808ED   83C6 03          ADD ESI,0x3
 *  101808F0   83C7 03          ADD EDI,0x3
 *  101808F3   83F9 08          CMP ECX,0x8
 *  101808F6  ^72 CC            JB SHORT RGSS301.101808C4
 *  101808F8   F3:A5            REP MOVS DWORD PTR ES:[EDI],DWORD PTR DS>
 *  101808FA   FF2495 B4091810  JMP DWORD PTR DS:[EDX*4+0x101809B4]
 *  10180901   8D49 00          LEA ECX,DWORD PTR DS:[ECX]
 *  10180904   23D1             AND EDX,ECX
 *  10180906   8A06             MOV AL,BYTE PTR DS:[ESI]
 *  10180908   8807             MOV BYTE PTR DS:[EDI],AL
 *  1018090A   8A46 01          MOV AL,BYTE PTR DS:[ESI+0x1]
 *  1018090D   C1E9 02          SHR ECX,0x2
 *  10180910   8847 01          MOV BYTE PTR DS:[EDI+0x1],AL
 *  10180913   83C6 02          ADD ESI,0x2
 *  10180916   83C7 02          ADD EDI,0x2
 *  10180919   83F9 08          CMP ECX,0x8
 *  1018091C  ^72 A6            JB SHORT RGSS301.101808C4
 *  1018091E   F3:A5            REP MOVS DWORD PTR ES:[EDI],DWORD PTR DS>
 *  10180920   FF2495 B4091810  JMP DWORD PTR DS:[EDX*4+0x101809B4]
 *  10180927   90               NOP
 *  10180928   23D1             AND EDX,ECX
 *  1018092A   8A06             MOV AL,BYTE PTR DS:[ESI]
 *  1018092C   8807             MOV BYTE PTR DS:[EDI],AL
 *  1018092E   83C6 01          ADD ESI,0x1
 *  10180931   C1E9 02          SHR ECX,0x2
 *  10180934   83C7 01          ADD EDI,0x1
 *  10180937   83F9 08          CMP ECX,0x8
 *  1018093A  ^72 88            JB SHORT RGSS301.101808C4
 *  1018093C   F3:A5            REP MOVS DWORD PTR ES:[EDI],DWORD PTR DS>
 *  1018093E   FF2495 B4091810  JMP DWORD PTR DS:[EDX*4+0x101809B4]
 *  10180945   8D49 00          LEA ECX,DWORD PTR DS:[ECX]
 *  10180948   AB               STOS DWORD PTR ES:[EDI]
 *  10180949   0918             OR DWORD PTR DS:[EAX],EBX
 *  1018094B   1098 09181090    ADC BYTE PTR DS:[EAX+0x90101809],BL
 *  10180951   0918             OR DWORD PTR DS:[EAX],EBX
 *  10180953   1088 09181080    ADC BYTE PTR DS:[EAX+0x80101809],CL
 *  10180959   0918             OR DWORD PTR DS:[EAX],EBX
 *  1018095B   1078 09          ADC BYTE PTR DS:[EAX+0x9],BH
 *  1018095E   1810             SBB BYTE PTR DS:[EAX],DL
 *  10180960   70 09            JO SHORT RGSS301.1018096B
 *  10180962   1810             SBB BYTE PTR DS:[EAX],DL
 *  10180964   68 0918108B      PUSH 0x8B101809
 *  10180969   44               INC ESP
 *  1018096A   8EE4             MOV FS,SP                                ; Modification of segment register
 *  1018096C   89448F E4        MOV DWORD PTR DS:[EDI+ECX*4-0x1C],EAX
 *  10180970   8B448E E8        MOV EAX,DWORD PTR DS:[ESI+ECX*4-0x18]
 *  10180974   89448F E8        MOV DWORD PTR DS:[EDI+ECX*4-0x18],EAX
 *  10180978   8B448E EC        MOV EAX,DWORD PTR DS:[ESI+ECX*4-0x14]
 *  1018097C   89448F EC        MOV DWORD PTR DS:[EDI+ECX*4-0x14],EAX
 *  10180980   8B448E F0        MOV EAX,DWORD PTR DS:[ESI+ECX*4-0x10]
 *  10180984   89448F F0        MOV DWORD PTR DS:[EDI+ECX*4-0x10],EAX
 *  10180988   8B448E F4        MOV EAX,DWORD PTR DS:[ESI+ECX*4-0xC]
 *  1018098C   89448F F4        MOV DWORD PTR DS:[EDI+ECX*4-0xC],EAX
 *  10180990   8B448E F8        MOV EAX,DWORD PTR DS:[ESI+ECX*4-0x8]
 *  10180994   89448F F8        MOV DWORD PTR DS:[EDI+ECX*4-0x8],EAX
 *  10180998   8B448E FC        MOV EAX,DWORD PTR DS:[ESI+ECX*4-0x4]
 *  1018099C   89448F FC        MOV DWORD PTR DS:[EDI+ECX*4-0x4],EAX
 *  101809A0   8D048D 00000000  LEA EAX,DWORD PTR DS:[ECX*4]
 *  101809A7   03F0             ADD ESI,EAX
 *  101809A9   03F8             ADD EDI,EAX
 *  101809AB   FF2495 B4091810  JMP DWORD PTR DS:[EDX*4+0x101809B4]
 *  101809B2   8BFF             MOV EDI,EDI
 *  101809B4   C409             LES ECX,FWORD PTR DS:[ECX]               ; Modification of segment register
 *  101809B6   1810             SBB BYTE PTR DS:[EAX],DL
 *  101809B8   CC               INT3
 *  101809B9   0918             OR DWORD PTR DS:[EAX],EBX
 *  101809BB   10D8             ADC AL,BL
 *  101809BD   0918             OR DWORD PTR DS:[EAX],EBX
 *  101809BF   10EC             ADC AH,CH
 *  101809C1   0918             OR DWORD PTR DS:[EAX],EBX
 *  101809C3   108B 45085E5F    ADC BYTE PTR DS:[EBX+0x5F5E0845],CL
 *  101809C9   C9               LEAVE
 *  101809CA   C3               RETN
 *  101809CB   90               NOP
 *  101809CC   8A06             MOV AL,BYTE PTR DS:[ESI]
 *  101809CE   8807             MOV BYTE PTR DS:[EDI],AL
 *  101809D0   8B45 08          MOV EAX,DWORD PTR SS:[EBP+0x8]
 *  101809D3   5E               POP ESI
 *  101809D4   5F               POP EDI
 *  101809D5   C9               LEAVE
 *  101809D6   C3               RETN
 *  101809D7   90               NOP
 *  101809D8   8A06             MOV AL,BYTE PTR DS:[ESI]
 *  101809DA   8807             MOV BYTE PTR DS:[EDI],AL
 *  101809DC   8A46 01          MOV AL,BYTE PTR DS:[ESI+0x1]
 *  101809DF   8847 01          MOV BYTE PTR DS:[EDI+0x1],AL
 *  101809E2   8B45 08          MOV EAX,DWORD PTR SS:[EBP+0x8]
 *  101809E5   5E               POP ESI
 *  101809E6   5F               POP EDI
 *  101809E7   C9               LEAVE
 *  101809E8   C3               RETN
 *  101809E9   8D49 00          LEA ECX,DWORD PTR DS:[ECX]
 *  101809EC   8A06             MOV AL,BYTE PTR DS:[ESI]
 *  101809EE   8807             MOV BYTE PTR DS:[EDI],AL
 *  101809F0   8A46 01          MOV AL,BYTE PTR DS:[ESI+0x1]
 *  101809F3   8847 01          MOV BYTE PTR DS:[EDI+0x1],AL
 *  101809F6   8A46 02          MOV AL,BYTE PTR DS:[ESI+0x2]
 *  101809F9   8847 02          MOV BYTE PTR DS:[EDI+0x2],AL
 *  101809FC   8B45 08          MOV EAX,DWORD PTR SS:[EBP+0x8]
 *  101809FF   5E               POP ESI
 *  10180A00   5F               POP EDI
 *  10180A01   C9               LEAVE
 *  10180A02   C3               RETN
 *  10180A03   90               NOP
 *  10180A04   8D7431 FC        LEA ESI,DWORD PTR DS:[ECX+ESI-0x4]
 *  10180A08   8D7C39 FC        LEA EDI,DWORD PTR DS:[ECX+EDI-0x4]
 *  10180A0C   F7C7 03000000    TEST EDI,0x3
 *  10180A12   75 24            JNZ SHORT RGSS301.10180A38
 *  10180A14   C1E9 02          SHR ECX,0x2
 *  10180A17   83E2 03          AND EDX,0x3
 *  10180A1A   83F9 08          CMP ECX,0x8
 *  10180A1D   72 0D            JB SHORT RGSS301.10180A2C
 *  10180A1F   FD               STD
 *  10180A20   F3:A5            REP MOVS DWORD PTR ES:[EDI],DWORD PTR DS>
 *  10180A22   FC               CLD
 *  10180A23   FF2495 500B1810  JMP DWORD PTR DS:[EDX*4+0x10180B50]
 *  10180A2A   8BFF             MOV EDI,EDI
 *  10180A2C   F7D9             NEG ECX
 *  10180A2E   FF248D 000B1810  JMP DWORD PTR DS:[ECX*4+0x10180B00]
 *  10180A35   8D49 00          LEA ECX,DWORD PTR DS:[ECX]
 *  10180A38   8BC7             MOV EAX,EDI
 *  10180A3A   BA 03000000      MOV EDX,0x3
 *  10180A3F   83F9 04          CMP ECX,0x4
 *  10180A42   72 0C            JB SHORT RGSS301.10180A50
 *  10180A44   83E0 03          AND EAX,0x3
 *  10180A47   2BC8             SUB ECX,EAX
 *  10180A49   FF2485 540A1810  JMP DWORD PTR DS:[EAX*4+0x10180A54]
 *  10180A50   FF248D 500B1810  JMP DWORD PTR DS:[ECX*4+0x10180B50]
 *  10180A57   90               NOP
 *  10180A58   64:0A18          OR BL,BYTE PTR FS:[EAX]
 *  10180A5B   1088 0A1810B0    ADC BYTE PTR DS:[EAX+0xB010180A],CL
 *  10180A61   0A18             OR BL,BYTE PTR DS:[EAX]
 *  10180A63   108A 460323D1    ADC BYTE PTR DS:[EDX+0xD1230346],CL
 *  10180A69   8847 03          MOV BYTE PTR DS:[EDI+0x3],AL
 *  10180A6C   83EE 01          SUB ESI,0x1
 *  10180A6F   C1E9 02          SHR ECX,0x2
 *  10180A72   83EF 01          SUB EDI,0x1
 *  10180A75   83F9 08          CMP ECX,0x8
 *  10180A78  ^72 B2            JB SHORT RGSS301.10180A2C
 *  10180A7A   FD               STD
 *  10180A7B   F3:A5            REP MOVS DWORD PTR ES:[EDI],DWORD PTR DS>
 *  10180A7D   FC               CLD
 *  10180A7E   FF2495 500B1810  JMP DWORD PTR DS:[EDX*4+0x10180B50]
 *  10180A85   8D49 00          LEA ECX,DWORD PTR DS:[ECX]
 *  10180A88   8A46 03          MOV AL,BYTE PTR DS:[ESI+0x3]
 *  10180A8B   23D1             AND EDX,ECX
 *  10180A8D   8847 03          MOV BYTE PTR DS:[EDI+0x3],AL
 *  10180A90   8A46 02          MOV AL,BYTE PTR DS:[ESI+0x2]
 *  10180A93   C1E9 02          SHR ECX,0x2
 *  10180A96   8847 02          MOV BYTE PTR DS:[EDI+0x2],AL
 *  10180A99   83EE 02          SUB ESI,0x2
 *  10180A9C   83EF 02          SUB EDI,0x2
 *  10180A9F   83F9 08          CMP ECX,0x8
 *  10180AA2  ^72 88            JB SHORT RGSS301.10180A2C
 *  10180AA4   FD               STD
 *  10180AA5   F3:A5            REP MOVS DWORD PTR ES:[EDI],DWORD PTR DS>
 *  10180AA7   FC               CLD
 *  10180AA8   FF2495 500B1810  JMP DWORD PTR DS:[EDX*4+0x10180B50]
 *  10180AAF   90               NOP
 *  10180AB0   8A46 03          MOV AL,BYTE PTR DS:[ESI+0x3]
 *  10180AB3   23D1             AND EDX,ECX
 *  10180AB5   8847 03          MOV BYTE PTR DS:[EDI+0x3],AL
 *  10180AB8   8A46 02          MOV AL,BYTE PTR DS:[ESI+0x2]
 *  10180ABB   8847 02          MOV BYTE PTR DS:[EDI+0x2],AL
 *  10180ABE   8A46 01          MOV AL,BYTE PTR DS:[ESI+0x1]
 *  10180AC1   C1E9 02          SHR ECX,0x2
 *  10180AC4   8847 01          MOV BYTE PTR DS:[EDI+0x1],AL
 *  10180AC7   83EE 03          SUB ESI,0x3
 *  10180ACA   83EF 03          SUB EDI,0x3
 *  10180ACD   83F9 08          CMP ECX,0x8
 *  10180AD0  ^0F82 56FFFFFF    JB RGSS301.10180A2C
 *  10180AD6   FD               STD
 *  10180AD7   F3:A5            REP MOVS DWORD PTR ES:[EDI],DWORD PTR DS>
 *  10180AD9   FC               CLD
 *  10180ADA   FF2495 500B1810  JMP DWORD PTR DS:[EDX*4+0x10180B50]
 *  10180AE1   8D49 00          LEA ECX,DWORD PTR DS:[ECX]
 *  10180AE4   04 0B            ADD AL,0xB
 *  10180AE6   1810             SBB BYTE PTR DS:[EAX],DL
 *  10180AE8   0C 0B            OR AL,0xB
 *  10180AEA   1810             SBB BYTE PTR DS:[EAX],DL
 *  10180AEC   14 0B            ADC AL,0xB
 *  10180AEE   1810             SBB BYTE PTR DS:[EAX],DL
 *  10180AF0   1C 0B            SBB AL,0xB
 *  10180AF2   1810             SBB BYTE PTR DS:[EAX],DL
 *  10180AF4   24 0B            AND AL,0xB
 *  10180AF6   1810             SBB BYTE PTR DS:[EAX],DL
 *  10180AF8   2C 0B            SUB AL,0xB
 *  10180AFA   1810             SBB BYTE PTR DS:[EAX],DL
 *  10180AFC   34 0B            XOR AL,0xB
 *  10180AFE   1810             SBB BYTE PTR DS:[EAX],DL
 *  10180B00   47               INC EDI
 *  10180B01   0B18             OR EBX,DWORD PTR DS:[EAX]
 *  10180B03   108B 448E1C89    ADC BYTE PTR DS:[EBX+0x891C8E44],CL
 *  10180B09   44               INC ESP
 *  10180B0A   8F               ???                                      ; Unknown command
 *  10180B0B   1C 8B            SBB AL,0x8B
 *  10180B0D   44               INC ESP
 *  10180B0E   8E18             MOV DS,WORD PTR DS:[EAX]                 ; Modification of segment register
 *  10180B10   89448F 18        MOV DWORD PTR DS:[EDI+ECX*4+0x18],EAX
 *  10180B14   8B448E 14        MOV EAX,DWORD PTR DS:[ESI+ECX*4+0x14]
 *  10180B18   89448F 14        MOV DWORD PTR DS:[EDI+ECX*4+0x14],EAX
 *  10180B1C   8B448E 10        MOV EAX,DWORD PTR DS:[ESI+ECX*4+0x10]
 *  10180B20   89448F 10        MOV DWORD PTR DS:[EDI+ECX*4+0x10],EAX
 *  10180B24   8B448E 0C        MOV EAX,DWORD PTR DS:[ESI+ECX*4+0xC]
 *  10180B28   89448F 0C        MOV DWORD PTR DS:[EDI+ECX*4+0xC],EAX
 *  10180B2C   8B448E 08        MOV EAX,DWORD PTR DS:[ESI+ECX*4+0x8]
 *  10180B30   89448F 08        MOV DWORD PTR DS:[EDI+ECX*4+0x8],EAX
 *  10180B34   8B448E 04        MOV EAX,DWORD PTR DS:[ESI+ECX*4+0x4]
 *  10180B38   89448F 04        MOV DWORD PTR DS:[EDI+ECX*4+0x4],EAX
 *  10180B3C   8D048D 00000000  LEA EAX,DWORD PTR DS:[ECX*4]
 *  10180B43   03F0             ADD ESI,EAX
 *  10180B45   03F8             ADD EDI,EAX
 *  10180B47   FF2495 500B1810  JMP DWORD PTR DS:[EDX*4+0x10180B50]
 *  10180B4E   8BFF             MOV EDI,EDI
 *  10180B50   60               PUSHAD
 *  10180B51   0B18             OR EBX,DWORD PTR DS:[EAX]
 *  10180B53   1068 0B          ADC BYTE PTR DS:[EAX+0xB],CH
 *  10180B56   1810             SBB BYTE PTR DS:[EAX],DL
 *  10180B58   78 0B            JS SHORT RGSS301.10180B65
 *  10180B5A   1810             SBB BYTE PTR DS:[EAX],DL
 *  10180B5C   8C0B             MOV WORD PTR DS:[EBX],CS
 *  10180B5E   1810             SBB BYTE PTR DS:[EAX],DL
 *  10180B60   8B45 08          MOV EAX,DWORD PTR SS:[EBP+0x8]
 *  10180B63   5E               POP ESI
 *  10180B64   5F               POP EDI
 *  10180B65   C9               LEAVE
 *  10180B66   C3               RETN
 *  10180B67   90               NOP
 *  10180B68   8A46 03          MOV AL,BYTE PTR DS:[ESI+0x3]
 *  10180B6B   8847 03          MOV BYTE PTR DS:[EDI+0x3],AL
 *  10180B6E   8B45 08          MOV EAX,DWORD PTR SS:[EBP+0x8]
 *  10180B71   5E               POP ESI
 *  10180B72   5F               POP EDI
 *  10180B73   C9               LEAVE
 *  10180B74   C3               RETN
 *  10180B75   8D49 00          LEA ECX,DWORD PTR DS:[ECX]
 *  10180B78   8A46 03          MOV AL,BYTE PTR DS:[ESI+0x3]
 *  10180B7B   8847 03          MOV BYTE PTR DS:[EDI+0x3],AL
 *  10180B7E   8A46 02          MOV AL,BYTE PTR DS:[ESI+0x2]
 *  10180B81   8847 02          MOV BYTE PTR DS:[EDI+0x2],AL
 *  10180B84   8B45 08          MOV EAX,DWORD PTR SS:[EBP+0x8]
 *  10180B87   5E               POP ESI
 *  10180B88   5F               POP EDI
 *  10180B89   C9               LEAVE
 *  10180B8A   C3               RETN
 *  10180B8B   90               NOP
 *  10180B8C   8A46 03          MOV AL,BYTE PTR DS:[ESI+0x3]
 *  10180B8F   8847 03          MOV BYTE PTR DS:[EDI+0x3],AL
 *  10180B92   8A46 02          MOV AL,BYTE PTR DS:[ESI+0x2]
 *  10180B95   8847 02          MOV BYTE PTR DS:[EDI+0x2],AL
 *  10180B98   8A46 01          MOV AL,BYTE PTR DS:[ESI+0x1]
 *  10180B9B   8847 01          MOV BYTE PTR DS:[EDI+0x1],AL
 *  10180B9E   8B45 08          MOV EAX,DWORD PTR SS:[EBP+0x8]
 *  10180BA1   5E               POP ESI
 *  10180BA2   5F               POP EDI
 *  10180BA3   C9               LEAVE
 *  10180BA4   C3               RETN
 *  10180BA5   CC               INT3
 *  10180BA6   CC               INT3
 *  10180BA7   CC               INT3
 *  10180BA8   CC               INT3
 *  10180BA9   CC               INT3
 *  10180BAA   CC               INT3
 *  10180BAB   CC               INT3
 */

bool attach() // attach scenario
{
  QString module = Private::getDllModuleName();
  if (module.isEmpty())
    return false;

  DOUT("dll =" << module);

  ulong startAddress, stopAddress;
  if (!Engine::getMemoryRange((LPCWSTR)module.utf16(), &startAddress, &stopAddress))
    return false;

  const quint8 bytes[] = {
    0x8b,0x54,0x24, 0x24,           // 1004155c   8b5424 24        mov edx,dword ptr ss:[esp+0x24]
    0x8b,0x02,                      // 10041560   8b02             mov eax,dword ptr ds:[edx]
    0x8b,0xc8,                      // 10041562   8bc8             mov ecx,eax
    0x83,0xc4, 0x0c,                // 10041564   83c4 0c          add esp,0xc
    0x81,0xe1, 0x00,0x20,0x00,0x00  // 10041567   81e1 00200000    and ecx,0x2000
  };
  ulong addr = MemDbg::findBytes(bytes, sizeof(bytes), startAddress, stopAddress);
  if (!addr)
    return false;
  addr = MemDbg::findEnclosingAlignedFunction(addr);
  if (!addr)
    return false;
  //addr = MemDbg::findPushAddress(addr, startAddress, stopAddress);
  //addr = 0x10041557;
  //addr = 0x100414a0;
  //addr = 0x10056BC0;
  return winhook::hook_before(addr, Private::hookBefore); //, Private::hookAfter);
}

} // namespace AgsHookW

#if 0

/**
 *  Sample game: Mogeko Castle with RGSS 3.01
 *  0x10036758: LOAD
 *  0x1004155c: DATA
 *
 *  Text accessed character by character
 *  0x10036463: LOAD    character by character
 *
 *  0x100378ed: $100
 *  0x100378ed: キャンセル
 *
 *  0x10038a44: 駅のホーム
 */
namespace DebugHook {

bool beforeStrcpy(winhook::hook_stack *s)
{
  auto arg = (LPCSTR)s->stack[2]; // arg2
  auto sig = s->stack[0]; // retaddr
  enum { role = Engine::OtherRole };
  if (!::strstr(arg, "\xe3\x82\xaa\xe3\x83\xac\xe3\x83\xb3\xe7\x97\x94"))
    return true;
  QString text = QString::fromUtf8((LPCSTR)arg, s->stack[3]);
  //if (!text.isEmpty() && text[0].unicode() >= 128 && text.size() == 5)
  //if (!text.isEmpty() && sig == 0x100378ed)
  EngineController::instance()->dispatchTextW(text, sig, role);
  return true;
}

bool attach()
{
  winhook::hook_before(0x10180840, beforeStrcpy);
  return true;
}

} // namespace DebugHook

#endif // 0


} // unnamed namespace

/** Public class */

bool RGSSEngine::attach()
{
  if (!RGSS3Hook::attach())
    return false;

  //DebugHook::attach();

  HijackManager::instance()->attachFunction((ulong)::GetGlyphOutlineW); // in order to customize font
  return true;
}

QString RGSSEngine::textFilter(const QString &text, int role)
{
  Q_UNUSED(role);
  if (!text.contains('\\'))
    return text;
  QString ret = text;
  static QRegExp rx_("\\\\[0-9A-Z.\\[\\]]+");
  ret.remove(rx_);
  return ret;
}

// EOF
/*
  //typedef char *(* hook_fun_t)(char *dst, const char *src, size_t size);
  //hook_fun_t oldHookFun;
  char *oldHookFun(char *dst, const char *src, size_t size)
  {
    ::memcpy(dst, src, size);
    if (src[size] == 0)
      dst[size] = 0;
    return dst;
  }

  // Similar to strncpy
  // Copy size of src to dst and append '\0' at the end
  // Return dst
  char *newHookFun(char *dst, const char *src, size_t size)
  {
    if (size) {
      QString oldText = QString::fromUtf8(src, size);
      if (oldText[0].unicode() >= 128) {

        enum { role = Engine::OtherRole };
        ulong split = (ulong)_ReturnAddress();
        auto sig = Engine::hashThreadSignature(role, split);

        QString newText = EngineController::instance()->dispatchTextW(oldText, sig, role);
        if (newText != oldText) {
          static QByteArray data;
          data = newText.toUtf8();
          return oldHookFun(dst, data.constData(), data.size());
        }
      }
    }
    return oldHookFun(dst, src, size);
  }
*/
