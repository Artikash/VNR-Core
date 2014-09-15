# coding: utf8
# colorconv.py
# 9/14/2014 jichi
#
# References:
# - HSL/HSV: https://en.wikipedia.org/wiki/HSL_and_HSV
#   https://hg.python.org/cpython/file/2.7/Lib/colorsys.py
# - HSI: http://www.had2know.com/technology/hsi-rgb-color-converter-equations.html
#   http://stackoverflow.com/questions/23889620/is-there-any-other-way-to-convert-rgb-to-hsi
#
# Others:
# - https://github.com/scikit-image/scikit-image/blob/master/skimage/color/colorconv.py
# - colorsys in Python2

import math

def pix2rgb(p): # int -> (int r, int g, int b)
  return p >> 16 & 0xff, p >> 8 & 0xff,  p & 0xff

def pix2argb(p): # int -> (int a, int r, int g, int b)
  return p >> 24 & 0xff, p >> 16 & 0xff, p >> 8 & 0xff, p & 0xff

def rgb2pix(r, g, b): # (int, int, int) -> int
  return r << 16 + g << 8 + b

def argb2pix(a, r, g, b): # (int, int, int) -> int
  return a << 24 + r << 16 + g << 8 + b

# S and I are easy to compute, but H is very expensive
# See: http://www.had2know.com/technology/hsi-rgb-color-converter-equations.html
# See: http://stackoverflow.com/questions/23889620/is-there-any-other-way-to-convert-rgb-to-hsi
def rgb2hsi(r, g, b): # int, int, int -> float rad or None, float, float
  s,i = rgb2hsi_si(r, g, b)
  h = rgb2hsi_h(r, g, b)
  return h,s,i

def rgb2hsi_h(r, g, b): # int, int, int -> float or None
  if r or g or b:
    dRG = r - g
    dRB = r - b
    dGB = g - b

    d = dRG*dRG + dRB*dGB
    if d:
      cos = (dRG+dRB)/(2*math.sqrt(d))
      try: return math.acos(cos) * 2
      except ValueError: pass  # if cos is too big
  return None

def rgb2hsi_si(r, g, b): # int, int, int -> float, float
  i = (r + g + b) / 3.0 # intense
  s = 1 - min(r, g, b)/float(i) if i else 0 # saturation
  return s,i

def deg2rad(d): return d * math.pi / 180 # float -> float
def rad2deg(d): return d * 180 / math.pi # float -> float

if __name__ == '__main__':
  #r,g,b = pix2rgb(0x0f32ff)
  r,g,b = pix2rgb(0x0000ff)
  print r,g,b
  h,s,i = rgb2hsi(r,g,b)
  print h,s,i
  print h * 180 / math.pi

# EOF
