# coding: utf8
# opencc/__init__.py
# 10/18/2014 jichi

# Initialization

OPENCC_DICDIR = ""

def setdicdir(path):
  global OPENCC_DICDIR
  OPENCC_DICDIR = path

def getconverter(fr, to):
  from pycc import SimpleChineseConverter
  ret = SimpleChineseConverter()
  return ret

# Conversion

def zh2zht(text): pass
def zh2zhs(text): pass
def zh2tw(text): pass
def zh2hk(text): pass
def zh2ja(text): pass

def zhs2zht(text): pass
def zhs2tw(text): pass
def zhs2hk(text): pass
def zhs2ja(text): pass

def zht2zhs(text): pass
def zht2tw(text): pass
def zht2hk(text): pass
def zht2ja(text): pass

# EOF
