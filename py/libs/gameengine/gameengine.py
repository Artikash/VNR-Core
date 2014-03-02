# coding: utf8
# gameengine.py
# 10/3/2013 jichi
# Windows only

from sakurakit.skdebug import dprint

# CHECKPOINT: clean up the logic
#def match

def tryBaldrEngine(pid):
  """
  @param  pid  long
  @return  bool
  """
  ret = False

  import os
  from sakurakit import skwin

  name = skwin.get_process_name(pid)
  #dprint("process = %s" % name)
  if name == 'bsz.exe': # BALDRSKY ZERO
    from gamedebugger import GameDebugger
    dbg = GameDebugger(pid)
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
        ret = th.addHook(code, name="BALDR")

  dprint(ret)
  return ret

def process(pid):
  """
  @param  pid  long
  @return  bool
  """
  if tryBaldrEngine(pid):
    return True
  return False

# EOF
