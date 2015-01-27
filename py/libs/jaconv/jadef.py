# coding: utf8
# jadef.py
# 10/18/2014 jichi
# See: http://sakuradite.com/wiki/en/Furigana

if __name__ == '__main__':
  import sys
  sys.path.append('..')

from unitraits.uniconv import hira2kata

HIRA = u"""
あ	い	う	え	お
か	き	く	け	こ
さ	し	す	せ	そ
た	ち	つ	て	と
な	に	ぬ	ね	の
は	ひ	ふ	へ	ほ
ま	み	む	め	も
や		ゆ		よ
ら	り	る	れ	ろ
わ				を
	ゐ		ゑ
ん

が	ぎ	ぐ	げ	ご
ざ	じ	ず	ぜ	ぞ
だ	ぢ	づ	で	ど
ば	び	ぶ	べ	ぼ
ぱ	ぴ	ぷ	ぺ	ぽ

		いぇ
うぁ	うぃ		うぇ	うぉ
すぁ	すぃ	すぅ	すぇ	すぉ
つぁ	つぃ	つぅ	つぇ	つぉ
とぁ	とぃ	とぅ	とぇ	とぉ
ふぁ	ふぃ	ふぅ	ふぇ	ふぉ

ゔぁ	ゔぃ	ゔ	ゔぇ	ゔぉ
どぁ	どぃ	どぅ	どぇ	どぉ

きゃ	きぃ	きゅ	きぇ	きょ
しゃ	しぃ	しゅ	しぇ	しょ
ちゃ	ちぃ	ちゅ	ちぇ	ちょ
てゃ	てぃ	てゅ	てぇ	てょ
にゃ	にぃ	にゅ	にぇ	にょ
ひゃ	ひぃ	ひゅ	ひぇ	ひょ
ふゃ		ふゅ		ふょ
みゃ	みぃ	みゅ	みぇ	みょ
りゃ	りぃ	りゅ	りぇ	りょ

ゔゃ		ゔゅ		ゔょ
ぎゃ	ぎぃ	ぎゅ	ぎぇ	ぎょ
じゃ	じぃ	じゅ	じぇ	じょ
ぢゃ	ぢぃ	ぢゅ	ぢぇ	ぢょ
でゃ	でぃ	でゅ	でぇ	でょ
びゃ	びぃ	びゅ	びぇ	びょ
ぴゃ	ぴぃ	ぴゅ	ぴぇ	ぴょ

ぁ	ぃ	ぅ	ぇ	ぉ
ゃ		ゅ		ょ
ゎ
ゕ			ゖ
っ
ー
"""

# Specific for ん
HIRA_N = u"""
あん	いん	うん	えん	おん
かん	きん	くん	けん	こん
さん	しん	すん	せん	そん
たん	ちん	つん	てん	とん
なん	にん	ぬん	ねん	のん
はん	ひん	ふん	へん	ほん
まん	みん	むん	めん	もん
やん		ゆん		よん
らん	りん	るん	れん	ろん
わん				をん
	ゐん		ゑん

がん	ぎん	ぐん	げん	ごん
ざん	じん	ずん	ぜん	ぞん
だん	ぢん	づん	でん	どん
ばん	びん	ぶん	べん	ぼん
ぱん	ぴん	ぷん	ぺん	ぽん

		いぇん
うぁん	うぃん		うぇん	うぉん
すぁん	すぃん	すぅん	すぇん	すぉん
つぁん	つぃん	つぅん	つぇん	つぉん
とぁん	とぃん	とぅん	とぇん	とぉん
ふぁん	ふぃん	ふっん	ふぇん	ふぉん

ゔぁん	ゔぃん	ゔん	ゔぇん	ゔぉん
どぁん	どぃん	どぅん	どぇん	どぉん

きゃん	きぃん	きゅん	きぇん	きょん
しゃん	しぃん	しゅん	しぇん	しょん
ちゃん	ちぃん	ちゅん	ちぇん	ちょん
てゃん	てぃん	てゅん	てぇん	てょん
にゃん	にぃん	にゅん	にぇん	にょん
ひゃん	ひぃん	ひゅん	ひぇん	ひょん
ふゃん		ふゅん		ふょん
みゃん	みぃん	みゅん	みぇん	みょん
りゃん	りぃん	りゅん	りぇん	りょん

ゔゃん		ゔゅ		ゔょん
ぎゃん	ぎぃん	ぎゅ	ぎぇん	ぎょん
じゃん	じぃん	じゅ	じぇん	じょん
ぢゃん	ぢぃん	ぢゅ	ぢぇん	ぢょん
でゃん	でぃん	でゅ	でぇん	でょん
びゃん	びぃん	びゅ	びぇん	びょん
ぴゃん	ぴぃん	ぴゅ	ぴぇん	ぴょん
"""

KATA = hira2kata(HIRA)
KATA_N = hira2kata(HIRA_N)

