# coding: utf8
# jpmacros.py
# 1/15/2015 jichi

# Dump from Shared Dictionary at 1/15/2014
_bos_ = u'。？！…~～〜♪❤【】＜《（『「“'
_eos_ = u'。？！…~～〜♪❤【】＞》）』」”'

_boc_ = u'{{:bos:}}、，―─'.replace('{{:bos:}}', _bos_)
_eoc_ = u'{{:eos:}}、，―─'.replace('{{:eos:}}', _eos_)

boc = u"(?:^|(?<=[{{:boc:}}]))".replace('{{:boc:}}', _boc_)
eoc = u"(?:^|(?<=[{{:eoc:}}]))".replace('{{:eoc:}}', _eoc_)

bos = u"(?:^|(?<=[{{:bos:}}]))".replace('{{:bos:}}', _bos_)
eos = u"(?:^|(?<=[{{:eos:}}]))".replace('{{:eos:}}', _eos_)

_kanji_ = u'一-龠'
_hira_ = u'あ-ん'
_kata_ = u'ア-ヴ'
_kana_ = _hira_ + _kata_

kanji = '[%s]'.replace('%s', _kanji_)
hira = '[%s]'.replace('%s', _hira_)
kata = '[%s]'.replace('%s', _kata_)
kana = '[%s]'.replace('%s', _kana_)

if __name__ == '__main__':
  import re
  for it in boc, eoc, bos, eos:
    re.compile(it)

  t = u'す、す、すみません'
  _re_jitter = re.compile(boc + ur'([あ-んア-ヴ])(?=[、…]\1)')
  t = _re_jitter.sub('xxx', t)
  print t

# EOF
