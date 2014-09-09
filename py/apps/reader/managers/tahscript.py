# coding: utf8
# tahscript.py
# 8/14/2014 jichi

import re
from functools import partial
from sakurakit.skclass import memoized
from sakurakit.skdebug import dprint

_repeat_comma = re.compile(ur'。+')
def repair_tah_text(t): # unicode -> unicode
  return _repeat_comma.sub(u'。', t)

@memoized
def manager(): return TahScriptManager()

class _TahScriptManager:

  def __init__(self):
    self.enabled = {} # {str key:bool}
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

    # Put it here instead of main.py to delay initializing scripts
    import settings
    ss = settings.global_()
    self.setEnabled('atlas', ss.isAtlasScriptEnabled())
    ss.atlasScriptEnabledChanged.connect(partial(self.setEnabled, 'atlas'))
    self.setEnabled('lec', ss.isLecScriptEnabled())
    ss.lecScriptEnabledChanged.connect(partial(self.setEnabled, 'lec'))

  def isEnabled(self, key): return bool(self.__d.enabled.get(key))
  def setEnabled(self, key, t):
    d = self.__d
    if d.enabled.get(key) != t:
      d.enabled[key] = t
      if t:
        d.getTah(key) # force loading tah script

  #def isEmpty(self):
  #  return all(it.isEmpty() for it in self.__d.tah.itervalues())
  #def scriptCount(self):
  #  return sum(it.size() for it in self.__d.tah.itervalues())

  #def reloadScripts(self): self.__d.reloadScripts() # reload scritps

  def apply(self, text, key):
    """
    @param  text  unicode
    @return  unicode
    """
    d = self.__d
    if not d.enabled.get(key):
      return text
    tah = d.getTah(key)
    return repair_tah_text(tah.translate(text)) or text # totally deleting ret is NOT allowed in case of malicious rule

# EOF
