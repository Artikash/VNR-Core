# tests.pro
# 12/15/2013 jichi

TEMPLATE = subdirs

SUBDIRS += test

SUBDIRS += \
  hashtest \
  jsontest \
  pytest \
  socktest \
  tahtest \
  uitest

win32: SUBDIRS += \
  ceviotest \
  ocrtest \
  sehtest \
  ttstest

# EOF
