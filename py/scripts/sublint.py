# coding: utf8
# sublint.py
# Verify the integrity of subtitles.
# 12/16/2014 jichi

if __name__ == '__main__':
  import initrc
  initrc.chcwd()
  initrc.initenv()

import os, sys
import yaml
from sakurakit.skdebug import dprint, dwarn
#from sakurakit.skprof import SkProfiler
import initdefs

def lint(path): # unicode -> bool
  print "process:", path
  ok = False
  try:
    with open(path, 'r') as f:
      l = yaml.load(f)
      if not l or not isinstance(l, list):
        raise TypeError("yaml root is not a valid list")
      options = l.pop(0)
      ok = True
  except Exception, e:
    dwarn(e)
  if ok:
    print "pass"
  else:
    print "failed"
  return ok

## Main ##

def usage():
  cmd = os.path.basename(sys.argv[0])
  print """\
usage: %s yamlfile1 [yamlfile2] ...
Check syntax errors in yaml subtitles.""" % cmd

def main(argv):
  """
  @param  argv  [unicode]
  @return  int
  """
  if len(argv) == 0 or argv[0] in initdefs.HELP_FLAGS:
    usage()
    return 0
  #dprint("enter")

  errorCount = 0
  for path in argv:
    ok = lint(path)
    if not ok:
      errorCount += 1
  return errorCount

if __name__ == '__main__':
  ret = main(sys.argv[1:])
  sys.exit(ret)

# EOF
