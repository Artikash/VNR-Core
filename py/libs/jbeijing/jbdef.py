# coding: utf8
# 2/9/2015

def mt_lang_test(to, fr='ja', online=False):
  """
  @param  to  str  language
  @param* fr  str  language
  @return  bool
  """
  return to.startswith('zh') and fr == 'ja'

# EOF
