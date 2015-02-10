# coding: utf8
# 2/9/2015 jichi
# See: http://online-translator.com
# See: http://translate.ru

MT_LANGUAGES = { # {str fr:[str to]}
  'ru': (      'en', 'de', 'fr', 'es', 'it', 'pt', 'ja', 'fi'),
  'en': ('ru',       'de', 'fr', 'es', 'it', 'pt', 'ja'),
  'de': ('ru', 'en',       'fr', 'es', 'it', 'pt'),
  'fr': ('ru', 'en', 'de',       'es'),
  'es': ('ru', 'en', 'de', 'fr'),
  'it': ('ru', 'en', 'de'),
  'pt': ('ru', 'en', 'de'),
  'ja': ('ru', 'en'),
  'fi': ('ru',), # Finn
}

def mt_lang_test(to=None, fr=None, online=True):
  """
  @param* to  str
  @param* fr  str
  @param* online  bool  ignored
  @return  bool
  """
  if fr and to:
    try: return to in MT_LANGUAGES[fr]
    except: return False
  else:
    return fr and fr in MT_LANGUAGES or to and to in MT_LANGUAGES

# EOF
