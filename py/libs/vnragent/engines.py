# coding: utf8
# engines.py
# 7/11/2015 jichi
import os
from sakurakit.skdebug import dprint

class Engine:
  name = '' # str
  def match(self, finder):
    """
    @param  finder  EngineFinder
    @return  bool
    """
    return False

class ShinaRioEngine(Engine):
  name = 'EmbedRio' # override
  def match(self, finder):
    """@reimp"""
    ini = finder.getAbsPath("RIO.INI")
    if ini and os.path.exists(ini):
      version = self.getRioVersion(ini)
      if version >= 248:
        return True
    return False

  @staticmethod
  def getRioVersion(path):
    """
    @param  path  unicode
    @return  int
    """
    # Example: [椎名里緒 v2.50]
    try:
      line = next(open(path, 'r'))
      TagSize = 8 # size of 椎名里緒 that contains 8 bytes in sjis
      if len(line) >= TagSize + 8 and line[0] == '[' and line[TagSize + 1] == ' ' and line[TagSize + 2] == 'v' and line[TagSize + 4] == '.':
        major = line[TagSize + 3]
        minor1 = line[TagSize + 5]
        minor2 = line[TagSize + 6]
        if str.isdigit(major):
          ret = int(major) * 100
          if str.isdigit(minor1):
            ret += int(minor1) * 10
          if str.isdigit(minor2):
            ret += int(minor2)
          dprint("version = %s" % ret)
          return ret
    except Exception: pass
    return 0

ENGINES = (
  ShinaRioEngine(),
)

# EOF
