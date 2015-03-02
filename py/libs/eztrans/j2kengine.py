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

if __name__ == '__main__': # DEBUG
  import sys
  sys.path.append("..")

import ctypes, os
import win32api
#from sakurakit import msvcrt
from sakurakit.skdebug import dprint, dwarn

EZTR_INIT_STR = 'CSUSER123455'

class _Loader(object):

  DLL_MODULE = 'J2KEngine'

  def __init__(self):
    self.initialized = False
    self._dll = None
    self._dllDirectory = None

  @property
  def dll(self):
    if not self._dll:
      try:
        self._dll = ctypes.WinDLL(self.DLL_MODULE)
        dprint("j2kengine dll is loaded")
      except (WindowsError, AttributeError), e:
        self._dll = None
        dwarn("failed to load j2kengine", e)
    return self._dll

  def _dllLocation(self):
    """
    @return  str not unicode
    @throw  WindowsError, AttributeError
    """
    return win32api.GetModuleFileName(self.dll._handle)

  @property
  def dllDirectory(self):
    """
    @return  str not unicode
    """
    if not self._dllDirectory:
      try: self._dllDirectory = os.path.dirname(self._dllLocation())
      except (WindowsError, AttributeError, TypeError, os.error), e: dwarn(e)
    return self._dllDirectory

  def init(self):
    """
    @return  bool
    @raise  WindowsError, AttributeError, TypeError

    Guessed:
    BOOL __stdcall J2K_InitializeEx(LPCSTR user_name, LPCSTR dat_path)
    """
    path = os.path.join(self.dllDirectory, 'Dat')
    return 1 == self.dll.J2K_InitializeEx(EZTR_INIT_STR, path)

  def translate(self, text):
    """
    @param  text  str not unicode
    @return  str not unicode
    @raise  WindowsError, AttributeError

    Guessed:
    char *  __stdcall J2K_TranslateMMNT(int data0, const char *jpStr)
    int  __stdcall J2K_FreeMem(char *krStr)
    """
    #dword = self.dll.J2K_TranslateMMNT(len(text), text)
    dword = self.dll.J2K_TranslateMMNT(0, text)
    if not dword: # int here
      dwarn("null translation address")
      return ""
    ret = ctypes.c_char_p(dword).value
    self.dll.J2K_FreeMem(dword)
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
    if d.initialized:
      return
    try: d.initialized = d.init()
    except (WindowsError, AttributeError, TypeError): pass

  def isInitialized(self): return self.__d.initialized

  def destroy(self): pass

  def translate(self, text):
    """
    @param  text  unicode
    @return   unicode
    @throw  RuntimeError
    """
    try: return self.__d.translate(
        text.encode(self.INPUT_ENCODING, errors='ignore')).decode(self.OUTPUT_ENCODING, errors='ignore')
    except (WindowsError, AttributeError), e:
      dwarn("failed to load j2kengine dll, raise runtime error", e)
      raise RuntimeError("failed to access j2kengine dll")
    #except UnicodeError, e:
    #  dwarn(e)

if __name__ == '__main__': # DEBUG
  import os
  os.environ['PATH'] += os.pathsep + r"S:\Applications\ezTrans XP"
  l = Loader()
  l.init()

  #ret = l.translate(u"お花の匂い☆")
  ret = l.translate(u"「まあね♪スカートとはおさらばだし。ハーフパンツなんて久しぶり♪」")

  from PySide.QtGui import QApplication, QTextEdit
  a = QApplication(sys.argv)
  w = QTextEdit(ret)
  w.show()
  a.exec_()

# EOF
