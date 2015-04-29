# Makefile
# 8/8/2013 jichi

ZHSZHT = opencc -c s2twp.json -i

.PHONY: all release clean

all: Makefile release zh_TW.ts

# Language short name in Japanese
# http://ja.wikipedia.org/wiki/国名の漢字表記一覧

LOCALES = \
	ko_KR \
	th_TH \
	vi_VN \
	id_ID \
	ms_MY \
	ar_SA \
	cs_CS \
	da_DA \
	de_DE \
	el_EL \
	fi_FI \
	fr_FR \
	es_ES \
	hu_HU \
	it_IT \
	no_NO \
	nl_NL \
	pl_PL \
	pt_PT \
	ro_RO \
	ru_RU \
	sk_SK \
	sv_SV \
	tr_TR \
	uk_UA
	#ja_JP
	#en_US
	#zh_TW
	#zh_CN

release: $(wildcard *.qm)

zh_TW.ts: zh_CN.ts
	$(ZHSZHT) $^ | sed 's/zh_CN/zh_TW/g' | sed 's/指令碼/脚本/g' > $@

$(addsuffix .ts,$(LOCALES)): ja_JP.ts
	python ../../../scripts/tsupdate.py $@ $^

%.qm: %.ts
	dos2unix $^
	lrelease $^

clean:

# EOF
