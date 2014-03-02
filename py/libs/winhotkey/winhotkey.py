# coding: utf8
# winhotkey.py
# 1/29/2014 jichi
# Windows only
# See: https://github.com/schurpf/pyhk

__all__ = ['WinHotKey']

if __name__ == '__main__': # DEBUG
  import sys
  sys.path.append("..")

from sakurakit.skdebug import dprint

# Debug entry
if __name__ == '__main__' and os.name == 'nt':
  # Start event loop and block the main thread
  import pythoncom
  pythoncom.PumpMessages() # wait forever

# EOF
