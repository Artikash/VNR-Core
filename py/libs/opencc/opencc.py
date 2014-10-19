# coding: utf8
# opencc/__init__.py
# 10/18/2014 jichi

import os

TYPE_ZH = 0
TYPE_ZHS = 1
TYPE_ZHT = 2
TYPE_TW = 3
TYPE_HK = 4
TYPE_JA = 5

# Initialization

OPENCC_DICDIR = "" # unicode  directory of OpenCC's plain-text dictionaries

def setdicdir(path):
  global OPENCC_DICDIR
  OPENCC_DICDIR = path

_CONVERTERS = {}
def getconverter(fr, to):
  """
  @param  fr  int
  @param  to  int
  @return  SimpleChineseConverter
  """
  key = fr * 10 + to
  ret = _CONVERTERS.get(key)
  if not ret:
    ret = _CONVERTERS[key] = makeconverter(fr, to)
  return ret

def makeconverter(fr, to):
  """
  @param  fr  int
  @param  to  int
  @return  SimpleChineseConverter
  """
  from pycc import SimpleChineseConverter
  ret = SimpleChineseConverter()
  txt = ''
  if fr == TYPE_ZHS and to == TYPE_ZHT:
    txt = "STCharacters.txt"
  elif fr == TYPE_ZHS and to == TYPE_ZHT:
    txt = "TSCharacters.txt"
  if txt:
    path = os.path.join(OPENCC_DICDIR, txt)
    if os.path.exists(path):
      #from sakurakit.skprofiler import SkProfiler
      #with SkProfiler(): # 10/19/2014: 0.006 seconds for zhs2zht
      ret.addFile(path)
  return ret

# Conversion

def convert(text, fr, to):
  """
  @param  text  unicode
  @param  fr  int
  @param  to  int
  @return  unicode
  """
  return getconverter(fr, to).convert(text)

def zh2zht(text): pass
def zh2zhs(text): pass
def zh2tw(text): pass
def zh2hk(text): pass
def zh2ja(text): pass

def zhs2zht(text): return convert(text, TYPE_ZHS, TYPE_ZHT)
def zhs2tw(text): pass
def zhs2hk(text): pass
def zhs2ja(text): pass

def zht2zhs(text): return convert(text, TYPE_ZHT, TYPE_ZHS)
def zht2tw(text): pass
def zht2hk(text): pass
def zht2ja(text): pass

# EOF
