# coding: utf8
# __main__.py
# 10/5/2012 jichi
#
# Top-level root objects (root object = object that do not have parent)
# - app.Application
# - main.MainObject: Root of all non-widget qobject

def print_help():
  print """\
usage: python . [options]

Launch the app.

options:
  --debug   Print debug output
  --help    Print help"""

def main():
  """
  @return  int
  """
  import os, sys
  from PySide.QtCore import QTextCodec
  # Use UTF-8 encoding for Qt
  #sys_codec = QTextCodec.codecForLocale()
  u8codec = QTextCodec.codecForName("UTF-8")
  QTextCodec.setCodecForCStrings(u8codec)
  QTextCodec.setCodecForTr(u8codec)

  debug = '--debug' in sys.argv # bool
  print >> sys.stderr, "reader: debug = %s" % debug

  from sakurakit import skdebug
  #skdebug.DEBUG = config.APP_DEBUG or '--debug' in sys.argv
  skdebug.DEBUG = debug

  import config
  config.APP_DEBUG = debug

  from sakurakit.skdebug import dprint, dwarn
  dprint("enter")

  if '--help' in sys.argv:
    print_help()
    dprint("exit: help")
    #sys.exit(os.EX_USAGE)
    return 0

  # Singleton
  # - Through mutex
  #   See: http://code.activestate.com/recipes/474070-creating-a-single-instance-application/
  #   See: http://code.google.com/p/python-windows-tiler/source/browse/singleinstance.py
  # - Through file
  #   See: http://stackoverflow.com/questions/380870/python-single-instance-of-program
  dprint("check single instance")
  from lockfile import lockfile
  app_mutex = lockfile.SingleProcessMutex()
  single_app = app_mutex.tryLock()
  if not single_app:
    dprint("multiple instances are running")

  dprint("python = %s" % sys.executable)
  #dprint("rootdir = %s" % rootdir)
  #dprint("mecabrc = %s" % mecabrc_path)

  # Must be set before any GUI is showing up
  # http://stackoverflow.com/questions/1551605/how-to-set-applications-taskbar-icon-in-windows-7
  from sakurakit import skos
  if skos.WIN:
    dprint("set app id")
    from sakurakit import skwin
    skwin.set_app_id("org.sakurakit.reader")

  # Detect user language
  import settings
  ss = settings.global_()
  lang = ss.userLanguage()
  if not lang:
    import locale
    lc = locale.getdefaultlocale()[0]
    if lc and lc.startswith('zh'):
      lang = 'zht'
    elif lc and lc in config.LANGUAGE_LOCALES.values():
      lang = lc[:2]
    else:
      lang = 'en'
    ss.setValue('UserLanguage', lang)

  dprint("user language = %s" % lang)

  # Warm-up shiboken dlls
  #if os.name == 'nt':
  #  dprint("load shiboken plugins")
  #  import pytexscript
  #  import pytexthook

  #Switch to OpenGL engine
  #from Qt5.QtWidgets import QApplication
  #QApplication.setGraphicsSystem("opengl")

  #dprint("elevate process priviledges")
  #from sakurakit import skos
  #if skos.WIN:
  #  from sakurakit import skwin
  #  skwin.enable_drop_event()

  dprint("create app")
  import app
  a = app.Application(sys.argv)
  #dprint("os default font:", a.font())
  #a.setFont(config.FONT_DEFAULT)
  #dprint("current default font:", a.font())

  # Fix the encoding issue for the current directory
  #from PySide.QtCore import QDir
  #QDir.setCurrent(a.applicationDirPath())

  if not single_app:
    from rpcman import RpcClient
    dprint("send metacall")
    r = RpcClient()
    r.start()
    if r.waitForConnected():
      r.activate()
      a.processEvents()
    else:
      dprint("warning: cannot connect to the server")
    dwarn("leave: multiple instance")
    #sys.exit(os.EX_UNAVAILABLE)
    return 0

  # Must come after QApplication is created
  dprint("load fonts")
  from PySide.QtGui import QFontDatabase
  for root, dirs, files in os.walk(config.FONT_LOCATION):
    FONT_EXTS = frozenset(('.otf', '.ttf', '.ttc'))
    for f in files:
      if os.path.splitext(f.lower())[1] in FONT_EXTS:
        p = os.path.join(root, f)
        index = QFontDatabase.addApplicationFont(p)
        if index >= 0:
          dprint(QFontDatabase.applicationFontFamilies(index))
        else:
          dwarn("failed to load font %s" % f)

  dprint("load translation")
  a.loadTranslations()

  dprint("autosync settings")
  ss.autoSync() # Load before qapplication is created

  dprint("show splash screen")
  from PySide.QtGui import QPixmap
  from Qt5.QtWidgets import QSplashScreen
  import rc
  splashImage = QPixmap(rc.image_path('splash-wait'))
  splash = QSplashScreen(splashImage)
  splash.show()
  a.processEvents()

  #dprint("cache fonts")
  #import fonts
  #fonts.test()

  # Take the ownership of sakurakit translation
  dprint("take the ownership of translations")
  from sakurakit import sktr
  sktr.manager().setParent(a)

  import mytr
  mytr.manager().setParent(a)
  mytr.my.setParent(a)

  # There should be at least one existing window (rootWindow), or sth is wrong
  #a.setQuitOnLastWindowClosed(False)

  #dprint("check unicode codec")
  #from sakurakit.skunicode import qunicode
  #ustr = qunicode("あのね", 'utf8')
  #assert ustr, "failed to load text code plugin from qt.conf"

  #dprint('init directories')
  #from PySide.QtCore import QDir

  #for path in rc.DIR_USER, rc.DIR_USER_DATA, rc.DIR_USER_XML, rc.DIR_USER_XML_COMMENTS:
  for it in (
      rc.DIR_XML_COMMENT,
      rc.DIR_XML_VOICE,
      rc.DIR_XML_REF,
      #rc.DIR_DICT_MECAB, # not needed to create
      rc.DIR_CACHE_AVATAR,
      rc.DIR_CACHE_AWS,
      rc.DIR_CACHE_DATA,
      rc.DIR_CACHE_IMAGE,
      rc.DIR_CACHE_DMM,
      rc.DIR_CACHE_GETCHU,
      rc.DIR_CACHE_GYUTTO,
      rc.DIR_CACHE_DIGIKET,
      rc.DIR_CACHE_DLSITE,
      rc.DIR_CACHE_HOLYSEAL,
      rc.DIR_CACHE_SCAPE,
      rc.DIR_CACHE_TRAILERS,
      rc.DIR_CACHE_WEB,
    ):
    if not os.path.exists(it):
      try: os.makedirs(it)
      except OSError:
        dwarn("warning: failed to create directory: %s" % it)

  if skos.WIN:
    from sakurakit import skwin
    for it in rc.DIR_APP_LIBRARY, rc.DIR_APP_CACHE:
      if os.path.exists(it):
        skwin.set_file_readonly(it)

  dprint("load settings")
  ss.setParent(a)

  dprint("append library path")

  from sakurakit import skpaths
  skpaths.append_paths((
    ss.jbeijingLocation(),
    ss.ezTransLocation(),
    ss.atlasLocation(),
    ss.localeEmulatorLocation(),
    os.path.join(ss.lecLocation(), r"Nova\JaEn") if ss.lecLocation() else "",
    os.path.join(ss.dreyeLocation(), r"DreyeMT\SDK\bin") if ss.dreyeLocation() else "",
  ))

  dprint("remove broken caches")
  for it in rc.DIR_APP_TMP,:
    if os.path.exists(it):
      from sakurakit import skfileio
      skfileio.removetree(it)

  if sys.getrecursionlimit() < config.PY_RECURSION_LIMIT:
    dprint("increase recursion limit")
    sys.setrecursionlimit(config.PY_RECURSION_LIMIT)

  dprint("reduce socket timeout")
  import socket
  socket.setdefaulttimeout(config.PY_SOCKET_TIMEOUT)

  #import threading
  #if threading.stack_size() < config.PY_STACK_SIZE:
  #  dprint("increase stack size")
  #  threading.stack_size(config.PY_STACK_SIZE)

  # On unix:
  # stackoverflow.com/questions/5061582/setting-stacksize-in-a-python-script
  #import resource
  #resource.setrlimit(resource.RLIMIT_STACK, (2**29,-1))

  dprint("config python site-packages")

  # Disable HTTP request session
  # See: http://docs.python-requests.org/en/latest/user/advanced/#keep-alive
  # See: http://stackoverflow.com/questions/10115126/python-requests-close-http-connection
  #import requests
  #s = requests.session()
  #s.config['keep_alive'] = False

  import numpy
  numpy.seterr(all='ignore')  # ignore overflow warning

  dprint("update settings")

  ss_version = ss.version()
  if ss_version != config.VERSION_TIMESTAMP:
    dprint("app update detected, migrate settings")
    if ss_version:
      migrate(ss_version)

    from sakurakit import skdatetime
    ss.setUpdateTime(skdatetime.current_unixtime())

    ss.setVersion(config.VERSION_TIMESTAMP)
    ss.sync()

  if not ss.userName() or not ss.userId():
    dprint("set user credential to guest")
    # Must be consistent with dataman.GUEST
    ss.setValue('UserId', 4)
    ss.setValue('UserName', 'guest')
    ss.setValue('UserPassword', 'guest')
    ss.setValue('UserGender', '')
    ss.setValue('UserAvatar', '')
    ss.setValue('UserColor', '')

  if ss.isCursorThemeEnabled():
    dprint("load cursor theme")
    import curtheme
    curtheme.load()

  #dprint("set max thread count")
  from PySide.QtCore import QThreadPool
  currentThreadCount = QThreadPool.globalInstance().maxThreadCount()
  if currentThreadCount < config.QT_THREAD_COUNT:
    dprint("increase thread pool capacity: %s -> %s" % (currentThreadCount, config.QT_THREAD_COUNT))
    QThreadPool.globalInstance().setMaxThreadCount(config.QT_THREAD_COUNT)

  dprint("register qml plugins")
  import qmlplugin

  dprint("query system metrics")
  import sysinfo

  dprint("create main object")

  import main
  m = main.MainObject(a)
  m.run(a.arguments())

  #import netman
  #netman.manager().queryComments(gameId=183)

  #import hashutil
  #print hashutil.md5sum('/Users/jichi/tmp/t.cpp')

  dprint("schedule to hide splash")
  # Disabled which is too slow
  #splashImage = QPixmap(rc.image_path('splash-ready'))
  #splash.setPixmap(splashImage)
  from functools import partial
  from sakurakit import skevents
  skevents.runlater(partial(
      splash.finish, m.topWindow()))

  dprint("exec")
  returnCode = a.exec_()

  # FIXME: Cannot normally exit
  # Shiboken 1.2 hang on exit!
  # All destructors are Invoked!

  dprint("unlock the mutex")
  app_mutex.unlock()

  import defs
  if returnCode == defs.EXIT_RESTART:
    skos.restart_my_process(['-B']) # -B: force disabling *.pyc and *.pyo
  else:
    #sys.exit(returnCode)
    #if skos.WIN: # prevent hanging on windows/mac
    skos.kill_my_process()

