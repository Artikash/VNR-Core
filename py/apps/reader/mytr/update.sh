#!/bin/bash
# update.sh
# 2/17/2013 jichi
set +x
proj=../mytr.pro

echo "pyside-lupdate '$proj'"
pyside-lupdate "$proj"
dos2unix *

# EOF
