# coding: utf8
# socketmarshal.py
# jichi 4/28/2014

if __name__ == '__main__':
  import sys
  sys.path.append('..')

from itertools import imap
from sakurakit.skdebug import dwarn

# Little endian
ENDIANNESS = 'little'

INT32_SIZE = 4 # number of bytes of int32
INT_SIZE = INT32_SIZE

# Bytes

# http://stackoverflow.com/questions/444591/convert-a-string-of-bytes-into-an-int-python
def bytes2int(s): #
  """
  @param  s  str|bytearray|QByteArray
  @return  int
  """
  #return sum(ord(c) << (i * 8) for i,c in enumerate(s[::-1])) # reverse sign does not work for QByteArray
  size = len(s)
  return sum((ord(c) << (8 * (size - i - 1))) for i,c in enumerate(s))

def bytes2int32(s, i=0): #
  """
  @param  s  str|bytearray|QByteArray
  @param*  i  int  start index
  @return  int
  """
  return (ord(s[i]) << 24) + (ord(s[i+1]) << 16) + (ord(s[i+2]) << 8) + ord(s[i+3]) if len(s) >= 4 + i else 0

def int2bytes(i, size=0): # int -> str
  """
  @param  i  int
  @param* size  int  total size after padding
  @return  str
  """
  r = bytes(chr(i))
  while len(r) < size:
    r = chr(0) + r
  return r

def int32bytes(int32): # int -> str
  """
  @param  number  int32
  @return  str  4 bytes
  """
  return int2bytes(int32, INT32_SIZE)

# String list

def marshalstrlist(l, encoding='utf8'):
  """
  @param  l  [unicode]
  @return  str
  """
  head = []
  body = []
  head.append(len(l))
  for s in l:
    if isinstance(s, unicode):
      s = s.encode(encoding, errors='ignore')
    head.append(len(s))
    body.append(s)
  return ''.join(imap(int32bytes, head)) + ''.join(body)

def unmarshalstrlist(data, encoding='utf8'):
  """
  @param  data  str|bytearray|QByteArray
  @return  [unicode] not None
  """
  dataSize = len(data)
  if dataSize < INT32_SIZE:
    dwarn("insufficient list size")
    return []
  index = 0
  count = bytes2int32(data, index); index += INT32_SIZE
  if count == 0:
    dwarn("empty list")
    return []
  if count < 0:
    dwarn("negative count")
    return []
  if count * INT32_SIZE > dataSize - index:
    dwarn("insufficient header size")
    return []
  sizes = [] # [int]
  for i in range(0, count):
    size = bytes2int32(data, index); index += INT32_SIZE
    if size < 0:
      dwarn("negative string size")
      return []
    sizes.append(size)
  if sum(sizes) > dataSize - index:
    dwarn("insufficient body size")
    return []
  ret = []
  for size in sizes:
    s = data[index:index+size]
    s = s.decode(encoding, errors='ignore')
    ret.append(s)
    index += size
  return ret

if __name__ == '__main__':
  l = ['aa', 'bbb', u'あれ？']

  print len(l), l
  for it in l:
    print it

  data =  marshalstrlist(l)
  print len(data), data
  l = unmarshalstrlist(data)
  print len(l), l
  for it in l:
    print it

# EOF
