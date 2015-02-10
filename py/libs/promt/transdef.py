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

def mt_lang_test(to, fr='ru', online=True):
  """
  @param  to  str
  @param* fr  str
  @return  bool
  """
  try: return to in MT_LANGUAGES[fr]
  except: return True

# EOF
