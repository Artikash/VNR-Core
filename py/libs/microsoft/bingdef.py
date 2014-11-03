# coding: utf8
# bingdef.py
# 11/2/2014 jichi

LANG_LOCALES = {
  'zht': 'zh-CHT',
  'zhs': 'zh-CHS',
}
def lang2locale(lang):
  """
  @param  lang  unicode
  @return  unicode
  """
  return LANG_LOCALES.get(lang) or lang

# EOF
