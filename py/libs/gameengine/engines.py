# coding: utf8
# engines.py
# 10/3/2013 jichi
# Windows only

if __name__ == '__main__': # DEBUG
  import sys
  sys.path.append("..")

import os, re
from glob import glob
from sakurakit.skdebug import dprint
#from sakurakit.skclass import memoized

SJIS_ENCODING = 'shift-jis'
UTF8_ENCODING = 'utf8'
UTF16_ENCODING = 'utf-16'

ENINES = []
def engines():
  """
  @yield  Engine
  """
  if not ENINES:
    ENGINES = [
      MonoEngine(),
      GXPEngine(),
      AliceEngine(),
    ]
  return ENGINES

class Engine(object): # placeholder
  NAME = ''
  ENCODING = ''

  DEBUG = False
  #DEBUG = True

  def name(self): return self.NAME
  def encoding(self): return self.ENCODING

  def addHook(self, code, name=''): # str, str -> bool
   from texthook import texthook
   return texthook.global_().addHook(code, name=name or self.name(), verbose=self.DEBUG)

  # Pure virtual functions
  def match(self, pid):
    """
    @param  pid  long
    @return  bool
    """
    return False

  def inject(self, pid):
    """
    @param  pid  long
    @return  bool
    """
    return False

  # Helpers

  def getAppName(self, pid):
    """
    @param  pid  long
    @return  unicode or None
    """
    from sakurakit import skwin
    return skwin.get_process_name(pid)

  def getAppPath(self, pid):
    """
    @param  pid  long
    @return  unicode or None
    """
    from sakurakit import skwin
    return skwin.get_process_path(pid)

  def getAppDirectory(self, pid):
    """
    @param  pid  long
    @return  unicode or None
    """
    path = self.getAppPath(pid)
    if path:
      return os.path.dirname(path)

  def exists(self, pattern, pid):
    """
    @param  pattern  unicode
    @param  pid  long
    @return  bool
    """
    path = self.getAppDirectory(pid)
    return bool(path) and os.path.exists(os.path.join(path, pattern))

  def globAppDirectory(self, pattern, pid):
    """
    @param  pattern  str
    @param  pid  long
    @return  [unicode path] or None
    """
    path = self.getAppDirectory(pid)
    if path:
      return glob(os.path.join(path, pattern))

  def globAppDirectories(self, patterns, pid):
    """Return all paths or None if failed
    @param  pattern  [str]
    @param  pid  long
    @return  [unicode path] or None  if return list, it must have the same length as patterns
    """
    path = self.getAppDirectory(pid)
    if path:
      ret = []
      for pat in patterns:
        r = glob(os.path.join(path, pat))
        if not r:
          return None
        ret.append(r)
      return ret

