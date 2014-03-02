# coding: utf8
# msime.py
# 4/2/2013 jichi

if __name__ == '__main__': # DEBUG
  import sys
  sys.path.append("..")

from sakurakit.skclass import memoized

## Locations ##

IME_UPPER_VERSION = 14  # Office 2010 is 14, in case future release
IME_LOWER_VERSION = 8   # Windows Me
IME_MAX_SIZE = 100  # maximum length of the array that can be processed

@memoized
def ja_ime_location():
  """Such like IMJP14.IME, which implements IMJP14K.dll
  @return  unicode or None
  """
  import os
  from sakurakit.skpaths import SYSTEM32
  for ver in xrange(IME_UPPER_VERSION, IME_LOWER_VERSION -1, -1):
    path = os.path.join(SYSTEM32, 'IMJP%s.IME' % ver)
    if os.path.exists(path):
      return path

  # Windows 8?
  from sakurakit.skpaths import WINDIR
  path = os.path.join(WINDIR, r'IME\IMEJP')
  if os.path.exists(path):
    return path

#def ja_dll_location(): # such like IMJP14K.dll

JA_INSTALL_URL = "http://www.microsoft.com/ja-jp/office/2010/ime"
JA_UPDATE_URL = "http://support.microsoft.com/kb/978478?ln=ja"

## Construction ##

@memoized
def ja():
  from sakurakit import skos
  if skos.WIN:
    import pythoncom # Make sure OleInitialzie is invoked
    from pymsime import Msime_ja
  else:
    class Msime_ja: # dummy
      def isValid(self): return False
  return Msime_ja()

@memoized
def ja_valid(): return ja().isValid()

#def destroy():
#  global JA
#  JA = None

## Shortcuts ##

def to_yomi_hira(text):
  """
  @param  text  unicode
  @return  unicode
  """
  if len(text) > IME_MAX_SIZE:
    return ""
  ime = ja()
  return ime.toYomigana(text, ime.Hiragana)

def to_yomi_kata(text):
  """
  @param  text  unicode
  @return  unicode
  """
  ime = ja()
  return ime.toYomigana(text, ime.Katagana)

def to_furi_hira(text):
  """
  @param  text  unicode
  @return  [(unicode kanji,unicode furi)]
  """
  if len(text) > IME_MAX_SIZE:
    return ""
  ime = ja()
  return ime.toFurigana(text, ime.Hiragana)

def to_furi_kata(text):
  """
  @param  text  unicode
  @return  [(unicode kanji,unicode furi)]
  """
  if len(text) > IME_MAX_SIZE:
    return ""
  ime = ja()
  return ime.toFurigana(text, ime.Katagana)

def to_kanji(text):
  """
  @param  text  unicode
  @return  unicode
  """
  if len(text) > IME_MAX_SIZE:
    return ""
  # Autocorrect is not enabled. A counter example follows:
  #   肯定 => 皇帝
  #
  #ime = ja()
  #return ime.toKanji(text, ime.Autocorrect)
  return ja().toKanji(text)

if __name__ == '__main__':
  import os, sys
  os.environ['PATH'] += os.path.pathsep + "../../../bin"
  sys.path.append("../../../bin")
  sys.path.append("..")

  import pythoncom
  print to_kanji(u'すもももももももものうち') #
  print to_kanji(u'みじん'*34) # Only larger than 100?!

  print to_yomi_hira(u'計画通り' * 25)
  print to_furi_hira(u'計画通り' * 26)
  print to_kanji(u'けいかくとおり')
  print to_kanji(u'かわいい')
  # IMJP 14 (windows 7): スモモも桃も桃のうち
  # IMJP 14 (office 2010): すもももももも桃のうち, because there is an anime called すもももももも

# EOF
