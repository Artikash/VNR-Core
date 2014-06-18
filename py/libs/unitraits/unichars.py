# coding: utf8
# unichars.py
# 6/16/2014 jichi

# Orders of hiragana and katagana
ORD_HIRA_FIRST = 12353
ORD_HIRA_LAST = 12438
DIST_HIRA_KATA = 96
ORD_KATA_FIRST = ORD_HIRA_FIRST + DIST_HIRA_KATA
ORD_KATA_LAST = ORD_HIRA_LAST + DIST_HIRA_KATA

# Orders of wide and thin characters
ORD_THIN_FIRST = 33
ORD_THIN_LAST = 126
DIST_THIN_WIDE = 65248
ORD_WIDE_FIRST = ORD_THIN_FIRST + DIST_THIN_WIDE
ORD_WIDE_LAST = ORD_THIN_LAST + DIST_THIN_WIDE

#ORD_NUM_FIRST = ord('0') # 48
#ORD_NUM_LAST = ord('9') # 57

def ordany(text, start, stop):
  """
  @param  text  unicode
  @param  start  int
  @param  stop  int
  @return  bool
  """
  for c in text:
    u8 = ord(c)
    if u8 >= start and u8 <= stop:
      return True
  return False

def ordall(text, start, stop):
  """
  @param  text  unicode
  @param  start  int
  @param  stop  int
  @return  bool
  """
  for c in text:
    u8 = ord(c)
    if u8 < start or u8 > stop:
      return False
  return True

# EOF