# Hepburn standard is used.
# See: https://en.wikipedia.org/wiki/Romanization_of_Japanese
# See: Transliterate/translation_maps.py
# Single っ is not handled, which will be processed later
ROMAJI = u"""
a	i	u	e	o
ka	ki	ku	ke	ko
sa	shi	su	se	so
ta	chi	tsu	te	to
na	ni	nu	ne	no
ha	hi	fu	he	ho
ma	mi	mu	me	mo
ya		yu		yo
ra	ri	ru	re	ro
wa				wo
	wyi		wye
n

ga	gi	gu	ge	go
za	ji	zu	ze	zo
da	ji	zu	de	do
ba	bi	bu	be	bo
pa	pi	pu	pe	po

			ye
wha	whi		whe	who
swa	swi	swu	swe	swo
tsa	tsi	tsu	the	tho
twa	twi	twu	twe	two
fa	fi	fuu	fe	fo

va	vi	vu	ve	vo
dwa	dwi	dwu	dwe	dwo

kya	kyi	kyu	kye	kyo
sha	shi	shu	she	sho
cha	chi	chu	che	cho
tha	thi	thu	the	tho
nya	nyi	nyu	nye	nyo
hya	hyi	hyu	hye	hyo
fya		fyu		fyo
mya	myi	myu	mye	myo
rya	ryi	ryu	rye	ryo

vya		vyu		vyo
gya	gyi	gyu	gye	gyo
ja	ji	ju	je	jo
dya	dyi	dyu	dye	dyo
dha	dhi	dhu	dhe	dho
bya	byi	byu	bye	byo
pya	pyi	pyu	pye	pyo

a	i	u	e	o
ya		yu		yo
wa
ka			ke
っ
-
"""

# http://en.wikipedia.org/wiki/Russian_alphabet
# http://ru.wikipedia.org/wiki/Кана
# http://ru.wikipedia.org/wiki/Хирагана
# http://ru.wikipedia.org/wiki/Катакана
#
# a: а
# i: и(individual), not й(composite, delay to fix later after ауэояё
# e: э
# o: о
#
# u: у except yu
# yu: ю
#
# k: к
# ts: ц
# s: с
# ch: ч
# sh: с  or ш
#
# m: м
# n: н
# t: т
# h: х
# r: р
# f: ф
# p: п
# g: г
# z: З
# d: д
# b: б
# p: п
# v: иэ
#
# wa: ва
#
# ji: зи
# ya: я
# yo: ё
# ye: йё
#
# wh: у
# sw: су
# tw: ту
# dw: до
# th: ц / тэ
# dh: дэ
#
# Definitions different from wiki:
# - cha/chu/cho: ча/чу/чо is used instead of тя/тю/тё
# - з is used instead of дз
ROMAJI_RU = u"""
а	и	у	э	о
ка	ки	ку	кэ	ко
са	си	су	сэ	со
та	ти	цу	тэ	то
на	ни	ну	нэ	но
ха	хи	фу	хэ	хо
ма	ми	му	мэ	мо
я		ю		ё
ра	ри	ру	рэ	ро
ва				о
	ви		вэ
н

га	ги	гу	гэ	го
за	зи	зу	зэ	зо
да	зи	зу	дэ	до
ба	би	бу	бэ	бо
па	пи	пу	пэ	по

			иё
уа	уи		уэ	уо
суа	суй	суу	суэ	суо
ца	ци	цу	цэ	цо
туа	туй	туу	туэ	туо
фа	фи	фуу	фэ	фо

ва	ви	ву	вэ	во
доа	дой	доу	доэ	доо

кя	кий	кю	киэ	кё
ся	ши	сю	шэ	сё
ча	чи	чу	чэ	чо
тэа	тэй	тэю	тээ	тэё
ня	ний	ню	ниэ	нё
хя	хий	хю	хиэ	хё
фуя		фую		фуё
мя	мий	мю	миэ	мё
ря	рий	рю	риэ	рё

уя		вю		вё
гя	гий	гю	гиэ	гё
зя	зий	зю	зиэ	зё
зя	зий	зю	зиэ	зё
дэа	дэй	дэю	дээ	дэё
бя	бий	бю	биэ	бё
пя	пий	пю	пиэ	пё

а	и	у	э	о
я		ю		й
wа
ка			кэ
っ
-
"""

# http://blog.naver.com/thinkstart/120072326990
# http://www.geocities.jp/p451640/moji/skm/gjo/utfgjo_02.html
# http://www.geocities.jp/p451640/moji/skm/gjo/gjo_02.html
# Ambiguity:
# - Use 語中 instead of 語頭: か, た
# - Use 語頭 instead of 語中: ち
# - Missing: だ
HANGUL = u"""
아	이	우	에	오
카	키	쿠	케	코
사	시	스	세	소
타	치	츠	테	토
나	니	누	네	노
하	히	후	헤	호
마	미	무	메	모
야		유		요
라	리	루	레	로
와				오
	이		에
응

가	기	구	게	고
자	지	즈	제	조
다	지	즈	데	도
바	비	부	메	보
파	피	푸	페	포

			에
와	위		웨	워
사	시	수	세	소
차	치	추	체	초
톼	퇴	토	퇘	토
하	휘	훗	훼	호

바	비	부	베	보
다	디	두	데	도

캬	키	큐	케	쿄
샤	시	슈	세	쇼
챠	치	츄	체	쵸
탸	티	튜	테	툐
냐	니	뉴	네	뇨
햐	히	휴	헤	효
햐		휴		효
먀	미	뮤	메	묘
랴	리	류	레	료

뱌		뷰		뵤
갸	기	규	게	교
자	지	쥬	제	조
자	지	쥬	제	조
댜	디	듀	데	됴
뱌	비	뷰	메	뵤
퍄	피	퓨	페	표

ㅏ	ㅣ	ㅜ	ㅔ	ㅗ
ㅑ		ㅠ		ㅛ
와
카			케
-
ー
"""

