# coding: utf8
# 2/9/2015

MT_LANGUAGES = 'en', 'zh', 'ja'

def mt_lang_test(to=None, fr=None, online=False):
  """
  @param* to  str  language
  @param* fr  str  language
  @param* online  bool  ignored
  @return  bool
  """
  return (not fr or fr[:2] in MT_LANGUAGES) and (not to or to[:2] in MT_LANGUAGES)

# EOF
