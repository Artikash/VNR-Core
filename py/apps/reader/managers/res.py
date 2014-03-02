# coding: utf8
# res.py
# 11/25/2012 jichi

import os
from sakurakit import skfileio
from sakurakit.skclass import memoized, memoizedproperty
from sakurakit.sktr import tr_
from mytr import my
import growl, rc

class Resource(object):
  def __init__(self, path='', lockpath=''):
    self.path = path # unicode
    self.lockpath = lockpath # unicode

  def open(self):
    location = os.path.dirname(self.path)
    import osutil
    osutil.open_location(location)

  def exists(self): # -> bool
    return os.path.exists(self.path)
  def locked(self): # -> bool
    return os.path.exists(self.lockpath)
  def remove(self): # -> bool
    return skfileio.removefile(self.path)
  def get(self): pass # -> bool
  def valid(self): return self.exists()

  def removetree(self): # -> bool # utility
    return skfileio.removetree(self.path)

class AppLocale(Resource):
  URL = 'http://ntu.csie.org/~piaip'
  #URL = 'http://ntu.csie.org/~piaip/papploc.msi'
  #URL = "http://www.microsoft.com/en-us/download/details.aspx?id=13209"

  def __init__(self):
    from sakurakit import skpaths
    super(AppLocale, self).__init__(
      #path=os.path.join(rc.DIR_CACHE_INST, "AppLocale/papploc.msi"),
      path=os.path.join(skpaths.WINDIR, "AppPatch/AlLayer.dll"),
      lockpath=os.path.join(rc.DIR_TMP, "apploc.lock"),
    )

  def remove(self): # override
    from scripts import apploc
    return apploc.remove()

  def get(self): # override
    from scripts import apploc
    return apploc.get()

# Global objects

@memoized
def apploc(): return AppLocale()

# EOF
