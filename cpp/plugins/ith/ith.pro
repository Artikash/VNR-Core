# ith.pro
# 10/13/2011 jichi

TEMPLATE = subdirs

# The order is important!
# Dependence:
# - cli <= sys
# - srv <= sys
# - eng <= cli + sys
SUBDIRS += \
  sys \
  hook hookxp \
  srv

OTHER_FILES += dllconfig.pri

include(common/common.pri)  # not used
#include(cli/cli.pri)       # not used
#include(srv/srv.pri)       # not used

# EOF
