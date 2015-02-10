# coding: utf8
# trtraits.py
# 11/21/2013 jichi

from mytr import mytr_

TRAITS = [ # [TranslatorTraits]
  {'key':'bing',        'name':"Bing.com",              'online':True,  'align':True,   'script':True,  },
  {'key':'google',      'name':"Google.com",            'online':True,  'align':True,   'script':True,  },
  {'key':'lecol',       'name':mytr_("LEC Online"),     'online':True,  'align':False,  'script':True,  },
  {'key':'infoseek',    'name':"Infoseek.co.jp",        'online':True,  'align':True,   'script':True,  },
  {'key':'excite',      'name':"Excite.co.jp",          'online':True,  'align':False,  'script':True,  },
  {'key':'transru',     'name':"Translate.Ru",          'online':True,  'align':False,  'script':True,  },
  {'key':'naver',       'name':"Naver.com",             'online':True,  'align':True,   'script':False, },
  {'key':'baidu',       'name':mytr_("Baidu") + ".com", 'online':True,  'align':True,   'script':False, },
  {'key':'jbeijing',    'name':mytr_("JBeijing"),       'online':False, 'align':False,  'script':False, },
  {'key':'fastait',     'name':mytr_("FastAIT"),        'online':False, 'align':False,  'script':False, },
  {'key':'dreye',       'name':mytr_("Dr.eye"),         'online':False, 'align':False,  'script':False, },
  {'key':'eztrans',     'name':mytr_("ezTrans XP"),     'online':False, 'align':False,  'script':False, },
  {'key':'atlas',       'name':mytr_("ATLAS"),          'online':False, 'align':False,  'script':True,  },
  {'key':'lec',         'name':mytr_("LEC"),            'online':False, 'align':False,  'script':True,  },
  {'key':'hanviet',     'name':u"Hán Việt",             'online':False, 'align':True,   'script':False, },
]

# EOF
