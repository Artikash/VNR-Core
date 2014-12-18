# coding: utf8
# hanviet.py
# 12/22/2014: jichi
#
# See: ChinesePhienAmWords.txt in QuickTranslator_TAO
# http://www.tangthuvien.vn/forum/showthread.php?t=30151
# http://www.mediafire.com/download/ijkm32ozmti/QuickTranslator_TAO.zip

def _load(): # -> dict
  import os
  path = os.path.join(os.path.dirname(__file__), 'hanviet.yaml')
  #for line in open(path, 'r'):
  #  if not line[0] == '#':
  #    x, s, y = line.encode('utf8').partition(':')
  #    HANVIET[x] = y.strip()
  import yaml
  return yaml.load(file(path, 'r'))
HANVIET = _load()

# For performance reason, redundant spaces after the last Chinese word is not checked
def hanviet(t): # unicode -> unicode
  #return ''.join((HANVIET.get(c) or c for c in t))
  l = []
  for i,c in enumerate(t):
    v = HANVIET.get(c)
    if v:
      l.append(v)
      l.append(' ')
    else:
      l.append(c)
  return ''.join(l)

han2viet = hanviet # backward compatibility

if __name__ == '__main__':
  print hanviet(u"你好 123 english")

# EOF
