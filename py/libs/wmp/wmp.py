# coding: utf8
# 10/6/2014 jichi

if skos.WIN:
  from pywmp import WindowsMediaPlayer

import os
from sakurakit import skpaths, skos
WMP_DLL_PATH = os.path.join(skpaths.SYSTEM32, "wmp.dll")

# EOF
