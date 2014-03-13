# coding: utf8
# winhotkey.py
# 1/29/2014 jichi
# Windows only
# See: https://github.com/schurpf/pyhk

__all__ = ['WinHotKey']

if __name__ == '__main__': # DEBUG
  import sys
  sys.path.append("..")

import os
from sakurakit.skdebug import dprint

# Debug entry
if __name__ == '__main__' and os.name == 'nt':
  # http://schurpf.com/python/python-hotkey-module/
  import pyhk
  def fun_k():
    print "key"
  def fun_m():
    print "mouse"

  #create pyhk class instance
  hot = pyhk.pyhk()
  #add hotkey
  hot.addHotkey(['Ctrl', 'A'],fun_k)
  hot.removeHotkey(['Ctrl', 'A'])

  hot.addHotkey(['mouse right'],fun_m)

  # Start event loop and block the main thread
  import pythoncom
  pythoncom.PumpMessages() # wait forever

# EOF
