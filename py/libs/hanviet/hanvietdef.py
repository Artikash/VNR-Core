# coding: utf8
# 2/10/2015 jichi

def mt_lang_test(to=None, fr=None, online=False):
  """
  @param* to  str  language
  @param* fr  str  language
  @param* online  bool  ignored
  return bool
  """
  return (not fr or fr.startswith('zh')) and (not to or to == 'vi')

# EOF
