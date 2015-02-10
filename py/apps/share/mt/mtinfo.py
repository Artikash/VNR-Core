# coding: utf8
# 2/10/2015

MT_INFO = {
  'bing',        'online':True,  'align':True,   'script':True,  },
  'google',      'online':True,  'align':True,   'script':True,  },
  'lecol',       'online':True,  'align':False,  'script':True,  },
  'infoseek',    'online':True,  'align':True,   'script':True,  },
  'excite',      'online':True,  'align':False,  'script':True,  },
  'transru',     'online':True,  'align':False,  'script':True,  },
  'naver',       'online':True,  'align':True,   'script':False, },
  'baidu',       'online':True,  'align':True,   'script':False, },
  'jbeijing',    'online':False, 'align':False,  'script':False, },
  'fastait',     'online':False, 'align':False,  'script':False, },
  'dreye',       'online':False, 'align':False,  'script':False, },
  'eztrans',     'online':False, 'align':False,  'script':False, },
  'atlas',       'online':False, 'align':False,  'script':True,  },
  'lec',         'online':False, 'align':False,  'script':True,  },
  'hanviet',     'online':False, 'align':True,   'script':False, },
}

def test_online(key):
  """
  @param  key  str
  @return  bool
  """
  try: MT_INFO[key]['online']
  except KeyError: return False

def test_align(key):
  """
  @param  key  str
  @return  bool
  """
  try: MT_INFO[key]['align']
  except KeyError: return False

def test_lang(key, to, fr='ja'):
  """
  @param  key  str
  @param  to  str  language
  @param* fr  str  language
  @return bool
  """
  if key == 'hanviet':
    return fr.startswith('zh') and to == 'vi'
  if key in ('google', 'bing'):
    return True
  f = get_mod_def(key)
  if f:
    online = test_online(key)
    return f.mt_lang_test(to=to, fr=fr, online=online)
  return False

def get_mod_def(key):
  """
  @param  key  str
  @return  module
  """
  if key == 'atlas':
    from atlas import atlasdef
    return atlasdef
  if key in ('lec', 'lecol'):
    from lec import lecdef
    return lecdef
  if key == 'eztrans':
    from eztrans import ezdef
    return ezdef
  if key == 'jbeijing':
    from jbeijing import jbdef
    return jbdef
  if key == 'dreye':
    from dreye import dreyedef
    return dreye
  if key in ('fastait', 'iciba'):
    from kingsoft import icibadef
    return icibadef
  if key == 'baidu':
    from baidu import baidudef
    return baidudef
  if key == 'naver':
    from naver import naverdef
    return naverdef
  if key == 'infoseek':
    from transer import infoseekdef
    return infoseekdef
  if key == 'excite':
    from excite import excitedef
    return excitedef
  if key == 'transru':
    from promt import transdef
    return transdef
  if key == 'google':
    from google import googledef
    return googledef
  if key == 'bing':
    from bing import bingdef
    return bingdef
  if key == 'hanviet':
    from hanviet import hanviet
    return hanviet
  return None

# EOF
