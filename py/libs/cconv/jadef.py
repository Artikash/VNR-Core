# coding: utf8
# jadef.py
# 10/18/2014 jichi

if __name__ == '__main__':
  import sys
  sys.path.append('..')

from unitraits.uniconv import hira2kata

HIRA = u"""
あ   い   う   え   お
か   き   く   け   こ
さ   し   す   せ   そ
た   ち   つ   て   と
な   に   ぬ   ね   の
は   ひ   ふ   へ   ほ
ま   み   む   め   も
や        ゆ        よ
ら   り   る   れ   ろ
わ                  を
     ゐ        ゑ
ん

が   ぎ   ぐ   げ   ご
ざ   じ   ず   ぜ   ぞ
だ   ぢ   づ   で   ど
ば   び   ぶ   べ   ぼ
ぱ   ぴ   ぷ   ぺ   ぽ

               いぇ
うぁ うぃ      うぇ うぉ
すぁ すぃ すぅ すぇ すぉ
つぁ つぃ つぅ つぇ つぉ
とぁ とぃ とぅ とぇ とぉ
ふぁ ふぃ ふっ ふぇ ふぉ

ゔぁ ゔぃ ゔ   ゔぇ ゔぉ
どぁ どぃ どぅ どぇ どぉ

きゃ きぃ きゅ きぇ きょ
しゃ しぃ しゅ しぇ しょ
ちゃ ちぃ ちゅ ちぇ ちょ
てゃ てぃ てゅ てぇ てょ
にゃ にぃ にゅ にぇ にょ
ひゃ ひぃ ひゅ ひぇ ひょ
ふゃ      ふゅ      ふょ
みゃ みぃ みゅ みぇ みょ
りゃ りぃ りゅ りぇ りょ

ゔゃ      ゔゅ      ゔょ
ぎゃ ぎぃ ぎゅ ぎぇ ぎょ
じゃ じぃ じゅ じぇ じょ
ぢゃ ぢぃ ぢゅ ぢぇ ぢょ
でゃ でぃ でゅ でぇ でょ
びゃ びぃ びゅ びぇ びょ
ぴゃ ぴぃ ぴゅ ぴぇ ぴょ

ぁ ぃ ぅ ぇ ぉ
ゃ    ゅ    ょ
ゎ
ゕ       ゖ
っ
ー
"""

KATA = hira2kata(HIRA)

# Hepburn standard is used.
# See: https://en.wikipedia.org/wiki/Romanization_of_Japanese
# See: Transliterate/translation_maps.py
# Single っ is not handled, which will be processed later
ROMAJI = u"""
  a   i   u   e   o
 ka  ki  ku  ke  ko
 sa shi  su  se  so
 ta chi tsu  te  to
 na  ni  nu  ne  no
 ha  hi  fu  he  ho
 ma  mi  mu  me  mo
 ya      yu      yo
 ra  ri  ru  re  ro
 wa              wo
    wyi     wye
n

 ga  gi  gu  ge  go
 za  ji  zu  ze  zo
 da  ji  zu  de  do
 ba  bi  bu  be  bo
 pa  pi  pu  pe  po

             ye
wha whi     whe who
swa swi swu swe swo
tsa tsi tsu the tho
twa twi twu twe two
 fa  fi fwu  fe  fo

 va  vi  vu  ve  vo
dwa dwi dwu dwe dwo

kya kyi kyu kye kyo
sha shi shu she sho
cha chi chu che cho
tha thi thu the tho
nya nyi nyu nye nyo
hya hyi hyu hye hyo
fya     fyu     fyo
mya myi myu mye myo
rya ryi ryu rye ryo

vya     vyu     vyo
gya gyi gyu gye gyo
 ja  ji  ju  je  jo
dya dyi dyu dye dyo
dha dhi dhu dhe dho
bya byi byu bye byo
pya pyi pyu pye pyo

  a   i   u   e  o
 ya      yu     yo
 wa
 ka          ke
っ
-
"""

# http://www.geocities.jp/p451640/moji/skm/gjo/utfgjo_02.html
# http://www.geocities.jp/p451640/moji/skm/gjo/gjo_02.html
# Ambiguity:
# - Use 語中 instead of 語頭: か, た
# - Use 語頭 instead of 語中: ち
# - Missing: だ
HANGUL = u"""
아   이   우   에   오
카   키   쿠   케   코
사   시   스   세   소
타   치   쓰   테   토
나   니   누   네   노
하   히   후   헤   호
마   미   무   메   모
야        유        요
라   리   루   레   로
와                  로
     이        에
ㄴ

가   기   구   게   고
자   지   즈   제   조
다   지   즈   데   도
바   비   부   메   보
파   피   푸   페   포

               이에
우아 우이      우에 우오
사   시   스   세   소
두아 두이 두   두에 두오
도아 도이 도우 도에 도오
후아 후이 후우 후에 후오

부아 부이 부우 부에 부오
도아 도이 도우 도에 도오

캬   키   큐   케   쿄
샤   시   슈   세   쇼
차   치   추   최   초
테아 테이 테우 테   테오
냐   니   뉴   네   뇨
햐   히   휴   헤   효
후야      후유      후요
먀   미   뮤   메   묘
랴   리   류   레   료

부야      부유      부요
갸   기   규   게   교
자   지   주   제   조
자   지   주   제   조
데아 데이 데우 데   데오
뱌   비   뷰   메   뵤
퍄   피   퓨   페   표

ㅏ   ㅣ   ㅜ   ㅔ   ㅗ
ㅑ        ㅠ        ㅛ
와
카             케
-
ー
"""

