# coding: utf8
# zunko.py
# 10/12/2014 jichi

from sakurakit import skos

if skos.WIN:
  from pyzunko import AITalkSynthesizer

  class _ZunkoTalk:
    def __init__(self):
      self.ai = AITalkSynthesizer()
      self.valid = False # bool
      self.played = False # bool
      self.volume = 1.0 # float

  class ZunkoTalk:
    DLL = "aitalked.dll"
    ENCODING = 'sjis'

    def __init__(self):
      self.__d =_ZunkoTalk()

    def load(self, path=DLL): # -> bool
      d = self.__d
      if not d.valid:
        d.valid = d.ai.init(path)
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
    def load(self, path=''): return False
    def isValid(self): return False
    def speak(self, text): return False
    def stop(self): pass
    def volume(self): return 1.0
    def setVolume(self, v): pass

# EOF
