# Makefile
# 8/8/2013 jichi

all: Makefile release zh_TW.ts

release: $(wildcard *.qm)

zh_TW.ts: zh_CN.ts
	zhs2zht $^ | sed 's/zh_CN/zh_TW/g' > $@

%.qm: %.ts
	dos2unix $^
	lrelease $^

clean:

# EOF
