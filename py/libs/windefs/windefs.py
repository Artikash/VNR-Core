# coding: utf8
# windefs.py
# 11/19/2012 jichi

# MS code page
# See: http://msdn.microsoft.com/en-us/library/windows/desktop/dd317756%28v=vs.85%29.aspx
CODEPAGE_NULL = 0
CODEPAGE_UTF8 = 65001
CODEPAGE_UTF16 = 1200
CODEPAGE_SHIFT_JIS = 932
CODEPAGE_GB2312 = 936
CODEPAGE_KS_C = 949
CODEPAGE_BIG5 = 950

# MS lcid
# See: http://download.microsoft.com/download/9/5/E/95EF66AF-9026-4BB0-A41D-A4F81802D92C/%5BMS-LCID%5D.pdf
# See: http://msdn.microsoft.com/en-us/library/aa369771%28v=vs.85%29.aspx
# See: http://msdn.microsoft.com/en-us/goglobal/bb964664.aspx
LCID_NULL = 0x0
LCID_ZH_TW = 0x404 # 1028
LCID_DE_DE = 0x407 # 1031
LCID_EN_US = 0x409 # 1033
LCID_FR_FR = 0x40C # 1036
LCID_IT_IT = 0x410 # 1040
LCID_JA_JP = 0x411 # 1041
LCID_KO_KR = 0x412 # 1042
LCID_PL_PL = 0x415 # 1045
LCID_RU_RU = 0x419 # 1049
LCID_ID_ID = 0x421 # 1057
LCID_VI_VN = 0x42a # 1066
LCID_TH_TH = 0x41e # 1054
LCID_ZH_CN = 0x804 # 2052
LCID_PT_PT = 0x816 # 2070
LCID_ES_ES = 0xC0A # 3082

LCID_LOCALES = {
  LCID_ZH_TW: 'zh-TW',
  LCID_ZH_CN: 'zh-CN',
  LCID_EN_US: 'en-US',
  LCID_JA_JP: 'ja-JP',
  LCID_KO_KR: 'ko-KR',
  LCID_TH_TH: 'th-TH',
  LCID_VI_VN: 'vi-VN',
  LCID_ID_ID: 'id-ID',
  LCID_DE_DE: 'de-DE',
  LCID_IT_IT: 'it-IT',
  LCID_PL_PL: 'pl-PL',
  LCID_RU_RU: 'ru-RU',
  LCID_FR_FR: 'fr-FR',
  LCID_PT_PT: 'pt-PT',
  LCID_ES_ES: 'es-ES',
}

LCID_LOCALES2 = {k:v[:2] for k,v in LCID_LOCALES.iteritems()}

# EOF
