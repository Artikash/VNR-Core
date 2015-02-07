# coding: utf8
# winlocale.py
# 11/19/2012 jichi

# MS LCID
# See: http://download.microsoft.com/download/9/5/E/95EF66AF-9026-4BB0-A41D-A4F81802D92C/%5BMS-LCID%5D.pdf
# See: http://msdn.microsoft.com/en-us/library/aa369771%28v=vs.85%29.aspx
# See: http://msdn.microsoft.com/en-us/goglobal/bb964664.aspx
LCID_NULL = 0x0
LCID_ZH_TW = 0x0404 # 1028
LCID_DE_DE = 0x0407 # 1031
LCID_EN_US = 0x0409 # 1033
LCID_FR_FR = 0x040C # 1036
LCID_IT_IT = 0x0410 # 1040
LCID_JA_JP = 0x0411 # 1041
LCID_KO_KR = 0x0412 # 1042
LCID_NL_NL = 0x0413 # 1043
LCID_PL_PL = 0x0415 # 1045
LCID_RU_RU = 0x0419 # 1049
LCID_ID_ID = 0x0421 # 1057
LCID_VI_VN = 0x042a # 1066
LCID_TH_TH = 0x041e # 1054
LCID_MS_MS = 0x044c # 1100
LCID_ZH_CN = 0x0804 # 2052
LCID_PT_PT = 0x0816 # 2070
LCID_ES_ES = 0x0C0A # 3082
LCID_AR_AR = 0x3801 # 14337, it is actually ar-ae. There is no LCID for ar_AR

LCID_LOCALE = {
  LCID_ZH_TW: 'zh_TW',
  LCID_ZH_CN: 'zh_CN',
  LCID_EN_US: 'en_US',
  LCID_JA_JP: 'ja_JP',
  LCID_KO_KR: 'ko_KR',
  LCID_TH_TH: 'th_TH',
  LCID_VI_VN: 'vi_VN',
  LCID_ID_ID: 'id_ID',
  LCID_DE_DE: 'de_DE',
  LCID_IT_IT: 'it_IT',
  LCID_NL_NL: 'nl_NL',
  LCID_PL_PL: 'pl_PL',
  LCID_RU_RU: 'ru_RU',
  LCID_FR_FR: 'fr_FR',
  LCID_PT_PT: 'pt_PT',
  LCID_ES_ES: 'es_ES',
  LCID_AR_AR: 'ar_AR',
}

#LCID_LOCALE2 = {k:v[:2] for k,v in LCID_LOCALE.iteritems()}

LOCALE_LCID = {v:k for k,v in LCID_LOCALE.iteritems()}

def lcid2locale(k): return LCID_LOCALE.get(k) or '' # long -> str
def locale2lcid(k): return LOCALE_LCID.get(k) or 0  # str ->long

# MS code page
# See: http://msdn.microsoft.com/en-us/library/windows/desktop/dd317756%28v=vs.85%29.aspx
# See: https://docs.moodle.org/dev/Table_of_locales
CODEPAGE_NULL = 0
CODEPAGE_UTF8 = 65001
CODEPAGE_UTF16 = 1200
CODEPAGE_SHIFT_JIS = 932 # ja_JP
CODEPAGE_GBK = 936       # zh_CN
CODEPAGE_EUC_KR = 949    # ko_KR
CODEPAGE_BIG5 = 950      # zh_CN
CODEPAGE_TIS620 = 874    # th_TH
CODEPAGE_CE = 1250       # Central/Eastern Europe
CODEPAGE_CYRILLIC = 1251 # ru_RU
CODEPAGE_LATIN1 = 1252   # en_US
CODEPAGE_AR = 1256       # ar_AR
CODEPAGE_VI = 1258       # vi_VN