HANGUL_N = """
앙	인	운	엔	온
칸	킨	군	켄	콘
산	신	슨	센	손
탄	친	츤	텐	톤
난	닌	눈	넨	논
한	힌	훈	헨	혼
만	민	문	멘	몬
얀		윤		욘
란	린	룬	렌	론
완				온
	인		엔

간	긴	군	겐	곤
잔	진	즌	젠	조
단	진	즌	덴	돈
반	빈	분	벤	본
판	핀	푼	펜	폰

		엔
완	윈		웬	온
산	신	순	센	손
찬	친	춘	첸	촌
탄	틴	툰	텐	톤
한	힌	훈	헨	혼

반	빈	분	벤	본
돤	된	둔	덴	돈

캰	킨	큔	켄	쿈
샨	신	슌	센	숀
챤	친	츈	첸	쵼
탼	틴	튠	텐	툔
냥	닌	뉸	넨	뇬
햔	힌	휸	헨	횬
햔		휸		횬
먄	민	뮨	멘	묜
랸	린	륜	렌	룐

뱐		뷴		뵨
갼	긴	균	겐	굔
쟌	진	쥰	젠	죤
쟌	진	쥰	젠	죤
댠	딘	듄	덴	됸
뱐	빈	뷴	벤	뵨
퍈	핀	퓬	펜	푠
"""

# http://th.wikipedia.org/wiki/คะนะ
# http://th.wikipedia.org/wiki/ฮิระงะนะ
# http://en.wikipedia.org/wiki/Thai_alphabet
# http://www.itagaki.net/trv/thai/language/Japanese_syllabary.htm
# http://www.thaismile.jp/ThaiLanguage/6/aiueo.html
#
# http://th.wikipedia.org/wiki/เทนโจอิง_อาซึกะ
#
# Issues:
# - su/zu: ซึ (すすら) or ซุ (อีซูซุ) or even ซูซุ for すず
THAI = u"""
อา	ย์	อุ	เอ	โอ
คา	กิ	คุ	เค	โก
ซา	ชิ	สึ	เซ	โซ
ตา	จิ	สึ	เท็	โท
นา	นิ	นุ	เนะ	โน
ฮา	ฮิ	ฟุ	เฮะ	โฮ
มา	มิ	มุ	เมะ	โม
ยา		ยุ		โย
รา	ริ	รุ	เร็	โร
ว				โว
	ย์		เอ
น

คา	คิ	กุ	เค	โก
ซา	จิ	ซุ	เซะ	โซ
ดา	จิ	ซึ	เท	โด
บา	บิ	บึ	เบ	โบ
พา	พิ	พึ	เพ	โพ

		เยะ
うぁ	วิ		เวะ	โว
すぁ	すぃ	สึ	すぇ	すぉ
สา	สิ	สึ	เสะ	โส
とぁ	とぃ	ทึ	とぇ	とぉ
ฟา	ฟิ	ふぅ	เฟะ	โฟ

ゔぁ	ゔぃ	ゔ	ゔぇ	ゔぉ
どぁ	どぃ	ดึ	どぇ	どぉ

きゃ	きぃ	きゅ	きぇ	きょ
しゃ	しぃ	しゅ	เชะ	しょ
ちゃ	ちぃ	ちゅ	เฉะ	ちょ
てゃ	ทิ	てゅ	てぇ	てょ
เนีย	にぃ	にゅ	にぇ	にょ
ひゃ	ひぃ	ひゅ	ひぇ	ひょ
ふゃ		ふゅ		ふょ
みゃ	みぃ	みゅ	みぇ	みょ
りゃ	りぃ	りゅ	りぇ	りょ

ゔゃ		ゔゅ		ゔょ
ぎゃ	ぎぃ	ぎゅ	ぎぇ	ぎょ
じゃ	じぃ	じゅ	เจะ	じょ
ぢゃ	ぢぃ	ぢゅ	ぢぇ	ぢょ
でゃ	でぃ	でゅ	でぇ	でょ
びゃ	びぃ	びゅ	びぇ	びょ
ぴゃ	ぴぃ	ぴゅ	ぴぇ	ぴょ

ぁ	ย์	อุ	เอ	โอ
ยา		ยุ		ょ
ゎ
ゕ			ゖ
っ
ー
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
