# coding: utf8
# inject.py
# 2/3/2013 jichi
# Windows only

from sakurakit import skos, skpaths, skwin, skwinsec
if skos.WIN:
  import os
  import config
  from sakurakit.skdebug import dprint

  def inject_vnragent(**kwargs):
    """
    @param* pid  ulong
    @param* handle  HANDLE
    @return  bool
    """
    dprint("enter")
    ret = True
    for dllpath in config.VNRAGENT_DLLS:
      #dllpath = os.path.abspath(dllpath)
      dllpath = skpaths.abspath(dllpath)
      assert os.path.exists(dllpath), "needed dll does not exist: %s" % dllpath
      ret = skwinsec.injectdll(dllpath, **kwargs) and ret
    dprint("leave: ret = %s" % ret)
    return ret

  def inject_vnrlocale(**kwargs):
    """
    @param* pid  ulong
    @param* handle  HANDLE
    @return  bool
    """
    dprint("enter")
    ret = True
    for dllpath in config.VNRLOCALE_DLLS:
      #dllpath = os.path.abspath(dllpath)
      dllpath = skpaths.abspath(dllpath)
      assert os.path.exists(dllpath), "needed dll does not exist: %s" % dllpath
      ret = skwinsec.injectdll(dllpath, **kwargs) and ret
    dprint("leave: ret = %s" % ret)
    return ret

else:
  def inject_vnragent(pid): return False
  def inject_vnrlocale(handle): return False

# EOF
