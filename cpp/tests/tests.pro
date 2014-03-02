# tests.pro
# 12/15/2013 jichi

TEMPLATE = subdirs

SUBDIRS += pytest

win32: SUBDIRS += \
  test \
  sehtest

# EOF
