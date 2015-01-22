# coding: utf8
# trtraits.py
# 11/21/2013 jichi

from mytr import mytr_

TRAITS = [ # [TranslatorTraits]
  {'key':'bing',    'online':True, 'name':"Bing.com"},
  {'key':'google',  'online':True, 'name':"Google.com"},
  {'key':'lecol',   'online':True, 'name':mytr_("LEC Online")},
  {'key':'infoseek','online':True, 'name':"Infoseek.co.jp"},
  {'key':'excite',  'online':True, 'name':"Excite.co.jp"},
  {'key':'transru', 'online':True, 'name':"Translate.Ru"},
  {'key':'baidu',   'online':True, 'name':mytr_("Baidu") + '.com'},
  {'key':'jbeijing','online':False, 'name':mytr_("JBeijing")},
  {'key':'fastait', 'online':False, 'name':mytr_("FastAIT")},
  {'key':'dreye',   'online':False, 'name':mytr_("Dr.eye")},
  {'key':'eztrans', 'online':False, 'name':mytr_("ezTrans XP")},
  {'key':'hanviet', 'online':False, 'name':u"Hán Việt"},
  {'key':'atlas',   'online':False, 'name':mytr_("ATLAS")},
  {'key':'lec',     'online':False, 'name':mytr_("LEC")},
  #{'key':'lou',     'online':False, 'name':u"ルー語"},
]

# EOF
