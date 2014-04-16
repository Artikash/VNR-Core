# coding: utf8
# engines.py
# 10/3/2013 jichi
# Windows only

if __name__ == '__main__': # DEBUG
  import sys
  sys.path.append("..")

from sakurakit.skdebug import dprint
#from sakurakit.skclass import memoized

ENINES = []
def engines():
  """
  @yield  Engine
  """
  if not ENINES:
    ENGINES = [
      UnityEngine(),
    ]
  return ENGINES

class Engine(object): # placeholder

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
    import os
    path = self.getAppPath(pid)
    if path:
      return os.path.dirname(path)

  def globAppDirectory(self, pid, pattern):
    """
    @param  pid  long
    @param  pattern  str
    @return  [unicode path] or None
    """
    path = self.getAppDirectory(pid)
    if path:
      import os
      from glob import glob
      return glob(os.path.join(path, pattern))

class UnityEngine(Engine):

  NAME = "Unity" # str

  def match(self, pid): # override
    return bool(self.globAppDirectory(pid,
        '*/Managed/UnityEngine.dll'))

  def inject(self, pid): # override
    from gamedebugger import GameDebugger
    dbg = GameDebugger(pid)
    ret = False
    if dbg.active():
      # See (ok123): http://9baka.com/read.php?tid=411756
      #pattern = 0x90, 0xff, 0x50, 0x3c, 0x83, 0xc4, 0x20, 0x8b, 0x45, 0xec
      #addr = dbg.searchbytes(pattern)
      pattern = 0x90ff503c83c4208b45ec
      length = 0x15000000   # it is usually around 0x7000000. larger enough to cover BALDRSKY ZERO
      addr = dbg.searchhex(pattern, length=length)
      if addr > 0:
        code = "/HQN4@%x" % addr
        from texthook import texthook
        th = texthook.global_()
        ret = th.addHook(code, name=self.NAME)
        #ret = th.addHook(code, name="BALDR")
    dprint(ret)
    return ret

# EOF
