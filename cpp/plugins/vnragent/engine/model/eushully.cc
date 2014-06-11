// eushully.cc
// 6/1/2014 jichi
// See: http://bbs.sumisora.org/read.php?tid=11044256
#include "engine/model/eushully.h"
#include "engine/enginedef.h"
#include "engine/engineenv.h"
#include "memdbg/memsearch.h"
#include <qt_windows.h>

/**
 *  Find the last function call of GetTextExtentPoint32A
 *  The function call must after int3
 */
bool EushullyEngine::attach()
{
  ulong startAddress,
        stopAddress;
  if (!Engine::getCurrentMemoryRange(&startAddress, &stopAddress))
    return false;
  // Find the last function call of GetTextExtentPoint32A
  // The function call must after int3
  ulong addr = MemDbg::findLastCallerAddressAfterInt3((ulong)::GetTextExtentPoint32A, startAddress, stopAddress);
  //ulong addr = ::searchEushully(startAddress, stopAddress);
  //addr = 0x451170; // 姫狩りダンジョンマイスター体験版
  //addr = 0x468fa0; // 天秤のLaDEA体験版
  //dmsg(addr);
  return addr && hookAddress(addr);
}

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
void EushullyEngine::hookFunction(HookStack *stack)
{
  static QByteArray data_; // persistent storage, which makes this function not thread-safe

  // All threads including character names are linked together
  enum { role = Engine::ScenarioRole, signature = Engine::ScenarioThreadSignature };

  LPCSTR text2 = (LPCSTR)stack->args[1]; // arg2

  data_ = instance()->dispatchTextA(text2, signature, role);
  //dmsg(QString::fromLocal8Bit(ret));
  stack->args[1] = (ulong)data_.constData(); // arg2
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
  //const BYTE ins[] = { // size = 14
  //  0x01,0x53, 0x58,                // 0153 58          add dword ptr ds:[ebx+58],edx
  //  0x8b,0x95, 0x34,0xfd,0xff,0xff, // 8b95 34fdffff    mov edx,dword ptr ss:[ebp-2cc]
  //  0x8b,0x43, 0x58,                // 8b43 58          mov eax,dword ptr ds:[ebx+58]
  //  0x3b,0xd7                       // 3bd7             cmp edx,edi ; hook here
  //};
  //enum { cur_ins_size = 2 };
  //enum { hook_offset = sizeof(ins) - cur_ins_size }; // = 14 - 2  = 12, current inst is the last one
  const BYTE ins1[] = {
    0x3b,0xd7, // 013baf32  |. 3bd7       |cmp edx,edi ; jichi: ITH hook here, char saved in edi
    0x75,0x4b  // 013baf34  |. 75 4b      |jnz short siglusen.013baf81
  };
  //enum { hook_offset = 0 };
  DWORD range1 = min(stopAddress - startAddress, Engine::MaximumMemoryRange);
  DWORD reladdr = MemDbg::searchPattern(startAddress, range1, ins1, sizeof(ins1));

  if (!reladdr)
    //ConsoleOutput("vnreng:Siglus2: pattern not found");
    return 0;

  const BYTE ins2[] = {
    0x55,      // 013bac70  /$ 55       push ebp ; jichi: function starts
    0x8b,0xec, // 013bac71  |. 8bec     mov ebp,esp
    0x6a,0xff  // 013bac73  |. 6a ff    push -0x1
  };
  enum { range2 = 0x300 }; // 0x013baf32  -0x013bac70 = 706 = 0x2c2
  DWORD addr = startAddress + reladdr - range2;
  reladdr = MemDbg::searchPattern(addr, range2, ins2, sizeof(ins2));
  if (!reladdr)
    //ConsoleOutput("vnreng:Siglus2: pattern not found");
    return 0;
  addr += reladdr;
  return addr;
}

#endif // 0
