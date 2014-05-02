# coding: utf8
# inject.py
# 2/3/2013 jichi
# Windows only

from sakurakit import skos, skpaths, skwin, skwinsec
if skos.WIN:
  import os
  import config
  from sakurakit.skdebug import dprint

  def inject_agent(pid):
    """
    @param  pid  ulong
    @return  bool
    """
    dprint("enter: pid = %s" % pid)
    ret = True
    for dllpath in config.VNRAGENT_DLLS:
      #dllpath = os.path.abspath(dllpath)
      dllpath = skpaths.abspath(dllpath)
      assert os.path.exists(dllpath), "needed dll does not exist"
      ret = skwinsec.injectdll(dllpath, pid=pid) and ret
    dprint("leave: ret = %s" % ret)
    return ret

else:
  def inject_agent(pid): return False

# EOF
