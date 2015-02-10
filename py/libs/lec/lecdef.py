# coding: utf8
# lecdef.py
# 1/20/2015 jichi

DLL_BUFFER_SIZE = 0x2000 # output buffer size

LEC_LANGUAGES = frozenset((
  'ja',
  'en',
  'zh',
  'ko',
  'id',
  'ar',
  'de',
  'es',
  'fr',
  'it',
  'nl',
  'pl',
  'pt',
  'ru',
  #'ms',
  #'th',
  #'vi',
  'he', # Hebrew
  'fa', # Persian
  'tl', # Tagalog in Philippine
  'tr', # Turkish
  'uk', # Ukrainian
  'ps', # Pashto in Afghanistan
))

def mt_lang_test(to, fr='ja', online=True):
  """
  @param  to  str
  @param* fr  str
  @param* online  bool
  @return  bool
  """
  if online:
    return fr[:2] in LEC_LANGUAGES and to[:2] in LEC_LANGUAGES
  else: # Offline
    return fr == 'ja' and to in ('en', 'ru')

# EOF
