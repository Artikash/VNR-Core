# coding: utf8
# kochars.py
# 4/25/2015 jichi

def ishangul(ch):
  """
  @param  ch  str
  @return  bool
  """
  if len(ch) != 1:
    return False
  ch = ord(ch[0])
  return (
    0xac00 <= ch and ch <= 0xd7a3    # Hangul Syllables (AC00-D7A3) which corresponds to (가-힣)
    or 0x1100 <= ch and ch <= 0x11ff # Hangul Jamo (1100–11FF)
    or 0x3130 <= ch and ch <= 0x318f # Hangul Compatibility Jamo (3130-318F)
    or 0xa960 <= ch and ch <= 0xa97f # Hangul Jamo Extended-A (A960-A97F)
    or 0xd7b0 <= ch and ch <= 0xd7ff # Hangul Jamo Extended-B (D7B0-D7FF)
  )

# EOF
