# coding: utf8
# skprofiler.py
# 1/16/2014 jichi
#
# http://preshing.com/20110924/timing-your-code-using-pythons-with-statement/

import time

class SkProfiler(object):
  def __init__(self, verbose=True):
    self.verbose = verbose

  def __enter__(self):
    self.start = time.clock()
    return self

  def __exit__(self, *args):
    self.end = time.clock()
    self.interval = self.end - self.start
    if self.verbose:
      print "time:", self.interval

# EOF
