# coding: utf8
# 10/6/2014 jichi

import os
from sakurakit import skpaths
from sakurakit.skdebug import dwarn

WMP_DLL_PATH = os.path.join(skpaths.SYSTEM32, "wmp.dll")

def getclass():
  """
  @return  pywmp.WindowsMediaPlayer or None
  """
  try:
    from pywmp import WindowsMediaPlayer
    return WindowsMediaPlayer
  except Exception, e:
    dwarn(e)

# EOF
