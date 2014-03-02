# coding: utf8
# __main__.py
# 12/13/2012 jichi
#
# Top-level root objects (root object = object that do not have parent)
# - app.Application
# - main.MainObject: Root of all non-widget qobject

## MAIN TEMPLATE BEGIN ##

import os, sys

#def u_from_native(s):
#  # Following contents are copied from sakurakit.skunicode
#  import locale
#  lc, enc = locale.getdefaultlocale()
#  return s.decode(enc, errors='ignore')
#u = u_from_native

def print_help():
  print """\
usage: python . [options]

Launch the app.

options:
  --debug   Print debug output
  --help    Print help"""

def reset_win_path():
  try:
    print >> sys.stderr, "browser:reset_win_path: enter"
    windir = os.environ['windir'] or os.environ['SystemRoot'] or r"C:\Windows"
    path = os.pathsep.join((
      windir,
      os.path.join(windir, 'System32'),
    ))
    os.environ['PATH'] = path
    print >> sys.stderr, "browser:reset_win_path: leave"
  except Exception, e:
    print >> sys.stderr, "browser:reset_win_path: leave, exception =", e

MB_OK = 0 # win32con.MB_OK
MB_ICONERROR = 16 # win32con.MB_ICONERROR
def msgbox(text, title, type=MB_OK):
  """A message box independent of pywin32.
  @param  title  str not unicode
  @param  text  str not unicode
  """
  if os.name == 'nt':
    from ctypes import windll
    windll.user32.MessageBoxA(None, text, title, type)

def probemod():
  print >> sys.stderr, "browser:probemod: enter"
  try: from PySide import QtCore
  except ImportError, e:
    print >> sys.stderr, "browser:probemod: ImportError:", e
    msgbox("""\
I am sorry that VNR got a severe error on startup m(_ _)m
It seems that some of the following libraries are missing:

* msvc 2008 sp1 x86 redist:
  http://www.microsoft.com/download/details.aspx?id=5582
* msvc 2010 sp1 x86 redist:
  http://www.microsoft.com/download/details.aspx?id=26999

Feel free to complain to me (annotcloud@gmail.com) if this error keeps bothering you.

ERROR MESSAGE BEGIN
%s
ERROR MESSAGE END""" % e,
      "VNR Startup Error",
      MB_OK|MB_ICONERROR)
    sys.exit(1)
  except UnicodeDecodeError, e:
    print >> sys.stderr, "browser:probemod: UnicodeDecodeError:", e
    msgbox("""\
I am sorry that VNR got a severe error on startup m(_ _)m

It seems that you have UNICODE (non-English) characters in the path to VNR.
Due to technical difficulties, UNICODE path would crash VNR.
I hope if you could try removing UNICODE characters including Japanese, Chinese, etc.

Feel free to complain to me (annotcloud@gmail.com) if this error keeps bothering you.

ERROR MESSAGE BEGIN
%s
ERROR MESSAGE END""" % e,
      "VNR Startup Error",
      MB_OK|MB_ICONERROR)
    sys.exit(1)
  print >> sys.stderr, "browser:probemod: leave"

def initenv():
  print >> sys.stderr, "browser:initenv: enter"
  # Enforce UTF-8
  # * Reload sys
  #   See: http://hain.jp/index.php/tech-j/2008/01/07/Pythonの文字化け
  # * Create sitecustomize.py
  #   See: http://laugh-labo.blogspot.com/2012/02/sitecustomizepy.html
  #import sys
  #reload(sys) # make 'setdefaultencoding' visible
  #sys.setdefaultencoding('utf8')

  # Add current and parent folder to module path
  mainfile = os.path.abspath(__file__)
  maindir = os.path.dirname(mainfile)

  sys.path.append(maindir)

  if os.name == 'nt':
    reset_win_path()

  # Python chdir is buggy for unicode
  #os.chdir(maindir)

  import config

  #for path in config.ENV_PYTHONPATH:
  #  sys.path.append(path)
  map(sys.path.append, config.ENV_PYTHONPATH)

  paths = os.pathsep.join(config.ENV_PATH)
  try:
    os.environ['PATH'] = paths + os.pathsep + os.environ['PATH']
  except KeyError: # PATH does not exists?!
    os.environ['PATH'] = paths

  #if os.name == 'nt':
  #  assert os.path.exists(config.ENV_MECABRC), "mecabrc does not exist"
  #  os.putenv('MECABRC', config.ENV_MECABRC.replace('/', os.path.sep))

  #from distutils.sysconfig import get_python_lib
  #sitedir = get_python_lib()
  #pyside_home =sitedir + '/PySide'
  #sys.path.append(pyside_home)

  #python_home = dirname(rootdir) + '/Python'
  #qt_home = dirname(rootdir) + '/Qt'
  #sys.path.append(qt_home + '/bin')

  print "browser:initenv: leave"

## MAIN TEMPLATE END ##

def main():
  # Use UTF-8 encoding for Qt
  from PySide.QtCore import QTextCodec
  #sys_codec = QTextCodec.codecForLocale()
  u8codec = QTextCodec.codecForName("UTF-8")
  QTextCodec.setCodecForCStrings(u8codec)
  QTextCodec.setCodecForTr(u8codec)

  import config
  from sakurakit import skdebug
  #skdebug.DEBUG = config.APP_DEBUG or '--debug' in sys.argv
  skdebug.DEBUG = '--debug' in sys.argv
  print "browser: debug = %s" % skdebug.DEBUG

  from sakurakit.skdebug import dprint, dwarn
  dprint("enter")

  if '--help' in sys.argv:
    print_help()
    dprint("leave: help")
    return

  dprint("python = %s" % sys.executable)
  #dprint("rootdir = %s" % rootdir)
  #dprint("mecabrc = %s" % mecabrc_path)

  from sakurakit import skos
  if skos.WIN:
    dprint("set app id")
    from sakurakit import skwin
    skwin.set_app_id("org.sakurakit.browser")

  import rc
  for it in (
      rc.DIR_CACHE_WEB,
      ):
    if not os.path.exists(it):
      try: os.makedirs(it)
      except OSError:
        dwarn("warning: failed to create directory: %s" % it)

  dprint("create app")
  import app
  a = app.Application(sys.argv)

  #dprint("load translations")
  #a.loadTranslations()

  # Take the ownership of sakurakit translation
  from sakurakit import sktr
  sktr.manager().setParent(a)

  dprint("load settings")
  import settings
  ss = settings.global_()
  ss.setParent(a)

  dprint("update settings")
  if ss.version() != config.VERSION_TIMESTAMP:
    dprint("app update detected, migrate settings")
    ss.setVersion(config.VERSION_TIMESTAMP)
    ss.sync()

  dprint("set max thread count")
  from PySide.QtCore import QThreadPool
  if QThreadPool.globalInstance().maxThreadCount() < config.QT_THREAD_COUNT:
    QThreadPool.globalInstance().setMaxThreadCount(config.QT_THREAD_COUNT)

  dprint("load web settings")
  import webrc
  webrc.update_web_settings()

  dprint("create main object")
  import main
  m = main.MainObject()
  m.run(a.arguments())

  dprint("exec")
  sys.exit(a.exec_())

if __name__ == '__main__':
  print  >> sys.stderr, "browser: enter"
  initenv()
  probemod()
  main()
  print  >> sys.stderr, "browser: leave" # unreachable
  assert False, "unreachable"

# EOF
