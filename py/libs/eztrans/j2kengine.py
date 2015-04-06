# coding: utf8
# j2kengine.py
# 6/7/2013 jichi
#
# See: http://ohhara.sarang.net/ohbbhlp/
#
# int (__stdcall *J2K_InitializeEx)(const char *initStr, const char *homeDir);
# char * (__stdcall *J2K_TranslateMMNT)(int data0, const char *jpStr);
# int (__stdcall *J2K_FreeMem)(char *krStr);
# int (__stdcall *J2K_StopTranslation)(int data0);
# int (__stdcall *J2K_Terminate)(void);
# int (__stdcall *K2J_InitializeEx)(const char *initStr, const char *homeDir);
# char * (__stdcall *K2J_TranslateMMNT)(int data0, const char *krStr);
# int (__stdcall *K2J_FreeMem)(char *jpStr);
# int (__stdcall *K2J_StopTranslation)(int data0);
# int (__stdcall *K2J_Terminate)(void);
#
# About Ehnd
# - Source: https://github.com/sokcuri/ehnd
# - Dictionary 2015/04: http://sokcuri.neko.kr/220301117949
# - Binary v3: http://blog.naver.com/waltherp38/220267098421
# - Tutorial: http://blog.naver.com/waltherp38/220286266694
#
# Test Ehnd
# - Input: まあね♪スカートとはおさらばだし。ハーフパンツなんて久しぶり♪
# - ezTrans without ehnd: 그냥♪스커트와는 안녕히이고. 하프 팬츠는 오래간만♪
# - With ehnd 3.1 and 201504 script: 글쎄♪스커트와는 안녕히이고. 하프 팬츠 같은거 오래간만♪

if __name__ == '__main__': # DEBUG
  import sys
  sys.path.append("..")

import ctypes, os
import win32api
#from sakurakit import msvcrt
from sakurakit.skdebug import dprint, dwarn

EZTR_LICENSE = 'CSUSER123455'

EZTR_DLL_MODULE = 'J2KEngine'
EHND_DLL_MODULE = 'ehnd'
#EHND_DLL_MODULE = EZTR_DLL_MODULE # use native DLL

class _Loader(object):

  def __init__(self):
    self.initialized = False
    self._ehndDll = None
    self._eztrDll = None

  @property
  def ehndDll(self):
    if not self._ehndDll:
      try:
        self._ehndDll = ctypes.WinDLL(EHND_DLL_MODULE)
        dprint("ehnd dll is loaded")
      except Exception, e:
        dwarn("failed to load ehnd", e)
    return self._ehndDll

  @property
  def eztrDll(self):
    if not self._eztrDll:
      try:
        for suffix in ('.dlx', '.dll'):
          try:
            self._eztrDll = ctypes.WinDLL(EZTR_DLL_MODULE + suffix)
            dprint("%s is loaded" % (EZTR_DLL_MODULE + suffix))
            break
          except WindowsError: pass
      except Exception, e:
        dwarn("failed to load j2kengine", e)
    return self._eztrDll

  def getEztrDirectory(self):
    """
    @return  str not unicode
    @raise
    """
    return os.path.dirname(
        win32api.GetModuleFileName(self.eztrDll._handle))

  def init(self):
    """
    @return  bool
    @raise

    Guessed:
    BOOL __stdcall J2K_InitializeEx(LPCSTR user_name, LPCSTR dat_path)

    """
    path = self.getEztrDirectory()
    path = os.path.join(path, 'Dat') # dic data path
    return 1 == self.ehndDll.J2K_InitializeEx(EZTR_LICENSE, path)

  @staticmethod
  def translateA(dll, text):
    """Translate through Eztr
    @param  dll  ctypes.DLL
    @param  text  str not unicode
    @return  str not unicode
    @raise  WindowsError, AttributeError

    Guessed:
    char *  __stdcall J2K_TranslateMMNT(int data0, const char *jpStr)
    int  __stdcall J2K_FreeMem(char *krStr)
    """
    addr = dll.J2K_TranslateMMNT(0, text)
    if not addr: # int here
      dwarn("null translation address")
      return ""
    ret = ctypes.c_char_p(addr).value
    dll.J2K_FreeMem(addr)
    return ret

  @staticmethod
  def translateW(dll, text):
    """Translate through Ehnd
    @param  dll  ctypes.DLL
    @param  text  unicode not str
    @return  unicode not str
    @raise  WindowsError, AttributeError
    """
    addr = dll.J2K_TranslateMMNTW(0, text)
    if not addr: # int here
      dwarn("null translation address")
      return ""
    ret = ctypes.c_wchar_p(addr).value
    dll.J2K_FreeMem(addr)
    return ret

class Loader(object):

  # See: http://en.wikipedia.org/wiki/Code_page
  INPUT_ENCODING = 'sjis' # Japanese
  OUTPUT_ENCODING = 'uhc' # Korean

  def __init__(self):
    self.__d = _Loader()

  def __del__(self):
    self.destroy()

  def init(self):
    d = self.__d
    if not d.initialized:
      try: d.initialized = d.init()
      except Exception, e: dwarn(e)

  def isInitialized(self): return self.__d.initialized

  def destroy(self): pass

  def translate(self, text, ehnd=True):
    """
    @param  text  unicode
    @param* ehnd  whenther enable ehnd
    @return   unicode
    @throw  RuntimeError
    """
    d = self.__d
    try:
      if ehnd:
        return d.translateW(d.ehndDll, text)
      else:
        text = text.encode(self.INPUT_ENCODING, errors='ignore')
        text = d.translateA(d.eztrDll, text)
        text = text.decode(self.OUTPUT_ENCODING, errors='ignore')
        return text
    except (WindowsError, AttributeError), e:
      dwarn("failed to load j2kengine dll, raise runtime error", e)
      raise RuntimeError("failed to access j2kengine dll")
    #except UnicodeError, e:
    #  dwarn(e)

if __name__ == '__main__': # DEBUG
  import os
  import sys
  os.environ['PATH'] += os.pathsep + r"Z:\Local\Windows\Applications\ezTrans XP"
  os.environ['PATH'] += os.pathsep + r"../../../../Boost/bin"
  os.environ['PATH'] += os.pathsep + r"../../../../Ehnd/bin"
  l = Loader()
  l.init()

  #t = u"お花の匂い"
  #t = 「まあね♪スカートとはおさらばだし。ハーフパンツなんて久しぶり♪」
  #t = u"『まあね♪スカートとはおさらばだし。ハーフパンツなんて久しぶり♪』"
  #t = u"まあね♪スカートとはおさらばだし。ハーフパンツなんて久しぶり♪"
  t = u"蜜ドル辞典"
  ehnd = True
  #ehnd = False
  ret = l.translate(t, ehnd=ehnd)

  # Without ehnd: 그냥♪스커트와는 안녕히이고. 하프 팬츠는 오래간만♪
  # With ehnd 3.1: 글쎄♪스커트와는 안녕히이고. 하프 팬츠 같은거 오래간만♪

  from PySide.QtGui import QApplication, QTextEdit
  a = QApplication(sys.argv)
  w = QTextEdit(ret)
  w.show()
  a.exec_()

# EOF
