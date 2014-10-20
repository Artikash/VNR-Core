# coding: utf8
# zunko.py
# 10/12/2014 jichi

#from sakurakit import skos
import os

if __name__ == '__main__':
  import sys
  sys.path.append('..')
  sys.path.append('../../../bin')
  os.environ['PATH'] += os.pathsep + os.pathsep.join((
    '../../../../Python',
    '../../../../Qt/PySide',
  ))

if os.name == 'nt':
  import re
  from PySide.QtCore import QTimer
  from unitraits import jpchars
  from pyzunko import AITalkSynthesizer

  SENTENCE_INTERVAL = 100 # the same as Zunko's TextToSpeech thread

  SENTENCE_RE = re.compile(ur"([。？！「」【】])(?![。！？）」\n]|$)")

  def splitSentences(text):
    """
    @param  text  unicode
    @return  [unicode]
    """
    return SENTENCE_RE.sub(r"\1\n", text).split("\n")

  class _ZunkoTalk:
    ENCODING = 'sjis'

    def __init__(self, parent, volume, audioBufferSize):
      self.ai = AITalkSynthesizer(volume, audioBufferSize)
      self.valid = False # bool
      self.played = False # bool
      self.volume = volume # float

      t = self.speakTimer = QTimer(parent)
      t.setSingleShot(False)
      t.setInterval(SENTENCE_INTERVAL)
      t.timeout.connect(self._playSentences)
      self.sentences = [] # [unicode]

    def _speak(self, text):
      if isinstance(text, unicode):
        text = text.encode(self.ENCODING, errors='ignore')
      self.played = self.ai.speak(text) if text else False
      return self.played

    def _clearTimer(self):
      if self.speakTimer.isActive():
        self.speakTimer.stop()
      if self.sentences:
        self.sentences.clear()

    def playOne(self, text): # unicode -> bool
      self._clearTimer()
      return self._speak(text)

    def playList(self, l): # [unicode] -> bool
      self._clearTimer()
      l.reverse()
      self.sentences = l
      first = l.pop()
      self.speakTimer.start()
      return self._speak(first)

    def _playSentences(self):
      if self.isPlaying():
        return
      first = self.sentences.pop()
      if not self.sentences:
        self.speakTimer.stop()
      self._speak(first)

    def stop(self):
      self._clearTimer()
      if self.played:
        self.ai.stop()

    def isPlaying(self): # -> bool
      return self.played and self.ai.isPlaying()

  class ZunkoTalk:
    DLL = "aitalked.dll"

    def __init__(self, parent=None, volume=1.0, audioBufferSize=0):
      self.__d =_ZunkoTalk(parent, volume, audioBufferSize)

    def load(self, path=None): # -> bool
      d = self.__d
      if not d.valid:
        d.valid = d.ai.init(path or self.DLL)
      return d.valid

    def isValid(self): return self.__d.valid # -> bool

    def speak(self, text):
      d = self.__d
      l = splitSentences(text)
      l = filter(jpchars.notallpunct, l)
      if not l:
        d.stop()
        return True
      elif len(l) == 1:
        return d.playOne(text)
      else:
        return d.playList(l)

    def isPlaying(self):
      return bool(self.__d.sentences) or self.__d.isPlaying()

    def stop(self):
      self.__d.stop()

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
    def isPlaying(self): return False
    def speak(self, text): return False
    def stop(self): pass
    def volume(self): return 1.0
    def setVolume(self, v): pass

if __name__ == '__main__':
  path = "Z:/Local/Windows/Applications/AHS/VOICEROID+/zunko"
  os.environ['PATH'] += os.pathsep + path

  from PySide.QtCore import QCoreApplication
  a = QCoreApplication(sys.argv)

  ai = ZunkoTalk()
  print ai.load()

  #t = u"憎しみ！！憎しみ。"
  t = u"「あ、自分は島津秀隆と言います。この中でも新人中の新人ですので、どうぞお手柔らかに」"
  #t = u"あ、自分は島津秀隆と言います、この中でも新人中の新人ですので、どうぞお手柔らかに"
  print ai.speak(t)

  a.exec_()

# EOF
