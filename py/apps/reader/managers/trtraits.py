# coding: utf8
# trtraits.py
# 11/21/2013 jichi

from mytr import mytr_

TRAITS = [ # [TranslatorTraits]
  {'key':'bing',        'name':"Bing.com",              },
  {'key':'google',      'name':"Google.com",            },
  {'key':'lecol',       'name':mytr_("LEC Online"),     },
  {'key':'infoseek',    'name':"Infoseek.co.jp",        },
  {'key':'excite',      'name':"Excite.co.jp",          },
  {'key':'transru',     'name':"Translate.Ru",          },
  {'key':'naver',       'name':"Naver.com",             },
  {'key':'baidu',       'name':mytr_("Baidu") + ".com", },
  {'key':'jbeijing',    'name':mytr_("JBeijing"),       },
  {'key':'fastait',     'name':mytr_("FastAIT"),        },
  {'key':'dreye',       'name':mytr_("Dr.eye"),         },
  {'key':'eztrans',     'name':mytr_("ezTrans XP"),     },
  {'key':'atlas',       'name':mytr_("ATLAS"),          },
  {'key':'lec',         'name':mytr_("LEC"),            },
  {'key':'hanviet',     'name':u"Hán Việt",             },
]

def gettrait(key): # str -> dict or None
  for it in TRAITS:
    if it['key'] == key:
      return it

# EOF
