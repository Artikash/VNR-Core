# coding: utf8
# applocale.py
# 11/19/2012

from sakurakit.skdebug import dprint
from sakurakit import skpaths, skwin
import os

ENV_LAYER = '__COMPAT_LAYER'
ENV_LCID = 'AppLocaleID'

def create_process(path, lcid, params=None):
  """
  @param  path  str  path to executable
  @param  lcid  int  ms lcid
  @param  params  [unicode] or None
  @return  long  pid
  """
  dprint("enter: lcid = 0x%.4x, path = %s" % (lcid, path))
  env = (
    (ENV_LAYER, '#APPLICATIONLOCALE'),
    (ENV_LCID, '%.4x' % lcid),
  )
  pid = skwin.create_process(path, environ=env, params=params)
  dprint("leave: pid = %i" % pid)
  return pid

def exists():
  """
  @return  bool
  """
  ret = os.path.exists(skpaths.WINDIR + "/AppPatch/AlLayer.dll")
  dprint("ret = %s" % ret)
  return ret

# EOF