def migrate(ss_version): # long ->
  import os
  from sakurakit import skdebug import dprint, dwarn
  from sakurakit import skfileio
  import config, rc, settings

  dprint("enter")

  ss = settings.global_()

  try: # this try is in case I forgot certain rc directories for update

    if ss_version <= 1403890414: # remove existing references directory
      for it in ( # delete all existing references
          rc.DIR_CACHE_DMM,
          rc.DIR_CACHE_GETCHU,
          rc.DIR_CACHE_GYUTTO,
          rc.DIR_CACHE_DIGIKET,
          rc.DIR_CACHE_DLSITE,
          rc.DIR_CACHE_HOLYSEAL,
          rc.DIR_CACHE_SCAPE,
          rc.DIR_CACHE_TRAILERS,
        ):
        if os.path.exists(it):
          skfileio.removetree(it)
          try: os.makedirs(it)
          except OSError:
            dwarn("warning: failed to create directory: %s" % it)

      path = rc.DIR_CACHE_IMAGE
      if os.path.exists(path):
        skfileio.removetree(path)
        try: os.makedirs(path)
        except OSError:
          dwarn("warning: failed to create directory: %s" % path)

    if ss_version <= 1402884913: # clear scape cache
      path = rc.DIR_CACHE_SCAPE
      if os.path.exists(path):
        skfileio.removetree(path)
        try: os.makedirs(path)
        except OSError:
          dwarn("warning: failed to create directory: %s" % path)

    if ss_version <= 1402884913: # delete UniDic MLJ
      path = rc.DIR_CACHE_DICT + '/UniDicMJL'
      if os.path.exists(path):
        skfileio.removetree(path)

    if ss_version <= 1401107220:
      ss.setValue('GameAgent', False) # disable game agent by default

    if ss_version <= 1394254407:
      location = rc.DIR_DICT_MECAB # delete old unidic/unidic-mlj
      for it in 'unidic', 'unidic-mlj':
        path = location + '/' + it
        if os.path.exists(path):
          skfileio.removetree(path)

    if ss_version <= 1393896804: # clearn china images
      if ss.isMainlandChina():
        path = rc.DIR_CACHE_IMAGE
        if os.path.exists(path):
          skfileio.removetree(path)
          try: os.makedirs(path)
          except OSError:
            dwarn("warning: failed to create directory: %s" % path)

    if ss_version <= 1393493964:
      if ss.value("LocaleSwitchEnabled"): # disable locale switch
        ss.setValue("LocaleSwitchEnabled", False)

    if ss_version <= 1393212106: # remove old mecab
      path = rc.DIR_DICT_MECAB
      if os.path.exists(path):
        skfileio.removetree(path)
        try: os.makedirs(path)
        except OSError:
          dwarn("warning: failed to create directory: %s" % path)

    if ss_version <= 1392183792: # disable lougo
      ss.setValue('LougoEnabled', False)

    if ss_version <= 1391988443: # disable lingoes dictionary
      for k in 'EdictEnabled', 'LingoesJaZh', 'LingoesJaKo', 'LingoesJaVi':
        ss.setValue(k, False)
      for it in (
          rc.DIR_USER + '/db',
          rc.DIR_USER + '../cjklib',
        ):
        if os.path.exists(it):
          skfileio.removetree(it)

    if ss_version <= 1390024359: # delete old tmp directories
      for it in (
          rc.DIR_USER_CACHE + '/images',
          rc.DIR_USER_CACHE + '/mecab',
          rc.DIR_USER_XML + '/comments',
        ):
        if os.path.exists(it):
          skfileio.removetree(it)

    if ss_version <= 1389682377: # enable baidu by default
      if lang.startswith('zh'):
        ss.setValue('BaiduEnabled', True)

    if ss_version <= 1386141114: # remove old digests
      for k in 'gameitems', 'gamefiles':
        f = rc.xml_path(k)
        if os.path.exists(f):
          skfileio.removefile(f)

    if ss_version <= 1383376949: # clear old images
      try:
        for root, dirs, files in os.walk(rc.DIR_CACHE_IMAGE):
          for f in files:
            #if f.endswith('.jpg'):
            p = os.path.join(root, f)
            skfileio.removefile(p)
      except Exception, e: dwarn(e)

    #if ss_version <= 1381812548:
    #  dprint("remove old digests")
    #  try:
    #    for k in 'gamedigest', 'itemdigest', 'refdigest':
    #      f = rc.xml_path(k)
    #      if os.path.exists(f):
    #        os.remove(f)
    #  except Exception, e: dwarn(e)

    if ss_version <= 1379659275:
      tmpdir = rc.DIR_CACHE_DATA
      try:
        for root, dirs, files in os.walk(tmpdir):
          for f in files:
            if f.endswith('.jpg'):
              p = os.path.join(root, f)
              skfileio.removefile(p)
      except Exception, e: dwarn(e)

    if ss_version <= 1378612993: # disable timezone by default
      ss.setValue('TimeZoneEnabled', False)

    if ss_version <= 1375218632: # reset text capacity
      ss.setValue('TextCapacity', config.SETTINGS_TEXT_CAPACITY) # TextHook data capacity

    if ss_version <= 1375068568: # disable springbaord lauch by default
      ss.setValue('SpringBoardLaunch', False) # SpringBoard do not launch game by default

    if ss_version <= 1374863216: # reset fonts
      #ss.setValue('GrimoireShadow', False)

      for lang in 'English', 'Chinese', 'Korean', 'Thai', 'Vietnamese', 'Indonesian', 'German', 'French', 'Italian', 'Spanish', 'Portuguese', 'Russian':
        ss.setValue(lang + "Font", "DFGirl")

    #if ss_version <= 1372534597:
    #  v = ss.value('MaleTTS')
    #  if v: ss.setValue('MaleVoiceJa', v)
    #  v = ss.value('FemaleTTS')
    #  if v: ss.setValue('FemaleVoiceJa', v)

    if ss_version <= 1372296306: # reset default grimoire settings
      ss.setValue('GrimoireZoomFactor', config.SETTINGS_ZOOM_FACTOR)
      ss.setValue('GrimoireWidthFactor', config.SETTINGS_WIDTH_FACTOR)
      ss.setValue('GrimoireShadowOpacity', config.SETTINGS_SHADOW_OPACITY)
      ss.setValue('GrimoireSubtitleColor', config.SETTINGS_SUBTITLE_COLOR)
      #ss.setValue('GrimoireTranslationColor', config.SETTINGS_TRANSLATION_COLOR)

    if ss_version <= 1370408692: # disable window hook by default
      ss.setValue('WindowHookEnabled', False) # disable window text translation by default

    if ss_version <= 1365484274: # disable game detection by default
      ss.setValue('GameDetectionEnabled', True) # enable running game detection

    if ss_version <= 1372227052 and ss.blockedLanguages(): # disable blocking Japanese
      try:
        l = ss.blockedLanguages()
        l.remove('ja')
        ss.setBlockedLanguages(l)
      except: pass

    #if ss_version <= 1365658762:
    #  ss.setValue('GrimoireHoverEnabled', False) # disable mouse hover
    #  ss.setValue('GameBorderVisible', False) # disable outlining game window

    if ss_version <= 1365691951: # enable user-defined hook by default
      ss.setValue('HookCodeEnabled', True) # enable user-defined hook code

    if ss_version <= 1365674660: # clear old terms by default
      xmlfile = rc.xml_path('terms')
      if xmlfile and os.path.exists(xmlfile):
        skfileio.removefile(xmlfile)

  except Exception, e:
    dwarn(e)
  dprint("leave")

if __name__ == '__main__':
  import sys
  print >> sys.stderr, "reader: enter"
  #print __file__
  import initrc
  initrc.initenv()
  initrc.probemod()
  initrc.checkintegrity()

  ret = main()
  print >> sys.stderr, "reader: leave, ret =", ret
  sys.exit(ret)
  #assert False, "unreachable"

# EOF
