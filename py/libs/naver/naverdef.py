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

def tts_lang_test(lang):
  """
  @param  lang  str
  @return  bool
  """
  return lang[:2] in TTS_LANGUAGES

def mt_lang_test(to=None, fr=None, online=True):
  """
  @param* to  str
  @param* fr  str
  @param* online  bool  ignored
  @return  bool  ignored
  """
  if fr and to:
    try: return to[:2] in MT_LANGUAGES[fr[:2]]
    except: return False
  else:
    return fr and fr[:2] in MT_LANGUAGES or to and to[:2] in MT_LANGUAGES

# EOF