CODEPAGE_ENCODING = {
  CODEPAGE_UTF8: 'utf-8',
  CODEPAGE_UTF16: 'utf-16',
  CODEPAGE_SHIFT_JIS: 'shift-jis',

  CODEPAGE_BIG5: 'big5',
  CODEPAGE_GBK: 'gbk',
  CODEPAGE_EUC_KR: 'euc-kr',
  CODEPAGE_TIS620: 'tis-620',

  CODEPAGE_LATIN1: 'latin1',

  CODEPAGE_CE: 'windows-1250',
  CODEPAGE_CYRILLIC: 'windows-1251',
  CODEPAGE_AR: 'windows-1256',
  CODEPAGE_VI: 'windows-1258',
}

ENCODING_CODEPAGE = {v:k for k,v in CODEPAGE_ENCODING.iteritems()}

CODEPAGE_ENCODING = {
  CODEPAGE_UTF8: 'utf-8',
  CODEPAGE_UTF16: 'utf-16',
  CODEPAGE_BIG5: 'big5',
  CODEPAGE_GBK: 'gbk',
  CODEPAGE_SHIFT_JIS: 'shift-jis',
  CODEPAGE_EUC_KR: 'euc-kr',
  CODEPAGE_TIS620: 'tis-620',
}

CODEPAGE_ENCODING_PY = {k:
  v if v.startswith('utf') else "cp%s" % k
  for k,v in CODEPAGE_ENCODING.iteritems()}

CODEPAGE_ENCODING_QT = {
  CODEPAGE_UTF8: 'utf-8',
  CODEPAGE_UTF16: 'utf-16',
  CODEPAGE_SHIFT_JIS: 'shift-jis',

  CODEPAGE_GBK: 'cp936',
  CODEPAGE_BIG5: 'cp950',
  CODEPAGE_EUC_KR: 'cp949',
  CODEPAGE_TIS620: 'cp874',

  CODEPAGE_LATIN1: 'windows-1252',

  CODEPAGE_CE: 'windows-1250',
  CODEPAGE_CYRILLIC: 'windows-1251',
  CODEPAGE_AR: 'windows-1256',
  CODEPAGE_VI: 'windows-1258',
}

ENCODING_PY = {v:CODEPAGE_ENCODING_PY[k] for k,v in CODEPAGE_ENCODING.iteritems()}
ENCODING_QT = {v:CODEPAGE_ENCODING_PY[k] for k,v in CODEPAGE_ENCODING.iteritems()}

def codepage2encoding(k): return CODEPAGE_ENCODING.get(k) or '' # long -> str
def encoding2codepage(k): return ENCODING_CODEPAGE.get(k) or ''  # str ->long

def encoding2py(k): return ENCODING_PY.get(k) or '' # str -> str
def encoding2qt(k): return ENCODING_PY.get(k) or '' # str -> str

LOCALE_CODEPAGE = {
  'zh_TW': CODEPAGE_BIG5,
  'zh_CN': CODEPAGE_GBK,
  'ja_JP': CODEPAGE_SHIFT_JIS,
  'ko_KR': CODEPAGE_EUC_KR,

  'th_TH': CODEPAGE_TIS620,
  'th_VI': CODEPAGE_VI,

  'ar_AR': CODEPAGE_AR,
  'ru_RU': CODEPAGE_CYRILLIC,
  'pl_PL': CODEPAGE_CE,

  'de_DE': CODEPAGE_LATIN1,
  'en_US': CODEPAGE_LATIN1,
  'es_ES': CODEPAGE_LATIN1,
  'fr_FR': CODEPAGE_LATIN1,
  'id_ID': CODEPAGE_LATIN1,
  'it_IT': CODEPAGE_LATIN1,
  'pt_PT': CODEPAGE_LATIN1,
  'ms_MS': CODEPAGE_LATIN1,
  'nl_NL': CODEPAGE_LATIN1,
}

def locale2codepage(k): return LOCALE_CODEPAGE.get(k) or 0  # str ->long

# EOF
