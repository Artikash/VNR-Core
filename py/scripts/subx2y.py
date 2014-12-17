# coding: utf8
# subx2y.py
# Convert subtitles in old XML format to new YAML format.
# 12/16/2014 jichi

if __name__ == '__main__':
  import initrc
  #initrc.chcwd()
  initrc.initenv()

import os, sys
import yaml
from lxml import etree
#from sakurakit.skdebug import dprint, dwarn
#from sakurakit.skprof import SkProfiler
import initdefs

CONTEXT_SEP = "||"
def convert(userId=0, type='', text='', language='', context="", contextSize=0, comment='', updateComment='', disabled=False, **kwargs):
  """
  @return  dict or None
  """
  if not disabled and text and context and type == 'subtitle':
    ret = {}
    if language:
      ret['subLang'] = language
    if contextSize > 1:
      t = context.split(CONTEXT_SEP)[-1]
    else:
      t = context
    ret['text'] = t
    ret['sub'] = text
    #if userId:
    #  ret['userId'] = userId
    c = comment or updateComment
    if c:
      ret['comment'] = c
    return ret

# Fields
# - userId
# - subLang
# - sub
# - text
# - comment
def readxml(xmlfile): # unicode -> [Subtitle]
  print "read xml:", xmlfile
  ret = []
  try:
    context = etree.iterparse(xmlfile, events=('start','end'))
    path = 0
    for event, elem in context:
      if event == 'start':
        path += 1
        if path == 3: # grimoire/comments/comment
          kw = {
            'id': int(elem.get('id')),
            'type': elem.get('type'),
            'disabled': elem.get('disabled') == 'true',
            'locked': elem.get('locked') == 'true',
          }
      else:
        path -= 1
        if path == 3: # grimoire/comments/comment
          tag = elem.tag
          text = elem.text
          if tag in ('language', 'text', 'comment', 'updateComment'):
            kw[tag] = text or ''
          #elif tag in ('gameId', 'userId', 'timestamp', 'updateUserId', 'updateTimestamp'):
          elif tag.endswith('Id') or tag.endswith('Hash') or tag.endswith('Count') or tag.endswith('imestamp'):
            kw[tag] = int(text)
          elif tag == 'context':
            kw['context'] = text or ''
            kw['hash'] = long(elem.get('hash'))
            kw['contextSize'] = int(elem.get('size'))

        elif path == 2: # grimoire/comments
          #if not kw.get('userHash'):
          #  kw['userHash'] = kw['userId']
          s = convert(**kw)
          if s:
            ret.append(s)

    print "pass"
    return ret

  except etree.ParseError, e:
    print "xml parse error", e.args
  except (TypeError, ValueError, AttributeError), e:
    print "xml malformat", e.args
  except Exception, e:
    print e
  print "failed"

def writeyaml(path, subs, textLang='ja'):
  """
  @param  path  unicode
  @param  subs  [dict]
  @param* textLang  str
  @return  bool
  """
  print "write yaml:", path
  subs.insert(0, {
    'textLang': textLang,
  })
  try:
    f = file(path, 'w')
    # See: skyaml
    yaml.safe_dump(subs, f, default_flow_style=False, encoding='utf-8', allow_unicode=True)
    print "pass"
    return True
  except Exception, e:
    print e
  print "failed"
  return False

def x2y(path):
  """
  @param  path  unicode
  @return  bool
  """
  base, suffix = os.path.splitext(path)
  if suffix.lower() == '.xml':
    output = base + '.yaml'
  else:
    output = path + '.yaml'
  l = readxml(path)
  return bool(l) and writeyaml(output, l)

## Main ##

def usage():
  cmd = os.path.basename(sys.argv[0])
  VERSION = 1418779662
  print """\
usage: %s xmlfile1 [xmlfile2] ...
version: %s
Convert subtitles in old XML format to new YAML format.""" % (cmd, VERSION)

def main(argv):
  """
  @param  argv  [unicode]
  @return  int  error file count
  """
  if len(argv) == 0 or argv[0] in initdefs.HELP_FLAGS:
    usage()
    return 0
  #dprint("enter")

  errorCount = 0
  for i,path in enumerate(argv):
    if i:
      print
    ok = x2y(path)
    if not ok:
      errorCount += 1
  return errorCount

if __name__ == '__main__':
  ret = main(sys.argv[1:])
  sys.exit(ret)

# EOF
