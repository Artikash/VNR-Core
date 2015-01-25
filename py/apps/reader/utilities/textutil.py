# coding: utf8
# textutil.py
# 10/8/2012 jichi

import re
from sakurakit import skstr
from sakurakit.skunicode import u
from unitraits import unichars, jpchars
import defs

## Encoding ##

# SHIFT-JIS VS CP932
# http://d.hatena.ne.jp/r_ikeda/20111105/shift_jis
def to_unicode(data, encoding): # str, str -> unicode
  if encoding == 'shift-jis':
    encoding = 'cp932'
  return u(data, encoding)
  #return qunicode(data, encoding)

## Helpers ##

__space_re = re.compile(r'\s{2,}')
def remove_repeat_spaces(text):
  """
  @param  text  unicode
  @return  unicode
  """
  return __space_re.sub(' ', text) if text else ''

__repeat_re = re.compile(r'(.+)\1+', re.DOTALL) # http://stackoverflow.com/questions/12468613/regex-to-remove-repeated-character-pattern-in-a-string
def remove_repeat_text(text):
  """
  @param  text  unicode
  @return  unicode
  """
  return __repeat_re.sub(r'\1', text) if text else ''

# http://e-words.jp/p/r-ascii.html
# 0-31, except:
# - x0a
# - x0d
__illegal_re = re.compile(r'[\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0b\x0c\x0e\x0f\x10\x11\x12\x12\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f]')
def remove_illegal_text(text):
  """
  @param  text  unicode
  @return  unicode
  """
  return __illegal_re.sub('', text) if text else ''

def is_illegal_text(text):
  """
  @param  text  unicode
  @return  bool
  """
  return bool(__illegal_re.search(text))

# At most 16 characters
__name_re = re.compile(ur'【(.{0,%s}?)】|(.{0,%s}?)「'
    % (defs.MAX_NAME_LENGTH, defs.MAX_NAME_LENGTH))
def guess_text_name(text):
  """
  @param  text  unicode
  @return  unicode or None
  """
  m = __name_re.match(text)
  if m:
    r = m.group(1) or m.group(2)
    if r:
      return r.strip()

#__noname_re = re.compile(ur'^(?:【.{0,%s}?】|.{0,%s}?(?=「))(.*)$'
#    % (defs.MAX_NAME_LENGTH, defs.MAX_NAME_LENGTH))
def remove_text_name(text):
  """
  @param  text  unicode
  @return  unicode not None
  """
  #return __noname_re.sub('\\1', text)
  # Avoid using regex for better performance
  if not text:
    return ""
  i = text.find(u'「')
  if i > 0 and i < defs.MAX_NAME_LENGTH:
    return text[i:]
  if text[0] == u'【':
    i = text.find(u'】')
    if i > 0 and i < defs.MAX_NAME_LENGTH:
      return text[i+1:].lstrip()
  return text

__beauty_text_re = re.compile(ur'([。？！」】])(?![。！？」]|$)')
def beautify_text(text):
  """
  @param  text  unicode
  @return  unicode
  """
  return __beauty_text_re.sub('\\1\n', text) #.replace(u'\n」', u'」')

__beauty_sub_re = re.compile(ur'】(?<!\n)')
def beautify_subtitle(text):
  """
  @param  text  unicode
  @return  unicode
  """
  return __beauty_sub_re.sub('】\n', text)

__normalize_name_re = re.compile(ur"[【】]")
"""
@param  text  unicode
@return  unicode
"""
def normalize_name(text):
  return __normalize_name_re.sub('', text)

def skip_empty_line(text):
  """
  @param  text  unicode
  @return  bool
  """
  return bool(text) and text != '\n'

normalize_punct = skstr.multireplacer({
  u"〜": u"～",
  u"‥": u"…",
  #u"\n": u" ", # JBeijing cannot handle multiple lines
})

