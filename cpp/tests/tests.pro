# tests.pro
# 12/15/2013 jichi

TEMPLATE = subdirs

SUBDIRS += test

SUBDIRS += \
  avtest \
  hanjatest \
  hashtest \
  jsontest \
  pinyintest \
  pytest \
  socktest \
  tahtest \
  uitest

win32: SUBDIRS += \
  ceviotest \
  fastaittest \
  imetest \
  ocrtest \
  sehtest \
  srtest \
  trtest \
  ttstest \
  vertest \
  wmptest \
  zunkotest

# EOF
