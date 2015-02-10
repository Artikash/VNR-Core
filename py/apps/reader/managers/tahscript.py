# coding: utf8
# tahscript.py
# 8/14/2014 jichi

import re
from sakurakit.skclass import memoized
from sakurakit.skdebug import dprint

_repeat_comma = re.compile(ur'。+')
def repair_tah_text(t): # unicode -> unicode
  return _repeat_comma.sub(u'。', t)

@memoized
def manager(): return TahScriptManager()

class _TahScriptManager:

  def __init__(self):
    self.tah = {} # {str key:TahScriptManager}

  def getTah(self, key): # str key -> TahScriptManager
    ret = self.tah.get(key)
    if not ret:
      import os
      from pytahscript import TahScriptManager
      import config
      ret = self.tah[key] = TahScriptManager()
      path = config.TAHSCRIPT_LOCATIONS[key]
      if os.path.exists(path):
        ret.loadFile(path)
        dprint("load %s rules for %s" % (ret.size(), key))
    return ret

class TahScriptManager:

  def __init__(self):
    self.__d = _TahScriptManager()

  #def reloadScripts(self): self.__d.reloadScripts() # reload scritps

  def apply(self, text, key):
    """
    @param  text  unicode
    @return  unicode
    """
    if key != 'atlas':
      key = 'lec'
    tah = self.__d.getTah(key)
    return repair_tah_text(tah.translate(text)) or text # totally deleting ret is NOT allowed in case of malicious rule

# EOF