__match_kata_hira_punc_re = re.compile(r"[%s]+" % ''.join((jpchars.s_kata, jpchars.s_hira, jpchars.s_punct)))
def match_kata_hira_punc(text):
  """
  @param  text  unicode
  @return  bool
  """
  return bool(__match_kata_hira_punc_re.match(text))

# http://www.sakuradite.com/wiki/zh/VNR/Voice_Settings
#import config
#repair_zunko_text = skstr.multireplacer(
#  config.load_yaml_file(config.ZUNKO_YAML_LOCATION)['escape']
#)

def __capitalize_sentence_s(m): # with space
  ch = m.group(2)
  if ch.isdigit():
    return (m.group(1) or '') + ch # do not change
  else:
    return ' ' + ch.upper()
def __capitalize_sentence_ns(m): # without space
  return m.group(1).upper()
#__capitalize_suffix = r"(\s)*(\w)"
__capitalize_period_re = re.compile(r"(?<=\w\.)(\s)*(\w)") # space
__capitalize_punct_re = re.compile(r"(?<=[?!])(\s)*(\w)") # space
__capitalize_paragraph_re = re.compile(ur"(?:^|(?<=[「」【】]))(\w)") # no space
def capitalize_sentence(text):
  """
  @param  text  unicode
  @return  unicode
  """
  text = __capitalize_paragraph_re.sub(__capitalize_sentence_ns, text)
  text = __capitalize_punct_re.sub(__capitalize_sentence_s, text)
  text = __capitalize_period_re.sub(__capitalize_sentence_s, text)
  return text

# Example sentence to test for LEC
# ひとまずいつものように今月の雑誌に目を通そう
def __capitalize_html_sentence_s(m): # with space
  ch = m.group(3)
  if ch.isdigit():
    return (m.group(1) or '') + m.group(2) + ch # do not change
  else:
    return ' ' + m.group(2) + ch.upper()
def __capitalize_html_sentence_ns(m): # without space
  return m.group(1) + m.group(2).upper()
__capitalize_html_period_re = re.compile(r"(?<=\w\.)(\s)*(\<[^>]+?\>)(\w)") # space
__capitalize_html_punct_re = re.compile(r"(?<=[?!])(\s)*(\<[^>]+?\>)(\w)") # space
__capitalize_html_paragraph_re = re.compile(ur"(?:^|(?<=[「」【】]))(\<[^>]+?\>)(\w)") # no space
def capitalize_html_sentence(text):
  """
  @param  text  unicode  containing html tags
  @return  unicode
  """
  text = capitalize_sentence(text)
  if '<' in text and '>' in text:
    text = __capitalize_html_paragraph_re.sub(__capitalize_html_sentence_ns, text)
    text = __capitalize_html_punct_re.sub(__capitalize_html_sentence_s, text)
    text = __capitalize_html_period_re.sub(__capitalize_html_sentence_s, text)
  return text

__html_tag_re = re.compile(r'<[^>]*>')
def remove_html_tags(text):
  """
  @param  text  unicode
  @return  unicode
  """
  return __html_tag_re.sub('', text.replace('<br/>', '\n')) if '<' in text else text

__re_chars = re.compile(r"[%s]" % re.escape(
  skstr.REGEX_SPECIAL_CHARS + r"{}"
))
def mightbe_regex(text):
  """
  @param  text  unicode
  @return  bool
  """
  return bool(__re_chars.search(text))

def validate_regex(text):
  """
  @param  text  unicode
  @return  bool
  """
  #text = text.replace('[^', '[\\')
  return not text or skstr.checkpair(text)

def validate_macro(text):
  """
  @param  text  unicode
  @return  bool
  """
  return not text or skstr.checkpair(text, pair=('{','}'))

_S_PUNCT = unichars.s_ascii_punct + jpchars.s_punct
def ispunct(ch):
  return ch in _S_PUNCT

if __name__ == '__main__':
  t = u"かたがな"
  print match_kata_hira_punc(t)

# EOF
