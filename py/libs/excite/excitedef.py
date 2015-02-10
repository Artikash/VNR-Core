# coding: utf8
# 2/9/2015 jichi

MT_LANGUAGES = frozenset((
  'ja',
  'en',

  'zh',
  'ko',

  'fr',
  'de',
  'it',
  'es',
  'pt',
  'ru',
))

def mt_lang_test(to, fr='ja', online=True):
  """
  @param  to  str
  @param* fr  str
  @return  bool
  """
  return all((
    fr in ('ja', 'en') or to in ('ja', 'en'),
    fr[:2] in MT_LANGUAGES,
    to[:2] in MT_LANGUAGES,
  ))

# EOF
