# coding: utf8
# procutil.py
# 10/14/2012 jichi

import os
import psutil
from PySide.QtCore import QUrl
from Qt5.QtWidgets import QDesktopServices
from sakurakit import skos, skpaths, skwin
from sakurakit.skdebug import dprint
from sakurakit.skunicode import u
from apploc import applocale
from mytr import my
import config, features, growl, osutil, rc, winutil

PID = os.getpid() # cached

## Filter ##

def is_blocked_process_name(name):
  """
  @param  name
  @return  bool
  """
  return name in config.PROCESS_BLACKLIST or name and (
      name.startswith('360') or
      name.startswith('Baidu') or
      name.startswith('baidu'))

def is_my_window(hwnd): return skwin.get_window_process_id(hwnd) == PID

def is_process_running(name=None, path=None): # Only ascii API is Implemented!
  """
  @param* name  unicode
  @param* path  unicode
  @return  bool
  """
  if name:
    name = name.lower()
    for p in psutil.process_iter():
      if p.pid and p.is_running():     # pid == 0 will raise access denied exception on Mac
        try: it = u(p.name)        # system processes raise access denied exception on Windows 7
        except: continue
        if name == it.lower():
          return True
  if path:
    np = osutil.normalize_path(path)
    for p in psutil.process_iter():
      if p.pid and p.is_running():   # pid == 0 will raise access denied exception on Mac
        try: it = u(p.exe)           # system processes raise access denied exception on Windows 7
        except: continue
        if path == osutil.normalize_path(it):
          return True
  return False

## Helper Functions ##

class Process:
  """This class is a replacement for psutil.Process with unicode strings"""
  def __init__(self, pid=0, name=u"", path=u""):
    self.pid = pid
    self.name = name
    self.path = path

def iterprocess():
  """Generator of candidate game process
  @yield  Process

  Implemented using psutil. Using WMI is more efficient, but less portable.
  See: http://www.blog.pythonlibrary.org/2010/10/03/how-to-find-and-list-all-running-processes-with-python/
  """

  windir = skpaths.WINDIR.lower()
  appdata = skpaths.APPDATA.lower()
  localappdata = skpaths.LOCALAPPDATA.lower()
  for p in psutil.process_iter():
    if (p.pid and       # pid == 0 will raise access denied exception on Mac
        p.is_running() and
        p.pid != PID and
        get_process_active_window(p.pid)):
      try: path = u(p.exe)        # system processes raise access denied exception on Windows 7
      except: continue
      name = u(p.name)
      lpath = path.lower()
      if (name in config.PROCESS_BLACKLIST or
          lpath.startswith(windir) or
          lpath.startswith(appdata) or
          lpath.startswith(localappdata)):
        continue
      pid = p.pid
      if '?' in path: # Japanese characters
        path = skwin.get_process_path(pid)
        name = os.path.basename(path)
      yield Process(pid=pid, name=name, path=path)

def get_process_by_path(path):
  """
  @param  path  unicode or None
  @return  Process or None
  """
  np = osutil.normalize_path(path)
  for p in iterprocess():
    if np == osutil.normalize_path(p.path):
      return p

def get_process_by_pid(pid):
  """
  @return  Process or None
  """
  for p in iterprocess():
    if p.pid == pid:
      return p

if not skos.WIN:
  def is_active_window(pid):
    """Unimplemented"""
    return False
  def get_process_active_window(pid):
    """Unimplemented"""
    return 0
else:
  def get_process_active_window(pid):
    """Return the wid of the first active window of the process
    @param  pid  long
    @return  HWND
    """
    return winutil.get_process_good_window(pid)

  def is_active_window(wid):
    """Return if the input window is visible
    @param  wid  long
    @return  bool
    """
    return bool(wid) and winutil.is_good_window(wid)

def may_be_game_window(wid):
  if not wid:
    return False
  if not skos.WIN:
    return False

  pid = skwin.get_window_process_id(wid)
  if not pid or pid == PID:
    return False

  path = skwin.get_process_path(pid)
  if not path:
    return False

  exe = os.path.basename(path)
  if exe in config.PROCESS_BLACKLIST:
    return False

  if (path.startswith(skpaths.WINDIR) or
      path.startswith(skpaths.APPDATA) or
      path.startswith(skpaths.LOCALAPPDATA)):
    return False
  return True

def open_executable(path, lcid=0, params=None, vnrlocale=False):
  """
  @param  path  str  path to executable
  @paramk lcid  int  Microsoft lcid
  @param* params  [unicode param] or None
  @param* vnrlocale  bool  whether inject vnrlocale on the startup
  @return  long  pid
  """
  dprint("lcid = 0x%.4x, vnrlocale = %s, path = %s" % (lcid, vnrlocale, path))

  env = applocale.create_environ(lcid) if lcid and applocale.exists() else None

  if not vnrlocale:
    return skwin.create_process(path, params=params, environ=env)
    #return QDesktopServices.openUrl(QUrl.fromLocalFile(path))
  else:
    from sakurakit import skwinsec
    with skwinsec.SkProcessCreator(path, params=params, environ=env) as proc:
      import inject
      inject.inject_vnrlocale(handle=proc.processHandle)
      return proc.processId

def open_executable_with_ntlea(path, params=None):
  """
  @param  path  str  path to executable
  @param* params  [unicode param] or None
  @return  long  pid of NTLEA, not the target process!
  """
  dprint("path = %s" % path)
  #exe = os.path.join(settings.global_().ntleaLocation(), 'ntleac.exe')
  exe = config.NTLEA_LOCATION

  from ntlea import ntlea
  params = ntlea.params(path=path, args=params)
  return skwin.create_process(exe, params=params)

def open_executable_with_leproc(path, params=None):
  """
  @param  path  str  path to executable
  @param* params  [unicode param] or None
  @return  long  pid of LEProc, not the target process!
  """
  dprint("path = %s" % path)
  import settings
  exe = os.path.join(settings.global_().localeEmulatorLocation(), 'LEProc.exe')
  #exe = 'LEProc.exe'
  if not os.path.exists(exe):
    growl.error('<br/>'.join((
        my.tr("Cannot find Locale Emulator executable") + ' ' + exe,
        my.tr("Please try adjusting the location in Preferences"),
        )))
    return 0

  from localeemu import leproc
  params = leproc.params(path=path, args=params)
  return skwin.create_process(exe, params=params)

def open_executable_with_lsc(path, params=None):
  """
  @param  path  str  path to executable
  @param* params  [unicode param] or None
  @return  long  pid of LEProc, not the target process!
  """
  dprint("path = %s" % path)
  exe = config.LSC_LOCATION

  from localeswitch import lsc
  params = lsc.params(path=path, args=params)
  return skwin.create_process(exe, params=params)

def getyoutube(vids, path=''):
  """
  @param  vids  [str]
  @param* path  unicode
  """
  from scripts import youtube
  if features.MAINLAND_CHINA:
    youtube.getlist_sina(vids, path=path)
  elif len(vids) == 1:
    youtube.get(vids[0], path=path)
  else:
    youtube.getlist(vids, path=path)

# EOF
