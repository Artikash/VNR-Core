# coding: utf8
# kanjidic.py
# 5/7/2014 jichi
if __name__ == '__main__':
  import sys
  sys.path.append('..')

def get():
  """
  @return  bool
  """
  import rc
  return rc.runscript('getkanjidic.py')

if __name__ == "__main__":
  get()

# EOF
