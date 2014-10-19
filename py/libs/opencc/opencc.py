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

DIC_NAMES = { # {(int fr,int to):str}
  (TYPE_ZHS, TYPE_ZHT): "STCharacters.txt",
  (TYPE_ZHS, TYPE_ZHT): "TSCharacters.txt",
  (TYPE_ZHT, TYPE_TW):  "TWVariants.txt",
  (TYPE_ZHT, TYPE_HK):  "HKVariants.txt",
  (TYPE_ZHT, TYPE_JA):  "JPVariants.txt",
}

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
  txt = DIC_NAMES.get((fr, to))
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

def zht2zhs(text): return convert(text, TYPE_ZHT, TYPE_ZHS)
def zht2tw(text): return convert(text, TYPE_ZHT, TYPE_TW)
def zht2hk(text): return convert(text, TYPE_ZHT, TYPE_HK)
def zht2ja(text): return convert(text, TYPE_ZHT, TYPE_JA)

def zhs2zht(text): return convert(text, TYPE_ZHS, TYPE_ZHT)

# Following are for convenient usage
def zhs2tw(text): return zht2hw(zhs2zht(text))
def zhs2hk(text): return zht2hk(zhs2zht(text))
def zhs2ja(text): return zht2ja(zhs2zht(text))

zh2zht = zhs2zht
zh2zhs = zht2zhs
zh2tw = zhs2tw
zh2hk = zhs2hk
zh2ja = zhs2ja

# EOF
