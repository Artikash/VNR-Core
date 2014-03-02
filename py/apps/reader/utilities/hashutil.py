# coding: utf8
# hashutil.py
# 10/30/2012 jichi

if __name__ == '__main__':
  import sys
  sys.path.append('..')
  import debug
  debug.initenv()

from sakurakit import skhash

MD5SUM = {} # {unicode path : str hex}
def md5sum(path):
  """
  @param  path  str or unicode not None  path to file
  @param  str  cached lowercase hex string
  """
  try: return MD5SUM[path]
  except KeyError:
    ret = skhash.md5sumpath(path)
    if ret:
      MD5SUM[path] = ret
    return ret


"""
@param  str or unicode not None
@return   str
"""
urlsum = skhash.md5sumdata

#"""
#@param  s  str or unicode not None
#@return   long
#"""
#urlhash = skhash.djb2
#
#def urlhashstr(url, maxsize=32):
#  """
#  @param  url  str
#  @param  maxsize  int
#  @return  str
#  """
#  h = urlhash(url)
#  ret = hex(h)
#  if len(ret) > maxsize:
#    ret = ret[-maxsize:]
#  return ret

"""
@param  s  str or unicode not None
@param  h long or None
@return   long
"""
strhash = skhash.djb2_64

"""Backward compat for VNR alpha
@param  s  str or unicode not None
@param  h long or None
@return   long
"""
strhash_old_vnr = skhash.djb2_64_s


"""Backward compat for Zky's subs of「はつゆきさくら」.
The problem is caused by 0 values in utf16 encoding.
@param  s  str or unicode not None
@return  long
"""
strhash_old_ap = skhash.djb2_64_s_nz

if __name__ == '__main__':
  print urlsum("http://www.amazon.co.jp")
  print urlsum("http://www.amazon.co.jp/")

# EOF
