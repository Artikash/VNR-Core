# coding: utf8
# mdcompile.py
# 11/9/2013 jichi
#
# See:
# http://tseiya.hatenablog.com/entry/2012/09/19/191114
# http://mecab.googlecode.com/svn/trunk/mecab/doc/dic.html
#
# Example:
# mecab-dict-index -d /usr/local/Cellar/mecab/0.993/lib/mecab/dic/ipadic -u original.dic -f utf8 -t utf8 out.csv

if __name__ == '__main__': # DEBUG
  import sys
  sys.path.append("..")

import os, subprocess
from sakurakit.skdebug import dwarn

MIN_CSV_SIZE = 10 # minimum CSV file size

def csv2dic(dic, csv, exe='mecab-dict-index', dicdir='', call=subprocess.call):
  """Note: this process would take at least 10 seconds
  @param  dic  unicode
  @param  csv  unicode
  @param* exe  unicode
  @param* dicdir  unicode
  @param* call  launcher function
  @return  bool
  """
  # MeCab would crash for empty sized csv
  from sakurakit import skfileio
  if skfileio.filesize(csv) < MIN_CSV_SIZE:
    dwarn("insufficient input csv size", csv)
    return False
  args = [
    exe,
    '-f', 'utf8', # from utf8
    '-t', 'utf8', # to utf8
    '-u', dic,
    csv,
  ]
  if dicdir:
    args.extend((
      '-d', dicdir
    ))
  return call(args) in (0, True) and os.path.exists(dic)

if __name__ == '__main__':
  dicdir = "/Users/jichi/opt/stream/Caches/Dictionaries/UniDic"
  print csv2dic('test.dic', 'test.csv', dicdir=dicdir)

# EOF