# http://www.itagaki.net/trv/thai/language/Japanese_syllabary.htm
# http://www.thaismile.jp/ThaiLanguage/6/aiueo.html
# Ambiguity:
# - ふ หุ (ฟุ)
THAI = u"""
อะ	อิ	อุ	เอะ	โอะ
ขะ	ขิ	ขุ	เขะ	โขะ
สะ	ฉิ	สุ	เสะ	โสะ
ถะ	ฉิ	สึ	เถะ	โถะ
หนะ	หนิ	หนุ	เหนะ	โหนะ
หะ	หิ	หุ	เหะ	โหะ
หมะ	หมิ	หมุ	เหมะ	โหมะ
หยะ		หยุ		โหยะ
หละ	หลิ	หลุ	แหละ	โหละ
หวะ				โอ๊ะ
	อิ		เอะ
อึ้น

กะ	กิ	กุ	เกะ	โกะ
สะ	จิ	สุ	เสะ	โซะ
ดะ	จิ	สุ	เดะ	โดะ
บะ	บิ	บุ	เบะ	โบะ
พะ	พิ	พุ	เพะ	โพะ

			อิเอะ
อุอะ	อุอิ		อุเอะ	อุโอะ
สะ	ฉิ	สุ	เสะ	โสะ
สึอะ	สึอิ	สึ	สึเอะ	สึโอะ
โถะอะ	โถะอิ	โถะอุ	โถะเอะ	โถะ
หุอะ	หุอิ	หุอุ	หุเอะ	หุโอะ

บุอะ	บุอิ	บุอุ	บุเอะ	บุโอะ
โดะอะ	โดะอิ	โดะอุ	โดะเอะ	โดะ

เขียะ	ขิ	ขิอึ	เขะ	คิโอะ
ชะ	ฉิ	ชึ	เสะ	โชะ
ฉะ	ฉิ	ฉึ	เถะ	โฉะ
เถะอะ	เถะอิ	เถะอุ	เถะ	เถะโอะ
นยะ	หนิ	หยึ	เหนะ	โหยะ
เคียะ	หิ	คิอึ	เหะ	เคียว
หุหยะ		หุหยุ		หุโหยะ
เมียะ	หมิ	หมึ	เหมะ	มิโอะ
เรียะ	หลิ	ริว	แหละ	เรียว

บุหยะ		บุหยุ		บุโหยะ
เกียะ	กิ	กอึ	เกะ	โกอะ
จะ	จิ	จึ	เสะ	โจะ
จะ	จิ	จึ	เสะ	โจะ
เดะอะ	เดะอิ	เดะอุ	เดะ	เดะโอะ
เบียะ	บิ	บีอึ	เบะ	บีโอะ
เพียะ	พิ	พิอึ	เพะ	พิโอะ

อะ	อิ	อุ	เอะ	โอะ
หยะ		หยุ		โหยะ
หวะ
ขะ			ะ
-
-
"""

TABLES = {
  'hira': HIRA,
  'kata': KATA,
  'romaji': ROMAJI,
  'hangul': HANGUL,
  'thai': THAI,
}

if __name__ == '__main__':
  for k,v in TABLES.iteritems():
    print k,len(v.split())

  s = set()
  for i in HIRA.split():
    if i in s:
      print i
    s.add(i)

# EOF

#HIRA = u"""
#が ぎ ぐ げ ご ざ じ ず ぜ ぞ だ ぢ づ で ど ば び ぶ べ ぼ ぱ ぴ ぷ ぺ ぽ
#あ い う え お か き く け こ さ し す せ そ た ち つ て と
#な に ぬ ね の は ひ ふ へ ほ ま み む め も や ゆ よ ら り る れ ろ
#わ を ん ぁ ぃ ぅ ぇ ぉ ゃ ゅ ょ っ
#"""
#
#KANJI = u"""
#咖 鸡 古 给 沟 杂 鸡 足 则 走 大 鸡 度 的 多 扒 比 不 被 波 啪 屁 扑 配 破
#啊 伊 无 诶 哦 卡 其 库 可 口 萨 西 苏 塞 搜 他 气 子 太 脱
#哪 泥 奴 捏 诺 哈 西 夫 黑 后 马 米 木 美 摸 压 由 有 啦 里 撸 累 落
#瓦 欧 恩 啊 衣 乌 诶 偶 呀 由 有 -
#"""
