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
  trtest \
  uitest

win32: SUBDIRS += \
  ceviotest \
  fastaittest \
  ocrtest \
  sehtest \
  srtest \
  ttstest \
  vertest \
  wmptest \
  zunkotest

# EOF
