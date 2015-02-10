# coding: utf8
# 2/9/2015

MT_LANGUAGES = 'en', 'zh', 'ja'

def mt_lang_test(to, fr='ja', online=False):
  """
  @param  to  str  language
  @param* fr  str  language
  @return  bool
  """
  return fr[:2] in MT_LANGUAGES and to[:2] in MT_LANGUAGES

# EOF
