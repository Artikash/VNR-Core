# launcher.pro
# 3/29/2012 jichi

TEMPLATE = subdirs
SUBDIRS = \
  pyreader \
  reader
  #pyinstaller
  #installer
  #updater

OTHER_FILES += \
  include/cc/ccmacro.h \
  include/main.tcc

# EOF
