# coding: utf8
# tahscript.py
# 8/14/2014 jichi

import re
from sakurakit.skclass import memoized, memoizedproperty
from sakurakit.skdebug import dprint

_repeat_comma = re.compile(ur'。+')
def repair_tah_text(t): # unicode -> unicode
  return _repeat_comma.sub(u'。', t)

@memoized
def manager(): return TahScriptManager()

class _TahScriptManager:

  def __init__(self):
    self.enabled = False

  @memoizedproperty
  def tah(self):
    import os
    from pytahscript import TahScriptManager
    import config
    ret = TahScriptManager()
    path = config.TAHSCRIPT_LOCATION
    if os.path.exists(path):
      ret.loadFile(path)
      dprint("load %s rules" % ret.size())
    return ret

  #def reloadScripts(self, lang=None):
  #  """
  #  @param* lang  str
  #  """
  #  import os
  #  paths = config.TAHSCRIPT_LOCATIONS
  #  if not lang:
  #    for lang,path in paths.iteritems():
  #      if os.path.exists(path):
  #        self.tah[lang].loadFile(path)
  #        dprint("load (%s) %s rules from %s" % (lang, self.tah[lang].size(), path))
  #  else:
  #    path = paths.get(lang)
  #    if path and os.path.exists(path):
  #      self.tah[lang].loadFile(path)
  #      dprint("load (%s) %s rules from %s" % (lang, self.tah[lang].size(), path))

class TahScriptManager:

  def __init__(self):
    self.__d = _TahScriptManager()

    import settings
    ss = settings.global_()
    self.setEnabled(ss.isTahScriptEnabled())
    ss.tahScriptEnabledChanged.connect(self.setEnabled)

  def isEnabled(self): return self.__d.enabled
  def setEnabled(self, t):
    d = self.__d
    if d.enabled != t:
      d.enabled = t
      if t:
        d.tah # force loading tah script

  #def isEmpty(self):
  #  return all(it.isEmpty() for it in self.__d.tah.itervalues())
  #def scriptCount(self):
  #  return sum(it.size() for it in self.__d.tah.itervalues())

  #def reloadScripts(self): self.__d.reloadScripts() # reload scritps

  def replace(self, text):
    """
    @param  text  unicode
    @return  unicode
    """
    d = self.__d
    if not d.enabled:
      return text
    return repair_tah_text(d.tah.translate(text)) or text # totally deleting ret is NOT allowed in case of malicious rule

# EOF