#  jichi 4/21/2014: Mono (Unity3D)
#  See (ok123): http://sakuradite.com/topic/214
#  Pattern: 33DB66390175
#
#  Example: /HWN-8*0:3C@ mono.dll search 33DB66390175
#  - length_offset: 1
#  - module: 1690566707 = 0x64c40033
#  - off: 4294967284 = 0xfffffff4 = -0xc
#  - split: 60 = 0x3c
#  - type: 1114 = 0x45a
#
#  Function starts:
#  1003b818  /$ 55             push ebp
#  1003b819  |. 8bec           mov ebp,esp
#  1003b81b  |. 51             push ecx
#  1003b81c  |. 807d 10 00     cmp byte ptr ss:[ebp+0x10],0x0
#  1003b820  |. 8b50 08        mov edx,dword ptr ds:[eax+0x8]
#  1003b823  |. 53             push ebx
#  1003b824  |. 8b5d 08        mov ebx,dword ptr ss:[ebp+0x8]
#  1003b827  |. 56             push esi
#  1003b828  |. 8b75 0c        mov esi,dword ptr ss:[ebp+0xc]
#  1003b82b  |. 57             push edi
#  1003b82c  |. 8d78 0c        lea edi,dword ptr ds:[eax+0xc]
#  1003b82f  |. 897d 08        mov dword ptr ss:[ebp+0x8],edi
#  1003b832  |. 74 44          je short mono.1003b878
#  1003b834  |. 2bf2           sub esi,edx
#  1003b836  |. 03f1           add esi,ecx
#  1003b838  |. 894d 10        mov dword ptr ss:[ebp+0x10],ecx
#  1003b83b  |. 8975 08        mov dword ptr ss:[ebp+0x8],esi
#  1003b83e  |. 3bce           cmp ecx,esi
#  1003b840  |. 7f 67          jg short mono.1003b8a9
#  1003b842  |. 8d4c4b 0c      lea ecx,dword ptr ds:[ebx+ecx*2+0xc]
#  1003b846  |> 0fb707         /movzx eax,word ptr ds:[edi]
#  1003b849  |. 33db           |xor ebx,ebx    ; jichi hook here
#  1003b84b  |. 66:3901        |cmp word ptr ds:[ecx],ax
#  1003b84e  |. 75 16          |jnz short mono.1003b866
#  1003b850  |. 8bf1           |mov esi,ecx
#  1003b852  |> 43             |/inc ebx
#  1003b853  |. 83c6 02        ||add esi,0x2
#  1003b856  |. 3bda           ||cmp ebx,edx
#  1003b858  |. 74 19          ||je short mono.1003b873
#  1003b85a  |. 66:8b06        ||mov ax,word ptr ds:[esi]
#  1003b85d  |. 66:3b045f      ||cmp ax,word ptr ds:[edi+ebx*2]
#  1003b861  |.^74 ef          |\je short mono.1003b852
#  1003b863  |. 8b75 08        |mov esi,dword ptr ss:[ebp+0x8]
#  1003b866  |> ff45 10        |inc dword ptr ss:[ebp+0x10]
#  1003b869  |. 83c1 02        |add ecx,0x2
#  1003b86c  |. 3975 10        |cmp dword ptr ss:[ebp+0x10],esi
#  1003b86f  |.^7e d5          \jle short mono.1003b846
class MonoEngine(Engine):

  NAME = "Mono" # str, override
  ENCODING = UTF16_ENCODING # str, override

  def match(self, pid): # override
    return bool(self.globAppDirectory('*/Mono/mono.dll', pid))

  def inject(self, pid): # override
    from gamedebugger import GameDebugger
    dbg = GameDebugger(pid)
    ret = False
    if dbg.active():
      # See (ok123): http://9baka.com/read.php?tid=411756
      #pattern = 0x90, 0xff, 0x50, 0x3c, 0x83, 0xc4, 0x20, 0x8b, 0x45, 0xec
      #addr = dbg.searchbytes(pattern)
      pattern = 0x33DB66390175
      #length = 0x50000 # larger than 0x3b849

      addr = dbg.search_module_memory(pattern, "mono.dll")
      if addr > 0:
        code = "/HWN-8*:3C@%x" % addr
        ret = self.addHook(code)
    dprint(ret)
    return ret

# 4/27/2015
# See also GXP comments in engine.cc
#
# Sample game: ウルスラグナ
# Debug step:
# 1. find the following location
#    00A78144   66:833C70 00     CMP WORD PTR DS:[EAX+ESI*2],0x0
#    i.e. 0x66833C7000
#    There are several matches, the first one is used.
# 2. Use Ollydbg to debug step by step until the first function call is encountered
#    Then, the text character is directly on the stack
#
#  Issues: It cannot extract character name.
#
#  00A7883A   24 3C            AND AL,0x3C
#  00A7883C   50               PUSH EAX
#  00A7883D   C74424 4C 000000>MOV DWORD PTR SS:[ESP+0x4C],0x0
#  00A78845   0F5B             ???                                      ; Unknown command
#  00A78847   C9               LEAVE
#  00A78848   F3:0F114424 44   MOVSS DWORD PTR SS:[ESP+0x44],XMM0
#  00A7884E   F3:0F114C24 48   MOVSS DWORD PTR SS:[ESP+0x48],XMM1
#  00A78854   E8 37040000      CALL .00A78C90  ; jichi: here's the target function to hook to, text char on the stack[0]
#  00A78859   A1 888EDD00      MOV EAX,DWORD PTR DS:[0xDD8E88]
#  00A7885E   A8 01            TEST AL,0x1
#  00A78860   75 30            JNZ SHORT .00A78892
#  00A78862   83C8 01          OR EAX,0x1
#  00A78865   A3 888EDD00      MOV DWORD PTR DS:[0xDD8E88],EAX
#
# 5/2/2015: Alternative H-code for the full game: /HWN-4@48806:verethragna.exe
# Try this one if GXP2 fails.
class GXPEngine(Engine):

  NAME = "GXP2" # str, override
  ENCODING = UTF16_ENCODING # str, override

  def match(self, pid): # override
    return bool(self.globAppDirectory('*.gxp', pid))

  def inject(self, pid): # override
    from gamedebugger import GameDebugger
    dbg = GameDebugger(pid)
    ret = False
    if dbg.active():
      #  00A78845   0F5B             ???                                      ; Unknown command
      #  00A78847   C9               LEAVE
      #  00A78848   F3:0F114424 44   MOVSS DWORD PTR SS:[ESP+0x44],XMM0
      #  00A7884E   F3:0F114C24 48   MOVSS DWORD PTR SS:[ESP+0x48],XMM1
      #  00A78854   E8 37040000      CALL .00A78C90  ; jichi: here's the target function to hook to, text char on the stack[0]
      pattern = 0x0f5bc9f30f11442444f30f114c2448e8
      addr = dbg.search_module_memory(pattern)
      if addr > 0:
        addr += 0x00A78854 - 0x00A78845
        # type: USING_UNICODE(W) | NO_CONTEXT (N) | DATA_INDIRECT(*) | FIXING_SPLIT(F)
        # length_offset: 1
        code = "/HWNF*@%x" % addr
        ret = self.addHook(code)
    dprint(ret)
    return ret

