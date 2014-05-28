# coding: utf8
# skprofiler.py
# 1/16/2014 jichi
#
# http://preshing.com/20110924/timing-your-code-using-pythons-with-statement/

import time
import skdebug

class SkProfiler(object):
  def __init__(self, verbose=skdebug.DEBUG):
    self.verbose = verbose

  def __enter__(self):
    self.start = time.clock()
    return self

  def __exit__(self, *args):
    self.end = time.clock()
    self.interval = self.end - self.start
    if self.verbose:
      print "time:", self.interval

if __name__ == '__main__':
  count = 1
  size = 100000 * 20
  text = u"日本語で"
  table = {str(k):'test' for k in xrange(0, size)}

  print "replace"
  with SkProfiler():
    for i in xrange(0, count):
      for k,v in table.iteritems():
        text = text.replace(k, v)

  import skstr
  print "prepare"
  with SkProfiler():
    repl = skstr.multireplacer(table)
  print "apply"
  with SkProfiler():
    for i in xrange(0, count):
      text = repl(text)

# EOF
