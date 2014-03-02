# coding: utf8
# csvutil.py
# 8/7/2013 jichi

if __name__ == '__main__': # DEBUG
  import sys
  sys.path.append('..')
  import debug
  debug.initenv()

import codecs, csv
from sakurakit.skdebug import dprint, dwarn
from sakurakit.sktr import tr_
from office import excelcsv
import defs, dataman

#def _u16(t):
#  """
#  @param  t  unicode
#  @return  str
#  """
#  return t.encode('utf16', errors='ignore')

def _escape(t):
  """Remove illegal characers for CSV
  @param unicode
  @return unicode
  """
  return t.replace('\n', ' ').replace('\r', ' ')

def _writeterms(fp, data, header=True):
  """
  @param  fp  file object
  @param  data  iter(dataman.Term or dataman._Term)
  """
  w = excelcsv.writer(fp)
  if header:
    w.writerow((
      tr_('Target'),
      tr_('Language'),
      tr_('Regular expression'),
      tr_('Pattern'),
      tr_('Translation'),
      tr_('Comment'),
    ))
  t = dataman.Term.typeName
  for it in data:
    if not it.disabled:
      w.writerow((
        t(it.type),
        it.language,
        it.regex,
        it.pattern,
        it.text,
        it.comment,
      ))

def _writecomments(fp, data, header=True):
  """
  @param  fp  file object
  @param  data  iter(dataman.Comment or dataman._Comment)
  """
  w = excelcsv.writer(fp)
  if header:
    w.writerow((
      tr_('Type'),
      tr_('Language'),
      tr_('Translation'),
      tr_('Comment'),
      'ctx',
      tr_('Context') + ":-3",
      tr_('Context') + ":-2",
      tr_('Context') + ":-1",
      tr_('Context') + ":0",
    ))
  t = dataman.Comment.typeName
  for it in data:
    if not it.disabled and it.type in ('subtitle', 'comment', 'popup'):
      ctx = it.context.split(defs.CONTEXT_SEP)
      ctx1 = ctx[-1] if len(ctx) >= 1 else ""
      ctx2 = ctx[-2] if len(ctx) >= 2 else ""
      ctx3 = ctx[-3] if len(ctx) >= 3 else ""
      ctx4 = ctx[-4] if len(ctx) >= 4 else ""
      w.writerow((
        t(it.type),
        it.language,
        _escape(it.text),
        _escape(it.comment),
        it.contextSize,
        _escape(ctx4),
        _escape(ctx3),
        _escape(ctx2),
        _escape(ctx1),
      ))

def savecomments(path, data):
  """
  @param  path  unicode
  @param  data  iter(dataman.Comment or dataman._Comment)
  """
  dprint(path)
  try:
    with open(path, 'w') as f:
      _writecomments(f, data)
      return True
  except IOError, e: dwarn(e)
  return False

def saveterms(path, data):
  """
  @param  path  unicode
  @param  data  iter(dataman.Term or dataman._Term)
  """
  dprint(path)
  try:
    with open(path, 'wb') as f:
      #f.write(codecs.BOM_UTF16_LE) # BOM at the beginning
      _writeterms(f, data)
      return True
  except IOError, e: dwarn(e)
  return False

if __name__ == '__main__':
  saveterms("test.csv", [])

# EOF
