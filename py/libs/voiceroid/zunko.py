# coding: utf8
# zunko.py
# 10/12/2014 jichi

#from sakurakit import skos
import os

if __name__ == '__main__':
  import sys
  sys.path.append('../../../bin')
  os.environ['PATH'] += os.pathsep + os.pathsep.join((
    '../../../../Python',
    '../../../../Qt/PySide',
  ))

if os.name == 'nt':
  from pyzunko import AITalkSynthesizer

  class _ZunkoTalk:
    def __init__(self, volume, audioBufferSize):
      self.ai = AITalkSynthesizer(volume, audioBufferSize)
      self.valid = False # bool
      self.played = False # bool
      self.volume = volume # float

  class ZunkoTalk:
    DLL = "aitalked.dll"
    ENCODING = 'sjis'

    def __init__(self, volume=1.0, audioBufferSize=0):
      self.__d =_ZunkoTalk(volume, audioBufferSize)

    def load(self, path=None): # -> bool
      d = self.__d
      if not d.valid:
        d.valid = d.ai.init(path or self.DLL)
      return d.valid

    def isValid(self): return self.__d.valid # -> bool

    def speak(self, text):
      d = self.__d
      if isinstance(text, unicode):
        text = text.encode(self.ENCODING, errors='ignore')
      d.played = d.ai.play(text) if text else False
      return d.played

    def stop(self): # ->
      if self.__d.played:
        self.__d.ai.stop()

    def volume(self): return self.__d.volume # -> float
    def setVolume(self, v): # float ->
      d = self.__d
      if d.volume != v:
        d.volume = v
        d.ai.setVolume(v)

else:

  class ZunkoTalk:
    def __init__(self, *args, **kwargs): pass
    def load(self, path): return False
    def isValid(self): return False
    def speak(self, text): return False
    def stop(self): pass
    def volume(self): return 1.0
    def setVolume(self, v): pass

if __name__ == '__main__':
  path = "Z:/Local/Windows/Applications/AHS/VOICEROID+/zunko"
  os.environ['PATH'] += os.pathsep + path

  ai = ZunkoTalk()
  print ai.load()
  t = u"憎しみ？憎しみ。"
  print ai.speak(t)

  sys.path.append('..')
  from sakurakit.skprofiler import SkProfiler
  import time
  for i in range(5):
    print "before sleep"
    time.sleep(5)
    print "after sleep"
    with SkProfiler(): # 0.002 seconds
      print ai.speak(t)

  from PySide.QtCore import QCoreApplication
  a = QCoreApplication(sys.argv)
  a.exec_()

# EOF
