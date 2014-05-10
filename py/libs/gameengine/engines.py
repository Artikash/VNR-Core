# coding: utf8
# engines.py
# 10/3/2013 jichi
# Windows only

if __name__ == '__main__': # DEBUG
  import sys
  sys.path.append("..")

import os
from sakurakit.skdebug import dprint
#from sakurakit.skclass import memoized

ENINES = []
def engines():
  """
  @yield  Engine
  """
  if not ENINES:
    ENGINES = [
      MonoEngine(),
    ]
  return ENGINES

class Engine(object): # placeholder
  NAME = ''
  ENCODING = ''

  def name(self): return self.NAME
  def encoding(self): return self.ENCODING

  def addHook(self, code, name=''): # str, str -> bool
   from texthook import texthook
   return texthook.global_().addHook(code, name=name or self.name())

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

  def globAppDirectory(self, pattern, pid):
    """
    @param  pattern  str
    @param  pid  long
    @return  [unicode path] or None
    """
    path = self.getAppDirectory(pid)
    if path:
      from glob import glob
      return glob(os.path.join(path, pattern))

  def globAppDirectories(self, patterns, pid):
    """Return all paths or None if failed
    @param  pattern  [str]
    @param  pid  long
    @return  [unicode path] or None  if return list, it must have the same length as patterns
    """
    path = self.getAppDirectory(pid)
    if path:
      from glob import glob
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
  ENCODING = "utf-16" # str, override

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

      addr = dbg.search_module_memory("mono.dll", pattern)
      if addr > 0:
        code = "/HWN-8*0:3C@%x" % addr
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
