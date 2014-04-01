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
      rc.DIR_CACHE_DATA,
      rc.DIR_CACHE_HISTORY,
      rc.DIR_CACHE_NETMAN,
      rc.DIR_CACHE_WEBKIT,
      ):
    if not os.path.exists(it):
      try: os.makedirs(it)
      except OSError:
        dwarn("warning: failed to create directory: %s" % it)

  dprint("create app")
  import app
  a = app.Application(sys.argv)

  dprint("load translations")
  a.loadTranslations()

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

  if settings.reader().isCursorThemeEnabled():
    dprint("load cursor theme")
    import curtheme
    curtheme.load()

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
  import sys
  print >> sys.stderr, "reader: enter"
  #print __file__
  import initrc
  initrc.initenv()
  initrc.probemod()
  #initrc.checkintegrity()

  ret = main()
  print >> sys.stderr, "reader: leave, ret =", ret
  sys.exit(ret)
  #assert False, "unreachable"
  assert False, "unreachable"

# EOF
