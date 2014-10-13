# coding: utf8
# debug.py
# 11/16/2012 jichi

import sys, os
def initenv():

  # Add current and parent folder to module path
  mainfile = os.path.abspath(__file__)
  maindir = os.path.dirname(mainfile)

  sys.path.append(maindir)
  os.chdir(maindir)

  import config
  map(sys.path.append, config.ENV_PYTHONPATH)
  map(sys.path.append, config.APP_PYTHONPATH)

  for it in 'PATH', 'LD_LIBRARY_PATH', 'DYLD_LIBRARY_PATH':
    try: os.environ[it] += os.pathsep + os.pathsep.join(config.ENV_PATH)
    except: pass

def debugon():
  from sakurakit import skdebug
  skdebug.DEBUG = True
  import config
  config.APP_DEBUG = True


def app():
  debugon()

  import app
  a = app.Application(sys.argv)
  a.loadTranslations()
  return a

def app_exec(timeout=1000):
  debugon()

  from PySide.QtCore import QTimer
  from Qt5.QtWidgets import QApplication
  app = QApplication(sys.argv)
  QTimer.singleShot(timeout, app.quit)
  return app.exec_()

if __name__ == '__main__':
  print "debug: enter"
  initenv()

  #import settings
  #ss = settings.global_()
  #ss.setBlockedLanguages({'en','de'})
  #print ss.blockedLanguages()

  def test_vnragent():
    import rpcman, config

    if False:
      import app
      a = QApplication(sys.argv)

      d = rpcman._RpcServer()
      d.startClient('127.0.0.1', config.QT_METACALL_PORT)
      d.waitForReady()
      #d.q_activated.emit()
      #d.q_pingServer.emit(123)
      d.q_updateServerJson.emit("123456")
      #d.q_push.emit(123)
      a.processEvents()
      sys.exit(0)

    import procutil
    path = ur'S:\Games\カミカゼ☆エクスプローラー！\cs2.exe'
    pid = procutil.get_process_by_path(path).pid

    from sakurakit import skwinsec
    # It is better to use injectfunc1(AddDllDirectory), which is only supported by win8
    dlls = [
      r'S:\Stream\Library\Frameworks\Qt\PySide\QtCore4.dll',
      r'S:\Stream\Library\Frameworks\Qt\PySide\QtNetwork4.dll',
      r'S:\Stream\Library\Frameworks\Sakura\build\vnragent.dll',
    ]
    for dll in dlls:
      import os
      print os.path.exists(dll)
      skwinsec.injectdll(dll, pid=pid)
    print "debug: leave"

  def test_wmp():
    import os
    from sakurakit import skthreads
    from sakurakit.skwincom import SkCoInitializer
    from wmp.wmp import WindowsMediaPlayer

    #url = "http://translate.google.com/translate_tts?tl=ja&q=hello"
    url = "http://tts.baidu.com/text2audio?lan=jp&ie=UTF-8&text=hello"
    #url = "Z:/Users/jichi/tmp/test.mp3"
    #url = r"Z:\Users\jichi\tmp\test.mp3"
    #url = "Z:\Users\jichi\tmp\test.mp3"
    #url = "Z:\\Users\\jichi\\tmp\\test.mp3"
    print os.path.exists(url)
    p = WindowsMediaPlayer()
    def run():
      with SkCoInitializer(threading=True):
        print p.isValid()
        print p.play(url)
        os.system("pause")

    a = app()
    skthreads.runasync(run)
    a.exec_()

  def test_ocr():
    from modi import modi
    path = "wiki.tiff"
    #path = r"Z:\Users\jichi\opt\stream\Library\Frameworks\Sakura\py\apps\reader\wiki.tiff"
    lang = modi.LANG_JA
    ok = modi.readfile(path, lang)

  def test_chat():
    a = app()
    import chatview
    chatview.manager().showTopic('global')
    a.exec_()

  def test_zunko():
    from sakurakit import skpaths
    path = r'Z:\Local\Windows\Applications\AHS\VOICEROID+\zunko'
    #skpaths.append_path(path) # crash
    skpaths.prepend_path(path)

    from voiceroid.zunko import ZunkoTalk
    ai = ZunkoTalk()
    print ai.load()
    #ai.setVolume(1)
    t = "hello world"
    print ai.speak(t)

    #from PySide.QtCore import QCoreApplication
    #a = QCoreApplication(sys.argv)
    a = app()
    a.exec_()

  test_zunko()

# EOF
