# coding: utf8
# engine.py
# 4/7/2013 jichi

__all__ = ['SapiEngine']

if __name__ == '__main__': # debug
  import os, sys
  os.environ['PATH'] += os.path.pathsep + "../../../bin"
  sys.path.append("../../../bin")
  sys.path.append("..")

from sakurakit import skstr
from sakurakit.skclass import memoizedproperty
import registry

# Examples:
# <pitch middle="0"><rate speed="-5">\n%s\n</rate></pitch>
# <rate speed="-5">\n%s\n</rate>
# <volume level="5">\n%s\n</volume>
# <rate speed="5">\nお花の匂い\n</rate>
def _toxmltext(text, speed=0):
  """
  @param  text  unicode  text to speak
  @param* speed  int
  @return  unicode  XML
  """
  # "\n" is critical to prevent unicode content from crashing
  # absspeed: absolute
  # speed: relative
  return '<rate speed="%i">\n%s\n</rate>' % (speed, skstr.escapehtml(text)) #if speed else text

def _tovoicekey(key):
  """
  @return  unicode
  """
  if key:
    for (hk,base) in (
        ('HKEY_LOCAL_MACHINE', registry.TTS_HKLM_PATH),
        ('HKEY_CURRENT_USER', registry.TTS_HKCU_PATH),
      ):
      path = base +  '\\' + key
      if registry.exists(path, hk):
        return hk + '\\' + path
  return ''

class SapiEngine(object):

  # Consistent with registry.py
  def __init__(self, key='',
      speed=0,
      name='', vendor='',
      language='ja', gender='f',
      **kw):
    self.key = key      # str registry key
    self.speed = speed  # int [-10,10]
    self.name = name    # str
    self.vendor = vendor  # str
    self.language = language # str
    self.gender = gender # str

  @memoizedproperty
  def tts(self):
    from  pywintts import WinTts
    ret = WinTts()
    ret.setVoice(_tovoicekey(self.key))
    return ret

  def isValid(self):
    return bool(self.key) and self.tts.isValid()

  def stop(self):
    self.tts.purge()

  def speak(self, text, async=True):
    """
    @param  text  unicode
    """
    #if stop:
    #  self.stop()
    if text:
      if self.speed:
        text = _toxmltext(text, speed=self.speed)
      self.tts.speak(text, async)

if __name__ == '__main__': # debug
  import sys
  import pythoncom
  #pythoncom.OleInitialize()
  #reg = r"HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Speech\Voices\Tokens\VW Misaki"
  kw = registry.query(key='VW Misaki')
  tts = SapiEngine(**kw)
  print 1
  t = u"Hello"
  t = u"お花の匂い"
  tts.speak(t, async=False)
  #tts.speak(u'<pitch middle="0"><rate speed="-5">%s</rate></pitch>' % t, async=False)
  #tts.speak(u'<rate speed="-5">\n%s\n</rate>' % t, async=False
  tts.speak(u'<volume level="5">\n%s\n</volume>' % t, async=False)
  #tts.speak(u'<rate speed="5">お花の匂い</rate>', async=False)
  sys.exit(0)

  print 2
  tts.stop()
  tts.speak(u"お早う♪", async=False)
  print 3
  import time
  time.sleep(4)
  print 4

# EOF

