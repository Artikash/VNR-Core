# tests.pro
# 12/15/2013 jichi

TEMPLATE = subdirs

SUBDIRS += \
  pytest \
  uitest

win32: SUBDIRS += \
  test \
  sehtest

# EOF
