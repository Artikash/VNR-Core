# coding: utf8
# skpaths.py
# 10/5/2012 jichi
#
# All constants in this file are in unicode

import os
from PySide.QtCore import QDir, QFileInfo
from Qt5.QtWidgets import QDesktopServices
from skunicode import u
from skdebug import dwarn

def abspath(path):
  """As a replacement of os.path.abspath, which has encoding issue for unicode path
  @param  path  unicode or str
  @return  unicode
  """
  return QFileInfo(path).absoluteFilePath() if path else u""

HOME = QDir.homePath()  # unicode
#HOME = os.path.expanduser('~') # This will be str instead of unicode

DESKTOP = (QDesktopServices.storageLocation(QDesktopServices.DesktopLocation)
    or os.path.join(HOME, 'Desktop'))

SYSTEMDRIVE = u(os.environ.get('windir') or r"C:")
WINDIR = u(os.environ.get('windir') or os.path.join(SYSTEMDRIVE, r"Windows"))
PROGRAMFILES = u(os.environ.get('programfiles') or os.path.join(SYSTEMDRIVE, r"Program Files"))
PROGRAMFILESx86 = u(os.environ.get('programfiles(x86)')) or PROGRAMFILES # try x86 first
APPDATA = u(os.environ.get('appdata') or os.path.join(HOME, r"AppData\Roaming"))
LOCALAPPDATA = u(os.environ.get('localappdata') or os.path.join(HOME, r"AppData\Local"))

SYSTEM32 = os.path.join(WINDIR, 'System32')

def prepend_path(path):
  if path:
    try: os.environ['PATH'] = path + os.pathsep + os.environ['PATH']
    except (OSError, UnicodeDecodeError), e: dwarn(e)

def append_path(path):
  if path:
    try: os.environ['PATH'] += os.pathsep + path
    except (OSError, UnicodeDecodeError), e: dwarn(e)

def append_paths(paths):
  paths = filter(bool, paths)
  if paths:
    append_path(os.pathsep.join(paths))

def prepend_paths(paths):
  paths = filter(bool, paths)
  if paths:
    prepend_path(os.pathsep.join(paths))

def get_python_executable(gui=None):
  """
  @param  gui  bool or None
  @return  str  path to python.exe or pythonw.exe
  """
  import sys
  ret = sys.executable
  if gui is not None:
    import skos
    if skos.WIN:
      if gui:
        ret = ret.replace('python.exe', 'pythonw.exe')
      else:
        ret = ret.replace('pythonw.exe', 'python.exe')
  return ret

# Get path to Lib/site-packages
def get_python_site():
  # http://stackoverflow.com/questions/122327/how-do-i-find-the-location-of-my-python-site-packages-directory
  #import site
  #return site.getsitepackages()[-1]
  from distutils import sysconfig
  return sysconfig.get_python_lib()

# EOF