# 5/8/2015
# Sample game: イブニクル version 1.0.1
# It is basically the same System43 engine but code moved to AliceRunPatch.dll
#
# See System43 in engine.cc
#   // 9/25/2014 TODO: I should use matchBytes and replace the part after e8 with XX4
#   // i.e. 83c40c5f5eb0015bc20400cccc without leading 0xe8
#   const BYTE ins[] = {  //   005506a9  |. e8 f2fb1600    call rance01.006c02a0 ; hook here
#     0x83,0xc4, 0x0c,    //   005506ae  |. 83c4 0c        add esp,0xc
#     0x5f,               //   005506b1  |. 5f             pop edi
#     0x5e,               //   005506b2  |. 5e             pop esi
#     0xb0, 0x01,         //   005506b3  |. b0 01          mov al,0x1
#     0x5b,               //   005506b5  |. 5b             pop ebx
#     0xc2, 0x04,0x00,    //   005506b6  \. c2 0400        retn 0x4
#     0xcc, 0xcc // patching a few int3 to make sure that this is at the end of the code block
#   };
#   enum { hook_offset = -5 }; // the function call before the ins
#
#   HookParam hp = {};
#   hp.addr = addr;
#   hp.off = 4;
#   hp.split = -0x18;
#   hp.type = NO_CONTEXT|USING_SPLIT|USING_STRING;
#
# TODO: Add alternative hook with character name
# See: http://capita.tistory.com/m/post/256
#   004EEB30   8BCF             MOV ECX,EDI
#   004EEB32   881E             MOV BYTE PTR DS:[ESI],BL
#   004EEB34   E8 67CB0100      CALL .0050B6A0  ; jichi: hook here
#   004EEB39   396C24 28        CMP DWORD PTR SS:[ESP+0x28],EBP
#   004EEB3D   72 0D            JB SHORT .004EEB4C
class AliceEngine(Engine):

  NAME = "AliceRunPatch" # str, override
  ENCODING = SJIS_ENCODING # str, override

  DLL = 'AliceRunPatch.dll'
  def match(self, pid): # override
    return bool(self.exists(self.DLL, pid))

  def inject(self, pid): # override
    from gamedebugger import GameDebugger
    dbg = GameDebugger(pid)
    ret = False
    if dbg.active():
      # Use 4 dots to represent the 8-bit calling address after e8
      pattern = re.compile(r'\xe8....\x83\xc4\x0c\x5f\x5e\xb0\x01\x5b\xc2\x04\x00\xcc\xcc')
      addr = dbg.search_module_memory(pattern, self.DLL)
      if addr > 0:
        code = "/HSN4:-14@%x" % addr
        ret = self.addHook(code)
    dprint(ret)
    return ret

# EOF

# jichi 10/4/2013: Unity3D
# See (ok123): http://9baka.com/read.php?tid=411756
#class UnityEngine(Engine):
#
#  NAME = "Unity" # str
#
#  def match(self, pid): # override
#    return bool(self.globAppDirectory(pid,
#        '*/Managed/UnityEngine.dll'))
#
#  def inject(self, pid): # override
#    from gamedebugger import GameDebugger
#    dbg = GameDebugger(pid)
#    ret = False
#    if dbg.active():
#      #pattern = 0x90, 0xff, 0x50, 0x3c, 0x83, 0xc4, 0x20, 0x8b, 0x45, 0xec
#      #addr = dbg.searchbytes(pattern)
#      pattern = 0x90ff503c83c4208b45ec
#      length = 0x15000000   # it is usually around 0x7000000. larger enough to cover BALDRSKY ZERO
#      addr = dbg.searchhex(pattern, length=length)
#      if addr > 0:
#        code = "/HQN4@%x" % addr
#        from texthook import texthook
#        th = texthook.global_()
#        ret = th.addHook(code, name=self.NAME)
#    dprint(ret)
#    return ret
