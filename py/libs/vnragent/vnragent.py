# coding: utf8
# vnragent.py
# 5/3/2014 jichi
# The logic in this file must be consistent with that in vnragent.dll.

if __name__ == '__main__': # DEBUG
  import sys
  sys.path.append("..")

import os
from sakurakit.skclass import memoized
from sakurakit.skdebug import dprint

ENGINE_YAML = os.path.join(os.path.dirname(__file__), 'engines.yaml')

@memoized
def get_engine_data():
  import yaml
  return yaml.load(file(ENGINE_YAML, 'r'))

def match(pid=0, path=''):
  """
  @param* pid  long
  @param* path  unicode  file executable
  @return  Engine or None
  """
  from engine import Engine, EngineFinder
  finder = EngineFinder(pid=pid, exepath=path)
  for eng in get_engine_data():
    if finder.eval(eng['exist']):
      dprint("engine = %s" % eng['name'])
      return Engine(**eng)

# EOF
