# coding: utf8
# hanviet.py
# 12/22/2014: jichi
#
# See: ChinesePhienAmWords.txt in QuickTranslator_TAO
# http://www.tangthuvien.vn/forum/showthread.php?t=30151
# http://www.mediafire.com/download/ijkm32ozmti/QuickTranslator_TAO.zip

from sakurakit.skclass import memoized

HANVIET_DIC_PATHS = {
  'word': '',
  'phrase': '',
}

def setdicpaths(paths): # {str key:unicode path}
  for k in HANVIET_DIC_PATHS:
    HANVIET_DIC_PATHS[k] = paths[k]

@memoized
def hvt():
  import os
  from pyhanviet import HanVietTranslator
  ret = HanVietTranslator()
  ret.addWordFile(HANVIET_DIC_PATHS['word'])
  ret.addPhraseFile(HANVIET_DIC_PATHS['phrase'])
  return ret

def lookupword(text):
  """
  @param  text  unicode
  @return  unicode or None
  """
  if text and len(text) == 1:
    i = ord(text[0])
    if i <= 0xffff:
      return hvt().lookupWord(i)

def lookupphrase(text):
  """
  @param  text  unicode
  @return  unicode or None
  """
  return hvt().lookupPhrase(text)

def translate(text, mark=False):
  """
  @param  text  unicode
  @param* delim  unicode
  @param* tone  bool
  @param* capital  bool
  @return  unicode
  """
  return hvt().translate(text, mark)

# EOF
