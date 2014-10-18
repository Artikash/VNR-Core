# coding: utf8
# jaconv.py
# 12/30/2012 jichi

if __name__ == '__main__':
  import sys
  sys.path.append("..")

import re
from itertools import imap
import jadef

def __make_table(f_list, t_list):
  assert len(f_list) == len(t_list), "list size mismatch"
  return dict(zip(f_list, t_list))

def __merge_dict(x, y): return dict(x.items() + y.items())

@memoized
def _hira_hangul_table(): return __make_table(_hira_list(), _hangul_list())
@memoized
def _kata_hangul_table(): return __make_table(_kata_list(), _hangul_list())
@memoized
def _kana_hangul_table(): return __merge_dict(_hira_hangul_table(), _kata_hangul_table())

@memoized
def _hira_thai_table(): return __make_table(_hira_list(), _thai_list())
@memoized
def _kata_thai_table(): return __make_table(_kata_list(), _thai_list())
@memoized
def _kana_thai_table(): return __merge_dict(_hira_thai_table(), _kata_thai_table())

## Functions ##

__regex_storage = {} # {long id: re}, cached regex
def _multiple_replace(text, table):
  key = id(table) # so that the table itself will not be saved
  rx = __regex_storage.get(key)
  if not rx:
    pattern = '|'.join(imap(re.escape, table))
    rx = re.compile(pattern)
    __regex_storage[key] = rx
  def proc_one(match):
    return table[match.group()]
  return rx.sub(proc_one, text)

def convert(text, table, encoding='utf8'):
  uflag = isinstance(text, unicode)

  ret = uflag and text or text.decode(encoding, errors='ignore') # enforce unicode
  if not ret:
    return text

  ret = _multiple_replace(ret, table)
  return uflag and ret or ret.encode(encoding)

def hira2hangul(text): return _template(text, 'hira', 'hangul')

#def kata2hangul(text): return convert(text, _kata_hangul_table())
#def kana2hangul(text): return convert(text, _kana_hangul_table())
#def hira2thai(text): return convert(text, _hira_thai_table())
#def kata2thai(text): return convert(text, _kata_thai_table())
#def kana2thai(text): return convert(text, _kana_thai_table())
#
## See: http://pypi.python.org/pypi/jTransliterate
#from jTransliterate import JapaneseTransliterator
#def kana2romaji(text):
#  return JapaneseTransliterator(text).transliterate_from_hrkt_to_latn()
#
#kata2romaji = kana2romaji
#hira2romaji = kana2romaji

if __name__ == '__main__':
  print kata2hira(u'バカだ?')
  print hira2kata(u'バカだ?')
  #print kata2kanji(u'ワタシハ!')
  #print zhs2zht(u'可爱!!')

  print wide2thin(u'あの１２３４５０９ｂ！！［[。、')
  print thin2wide(u'あの12344０９ｂ?！')

  print wide2thin_digit(u'あの12344０９ｂ?！')

  print hira2thai(u'バカだ?')
  print kata2thai(u'バカだ?')
  print kana2thai(u'バカだ?')

  print hira2hangul(u'バカだ?')
  print kata2hangul(u'バカだ?')
  print kana2hangul(u'バカだ?')

  #print kana2kanji(u'かわいい?')
  print kana2hangul(u'かわいい?')
  print kana2thai(u'かわいい?')

# EOF

#def hira_list(): # meaningless kanji > <
#  return u"""\
#が ぎ ぐ げ ご ざ じ ず ぜ ぞ だ ぢ づ で ど ば び ぶ べ ぼ ぱ ぴ ぷ ぺ ぽ
#あ い う え お か き く け こ さ し す せ そ た ち つ て と
#な に ぬ ね の は ひ ふ へ ほ ま み む め も や ゆ よ ら り る れ ろ
#わ を ん ぁ ぃ ぅ ぇ ぉ ゃ ゅ ょ っ
#""".split()

# Consistent with jpchars
#def _kanji_list(): # meaningless kanji > <
#  return u"""\
#咖 鸡 古 给 沟 杂 鸡 足 则 走 大 鸡 度 的 多 扒 比 不 被 波 啪 屁 扑 配 破
#啊 伊 无 诶 哦 卡 其 库 可 口 萨 西 苏 塞 搜 他 气 子 太 脱
#哪 泥 奴 捏 诺 哈 西 夫 黑 后 马 米 木 美 摸 压 由 有 啦 里 撸 累 落
#瓦 欧 恩 啊 衣 乌 诶 偶 呀 由 有 -
#""".split()
