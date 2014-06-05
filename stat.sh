#!/bin/bash
# 8/6/2013 jichi
#set +x
echo Line numbers for different languages

echo -n Total:
find . -type f \( \
  -name '*.*' \
  \) | xargs wc -l | tail -1 | sed 's/ total//'

echo -n Python:
find . -type f \( \
  -name '*.py' \
  \) | xargs wc -l | tail -1 | sed 's/ total//'

echo -n C/C++:
find . -type f \( \
  -name '*.h' \
  -o -name '*.c' \
  -o -name '*.cc' \
  -o -name '*.cpp' \
  -o -name '*.pro' \
  -o -name '*.pri' \
  \) | xargs wc -l | tail -1 | sed 's/ total//'

echo -n QML:
find . -type f \( \
  -name '*.qml' \
  -o -name 'qmldir' \
  \) | xargs wc -l | tail -1 | sed 's/ total//'

echo -n HTML/CSS/Javascript:
find . -type f \( \
  -name '*.html' \
  -o -name '*.haml' \
  -o -name '*.css' \
  -o -name '*.js' \
  \) | xargs wc -l | tail -1 | sed 's/ total//'

echo -n Ruby/HAML/SASS/Coffee:
find . -type f \( \
  -name '*.rb' \
  -o -name '*.sass' \
  -o -name '*.coffee' \
  -o -name '*.haml' \
  \) | xargs wc -l | tail -1 | sed 's/ total//'

echo -n XML/YAML:
find . -type f \( \
  -name '*.xml' \
  -o -name '*.yaml' \
  \) | xargs wc -l | tail -1 | sed 's/ total//'

echo -n CMD/Shell/Makefile:
find . -type f \( \
  -name '*.cmd' \
  -o -name '*.sh' \
  -o -name Makefile \
  \) | xargs wc -l | tail -1 | sed 's/ total//'

echo -n TS:
find . -type f \( \
  -name '*.ts' \
  \) | xargs wc -l | tail -1 | sed 's/ total//'

exit

# EOF

# 4/25/2014
Line numbers for different languages
Total:  447457
Python:   88522
C/C++:   70778
QML:   17859
HTML/CSS/Javascript:    6818
Ruby/HAML/SASS/Coffee:    5159
XML/YAML:    1911
CMD/Shell/Makefile:    1396
TS:   45853
