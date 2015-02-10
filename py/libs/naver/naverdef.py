# coding: utf8
# 2/9/2015 jichi
# See: http://translate.naver.com

TTS_LANGUAGES = frozenset((
  'en', 'zh', 'ja', 'ko',
))

MT_LANGUAGES = { # {str fr:[str to]}
  'ko': ('en', 'ja', 'zh'),
  'en': ('ko', 'pt', 'id', 'th'),
  'ja': ('ko',),
  'zh': ('ko',),
  'es': ('en',),
  'pt': ('en',),
  'id': ('en',),
  'th': ('en',),
}

def mt_lang_test(to, fr='ru'):
  """
  @param  to  str
  @param* fr  str
  @return  bool
  """
  try: return to[:2] in MT_LANGUAGES[fr[:2]]
  except: return True

def tts_lang_test(lang):
  """
  @param  lang  str
  @return  bool
  """
  return lang[:2] in TTS_LANGUAGES

# EOF
