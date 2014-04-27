# tests.pro
# 12/15/2013 jichi

TEMPLATE = subdirs

SUBDIRS += \
  hashtest \
  jsontest \
  pytest \
  uitest

win32: SUBDIRS += \
  test \
  sehtest

# EOF
