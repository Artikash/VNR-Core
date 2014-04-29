# coding: utf8
# socketmarshal.py
# jichi 4/28/2014

# Little endian
ENDIANNESS = 'little'

MESSAGE_HEADER_SIZE = 4 # 4 bytes

# http://stackoverflow.com/questions/444591/convert-a-string-of-bytes-into-an-int-python
def bytes2int(s): # QByteArray -> int
  #return sum(ord(c) << (i * 8) for i,c in enumerate(s[::-1])) # reverse sign does not work for QByteArray
  size = len(s)
  return sum((ord(c) << (8 * (size - i - 1))) for i,c in enumerate(s))

def int32bytes(number): # int -> str
  r = bytes(chr(number))
  while len(r) < 4:
    r = chr(0) + r
  return r

# EOF
