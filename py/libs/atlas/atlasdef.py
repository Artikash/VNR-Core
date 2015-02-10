# coding: utf8
# 2/9/2015

def mt_lang_test(to=None, fr=None, online=False):
  """
  @param* to  str  language
  @param* fr  str  language
  @param* online  bool  ignored
  @return  bool
  """
  return (not fr or fr == 'ja') and (not to or to == 'en')

# EOF
