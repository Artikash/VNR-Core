# coding: utf8
# janovutil.py
# 5/20/2015 jichi

if __name__ == '__main__':
  import sys
  sys.path.append("..")

import janovdef as defs

# At most 16 characters
#__name_re = re.compile(ur'【(.{0,%s}?)】|(.{0,%s}?)「'
#    % (defs.MAX_NAME_LENGTH, defs.MAX_NAME_LENGTH))
def guess_text_name(text, limit=defs.MAX_NAME_LENGTH, quotes=defs.NAME_QUOTES):
  """
  @param  text  unicode
  @param* limit  int
  @param* quotes  [unicode]
  @return  unicode or None
  """
  if text:
    if text[0] == u'【':
      i = text.find(u'】')
      if 0 < i and i <= limit:
        return text[1:i].strip()
    i = text.find(u'「')
    if 0 < i and i <= limit:
      return text[:i]
    if quotes:
      for q in quotes:
        if text[-1]  == q[-1]:
          i = text.find(q[0])
          if 0 < i and i <= limit and -1 == text.find(q[-1], i, -1):
            return text[:i].strip()

  #m = __name_re.match(text)
  #if m:
  #  r = m.group(1) or m.group(2)
  #  if r:
  #    return r.strip()

def remove_text_name(text, limit=defs.MAX_NAME_LENGTH, quotes=defs.NAME_QUOTES):
  """Hash unicode text combined with context_sep
  @param  text  unicode
  @param* limit  int
  @param* quotes  [unicode]
  @return   unicode
  """
  if not text:
    return ''
  for q in quotes:
    if text[-1] == q[-1]:
      i = text.find(q[0]) # remove character name
      if i != -1 and i <= limit:
        if i:
          text = text[i:]
        return text
  if text[0] == u'【':
    i = text.find(u'】')
    if 0 < i and i < limit:
      text = text[i+1:].lstrip() or text # avoid deleting context
  return text

if __name__ == '__main__':
  t = u'【爽】「悠真くんを攻略すれば２１０円か。なるほどなぁ…」'
  #t = u'爽（悠真くんを攻略すれば２１０円か。なるほどなぁ…）'
  print guess_text_name(t)

  print remove_text_name(t)

# EOF
