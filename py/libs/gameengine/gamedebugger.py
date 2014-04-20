# coding: utf8
# gamedebugger.py
# 10/3/2013 jichi
# Windows only
# See: http://code.google.com/p/paimei/source/browse/trunk/MacOSX/PaiMei-1.1-REV122/build/lib/pydbg/pydbg.py?r=234

if __name__ == '__main__': # DEBUG
  import sys
  sys.path.append("..")

import math
from itertools import imap
from sakurakit.skdebug import dprint, dwarn

# Default values in CheatEngine
#SEARCH_START = 0x0
#SEARCH_STOP = 0x7fffffff
SEARCH_START =  0x400000    # base address for most games
SEARCH_LENGTH = 0x300000    # range to search

class debug(object):
  def __init__(self, pid):
    """
    @param  pid  long
    """
    self.pid = pid

  def __enter__(self):
    self.debugger = GameDebugger(pid)
    if self.debugger.active:
      return self.debugger

  def __exit__(self, *err): pass

def _empty_func(self, *args, **kwargs): pass

class GameDebugger(object):
  def __init__(self, pid):
    """
    @param  pid  long
    """
    self.pid = pid # long

    from pysafedbg import pysafedbg
    self.debugger = pysafedbg() # pydbg

    if self.pid:
      try: self.debugger.attach(self.pid)
      except Exception, e:
        dwarn(e)
        self.pid = 0
    dprint("pass")

  def __del__(self):
    dprint("pass")
    if self.pid:
      try: self.debugger.detach()
      except Exception, e: dwarn(e)

  def active(self):
    """
    Wreturn  bool
    """
    return bool(self.pid)

  def searchhex(self, pattern, start=SEARCH_START, length=SEARCH_LENGTH):
    """Search memory
    @param  pattern  int
    @return  long  addr
    """
    # http://stackoverflow.com/questions/6187699/how-to-convert-integer-value-to-array-of-four-bytes-in-python
    r = xrange(int(math.log(pattern, 2)/8) * 8, -1, -8)
    s = (pattern >> i & 0xff for i in r)
    return self.searchbytes(s, start=start, length=length)

  def searchbytes(self, pattern, start=SEARCH_START, length=SEARCH_LENGTH):
    """Search memory
    @param  pattern  [int]
    @return  long  addr
    """
    #pattern = 0x90ff503c83c4208b45ec
    s = ''.join(imap(chr, pattern))
    return self.searchstring(s, start=start, length=length)

  def searchstring(self, pattern, start=SEARCH_START, length=SEARCH_LENGTH):
    """Search memory
    @param  pattern  str  regular expression
    @return  long  addr
    See:  http://paimei.googlecode.com/svn-history/r234/trunk/MacOSX/PaiMei-1.1-REV122/build/lib/pydbg/pydbg.py
    """
    #if start > stop:
    #  return -1
    #import re
    #rx = re.compile(pattern)
    from pydbg import defines
    skipped = defines.PAGE_GUARD|defines.PAGE_NOACCESS|defines.PAGE_READONLY

    pydbg = self.debugger
    cursor  = start
    # scan through the entire memory range and save a copy of suitable memory blocks.
    stop = start + length
    while cursor < stop:
      try: mbi = pydbg.virtual_query(cursor)
      except: break # out of region
      #if(mbi.BaseAddress):
      #  print ("%08x, size %x " % (mbi.BaseAddress, mbi.RegionSize))

      if not mbi.Protect&skipped:
        # read the raw bytes from the memory block.
        try:
          data = pydbg.read_process_memory(mbi.BaseAddress, mbi.RegionSize)
          offset = data.find(pattern)
          if offset >= 0:
            return mbi.BaseAddress + offset
        except: pass # ignore accessed denied
      cursor += mbi.RegionSize
    return -1

          #m = rx.search(data[loc:])
          #if m:
          #  return mbi.BaseAddress + m.start()
          # Find all matched string
          #loc = 0
          #m = rx.search(data[loc:])
          #while m:
          #  #loc += m.span()[0] + 1
          #  loc += m.start() + 1
          #  addy = mbi.BaseAddress + loc - 1
          #  rounded_addy = addy & 0xfffffff0
          #  # convert addy from int to long
          #  addy = rounded_addy + (addy & 0x0000000f)
          #  rounded_size = ((32 + len(pattern) - (16 - (addy - rounded_addy)) ) / 16) * 16
          #  data = pydbg.read_process_memory(rounded_addy, rounded_size)
          #  #print pydbg.hex_dump(data, rounded_addy)
          #  return rounded_addy
          #  m = rx.search(data[loc:]) # continue searching

if __name__ == '__main__':
  # BALDRSKY ZERO
  # See: http://9gal.com/read.php?tid=411756
  #pattern = 0x90ff503c83c4208b45ec
  pattern = 0x90, 0xff, 0x50, 0x3c, 0x83, 0xc4, 0x20, 0x8b, 0x45, 0xec

  pid = 10944
  dbg = GameDebugger(pid=pid)
  if dbg.active():
    #print dbg.searchbytes(pattern)
    #pattern = ''.join(imap(chr, pattern))
    #addr = dbg.searchstring(pattern)
    addr = dbg.searchbytes(pattern)
    print hex(addr)

# EOF
