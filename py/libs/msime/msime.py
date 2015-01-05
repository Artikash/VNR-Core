# coding: utf8
# msime.py
# 4/2/2013 jichi
#
# Debug on Windows:
# set PATH=..\..\..\..\Qt\PySide;..\..\..\bin;%PATH%

if __name__ == '__main__': # DEBUG
  import os, sys
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

@memoized
def ko_ime_location():
  """Such like imkr80.IME
  @return  unicode or None
  """
  import os
  from sakurakit.skpaths import SYSTEM32
  ver = 80
  path = os.path.join(SYSTEM32, 'imkr%s.ime' % ver)
  if os.path.exists(path):
    return path

  # Windows 8?
  from sakurakit.skpaths import WINDIR
  path = os.path.join(WINDIR, r'IME\IMEKR')
  if os.path.exists(path):
    return path

#def ja_dll_location(): # such like IMJP14K.dll

JA_INSTALL_URL = "http://www.microsoft.com/ja-jp/office/2010/ime"
JA_UPDATE_URL = "http://support.microsoft.com/kb/978478?ln=ja"

## Construction ##

@memoized
def ja():
  import pythoncom # Make sure OleInitialzie is invoked
  from pymsime import Msime
  return Msime(Msime.Japanese)

@memoized
def ko():
  import pythoncom # Make sure OleInitialzie is invoked
  from pymsime import Msime
  return Msime(Msime.Korean)

@memoized
def zhs():
  import pythoncom # Make sure OleInitialzie is invoked
  from pymsime import Msime
  return Msime(Msime.SimplifiedChinese)

@memoized
def zht():
  import pythoncom # Make sure OleInitialzie is invoked
  from pymsime import Msime
  return Msime(Msime.TraditionalChinese)

@memoized
def ja_valid(): return ja().isValid()

@memoized
def ko_valid(): return ko().isValid()

@memoized
def zhs_valid(): return zhs().isValid()

@memoized
def zht_valid(): return zht().isValid()

#def destroy():
#  global JA
#  JA = None

## Shortcuts ##

def to_kanji(text):
  """
  @param  text  unicode
  @return  unicode
  """
  if len(text) > IME_MAX_SIZE:
    return ""
  return ja().toKanji(text)

def to_kanji_list(text):
  """
  @param  text  unicode
  @return  [(unicode ruby,unicode kanji)]
  """
  if len(text) > IME_MAX_SIZE:
    return ""
  return ja().toKanjiList(text)

def to_hira(text):
  """
  @param  text  unicode
  @return  unicode
  """
  if len(text) > IME_MAX_SIZE:
    return ""
  ime = ja()
  return ime.toRuby(text, ime.Hiragana)

def to_kata(text):
  """
  @param  text  unicode
  @return  unicode
  """
  return ja().toRuby(text, ime.Katagana)

def to_hira_list(text):
  """
  @param  text  unicode
  @return  [(unicode kanji,unicode ruby)]
  """
  if len(text) > IME_MAX_SIZE:
    return ""
  ime = ja()
  return ime.toRubyList(text, ime.Hiragana)

def to_kata_list(text):
  """
  @param  text  unicode
  @return  [(unicode kanji,unicode ruby)]
  """
  if len(text) > IME_MAX_SIZE:
    return ""
  ime = ja()
  return ime.toRubyList(text, ime.Katagana)

def to_hangul(text):
  """
  @param  text  unicode
  @return  unicode
  """
  if len(text) > IME_MAX_SIZE:
    return ""
  ime = ko()
  return ime.toRuby(text, ime.Hangul)

def to_hangul_list(text):
  """
  @param  text  unicode
  @return  unicode
  """
  if len(text) > IME_MAX_SIZE:
    return ""
  ime = ko()
  return ime.toRubyList(text, ime.Hangul)

def to_pinyin(text):
  """
  @param  text  unicode
  @return  unicode
  """
  if len(text) > IME_MAX_SIZE:
    return ""
  ime = zhs()
  return ime.toRuby(text, ime.Pinyin)

def to_pinyin_list(text):
  """
  @param  text  unicode
  @return  unicode
  """
  if len(text) > IME_MAX_SIZE:
    return ""
  ime = zhs()
  return ime.toRubyList(text, ime.Pinyin)

if __name__ == '__main__':
  import os, sys
  os.environ['PATH'] += os.path.pathsep + "../../../bin"
  os.environ['PATH'] += os.path.pathsep + "../../../../Qt/PySide"
  sys.path.append("../../../bin")
  sys.path.append("../../../../Qt/PySide")
  sys.path.append("..")

  print ja_valid()
  print ko_valid()
  print zhs_valid()
  print zht_valid()

  import pythoncom
  print "kanji:", to_kanji(u'すもももももももものうち') #
  for k,v in to_kanji_list(u'すもももももももものうち'):
    print k,v
  print "kanji:", to_kanji(u'みじん'*34) # Only larger than 100

  print "hangul:", to_hangul(u'計画通')

  print "pinyin:", to_pinyin(u'計画通')

  print to_hira(u'計画通り')
  print to_hira_list(u'計画通り')
  print to_kanji(u'けいかくとおり')
  print to_kanji(u'かわいい')
  # IMJP 14 (windows 7): スモモも桃も桃のうち
  # IMJP 14 (office 2010): すもももももも桃のうち, because there is an anime called すもももももも

# EOF
