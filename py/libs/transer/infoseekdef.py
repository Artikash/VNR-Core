# coding: utf8
# 2/9/2015 jichi

# According to userinfo.js:
# ja-zhs => JC
# zhs-ja => CJ
# ja-zht => JCT
# zht-ja => CJT, not CTJ!
#INFOSEEK_LCODE = {
#  'en' : 'E',
#  'ja' : 'J',
#  'zht': 'CT',
#  'zhs': 'C',
#  'ko' : 'K',
#  'fr' : 'F',
#  'de' : 'G',
#  'it' : 'I',
#  'es' : 'S',
#  'pt' : 'P',
#  'th' : 'ATHMS',   # thai
#  'vi' : 'AVIMS',   # vietnam
#  'id' : 'AIDMS',   # indonesia
#
#  'ru' : 'E', # Russia is not supported, use English instead
#}
#
#def _lang(to, fr):
#  """
#  @param  to  unicode
#  @param  fr  unicode
#  @return  unicode
#  """
#  return (
#      INFOSEEK_LCODE[fr] + INFOSEEK_LCODE[to] if fr != 'zht' else
#      'C%sT' % INFOSEEK_LCODE[to])
INFOSEEK_LCODE = {
  'jade': 'JG',
  'jaen': 'JE',
  'jaes': 'JS',
  'jafr': 'JF',
  'jait': 'JI',
  'jako': 'JK',
  'japt': 'JP',
  'jazhs': 'JC',
  'jazht': 'JCT',

  'jath': 'JATHMS',
  'javi': 'JAVIMS',
  'jaid': 'JAIDMS',
  'thja': 'THJAMS',
  'vija': 'VIJAMS',
  'idja': 'IDJAMS',

  'deen': 'GE',
  'dees': 'GS',
  'defr': 'GF',
  'deit': 'GI',
  'deja': 'GJ',
  'deko': 'GK',
  'dept': 'GP',
  'dezhs': 'GC',
  'dezht': 'GCT',

  'ende': 'EG',
  'enes': 'ES',
  'enfr': 'EF',
  'enit': 'EI',
  'enja': 'EJ',
  'enko': 'EK',
  'enpt': 'EP',
  'enzhs': 'EC',
  'enzht': 'ECT',

  'esde': 'SG',
  'esen': 'SE',
  'esfr': 'SF',
  'esit': 'SI',
  'esja': 'SJ',
  'esko': 'SK',
  'espt': 'SP',
  'eszhs': 'SC',
  'eszht': 'SCT',

  'frde': 'FG',
  'fren': 'FE',
  'fres': 'FS',
  'frit': 'FI',
  'frja': 'FJ',
  'frko': 'FK',
  'frpt': 'FP',
  'frzhs': 'FC',
  'frzht': 'FCT',

  'itde': 'IG',
  'iten': 'IE',
  'ites': 'IS',
  'itfr': 'IF',
  'itja': 'IJ',
  'itko': 'IK',
  'itpt': 'IP',
  'itzhs': 'IC',
  'itzht': 'ICT',

  'kode': 'KG',
  'koen': 'KE',
  'koes': 'KS',
  'kofr': 'KF',
  'koit': 'KI',
  'koja': 'KJ',
  'kopt': 'KP',
  'kozhs': 'KC',
  'kozht': 'KCT',

  'ptde': 'PG',
  'pten': 'PE',
  'ptes': 'PS',
  'ptfr': 'PF',
  'ptit': 'PI',
  'ptja': 'PJ',
  'ptko': 'PK',
  'ptzhs': 'PC',
  'ptzht': 'PCT',

  'zhsde': 'CG',
  'zhsen': 'CE',
  'zhses': 'CS',
  'zhsfr': 'CF',
  'zhsit': 'CI',
  'zhsja': 'CJ',
  'zhsko': 'CK',
  'zhspt': 'CP',

  'zhtde': 'CGT',
  'zhten': 'CET',
  'zhtes': 'CST',
  'zhtfr': 'CFT',
  'zhtit': 'CIT',
  'zhtja': 'CJT',
  'zhtko': 'CKT',
  'zhtpt': 'CPT',
}
def lang2lcode(to, fr):
  """
  @param  to  unicode
  @param  fr  unicode
  @return  unicode
  """
  return INFOSEEK_LCODE.get(fr + to) or 'JE' # from 'ja' to 'en'

def lang_test(to, fr='ja'):
  """
  @param  to  str
  @param* fr  str
  @return  bool
  """
  if to == 'zh':
    to = 'zht'
  if fr == 'zh':
    fr = 'zht'
  return (fr + to) in INFOSEEK_LCODE

# EOF
