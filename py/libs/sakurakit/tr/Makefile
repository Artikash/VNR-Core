# Makefile
# 8/8/2013 jichi

ZHSZHT = opencc -c zhs2zhtw_vp.ini -i

.PHONY: all release clean

all: Makefile release zh_TW.ts

LOCALES = \
	ko_KR \
	th_TH \
	vi_VN \
	ms_MS \
	id_ID \
	ar_AR \
	de_DE \
	fr_FR \
	es_ES \
	it_IT \
	nl_NL \
	pl_PL \
	pt_PT \
	ru_RU
	#ja_JP
	#en_US
	#zh_TW
	#zh_CN

release: $(wildcard *.qm)

zh_TW.ts: zh_CN.ts
	$(ZHSZHT) $^ | sed 's/zh_CN/zh_TW/g' > $@

$(addsuffix .ts,$(LOCALES)): ja_JP.ts
	python ../../../scripts/tsupdate.py $@ $^

%.qm: %.ts
	dos2unix $^
	lrelease $^

clean:

# EOF
