# coding: utf8
# stardict.py
# 2/14/2014 jichi
if __name__ == '__main__':
  import sys
  sys.path.append('..')

def get(lang):
  """
  @param  lang  str  such as ja-en
  @return  bool
  """
  import rc
  return rc.runscript('getstardict.py', (lang,))

if __name__ == "__main__":
  get('ja-vi')

# EOF
