# qmljs.mk
# 8/8/2013 jichi

.PHONY: all clean compile compress download

WGET = wget
ifeq ($(OS),Windows_NT)
  CLOSURE = closure.cmd
  YUI = yuicompressor.cmd
else
  #CLOSURE = closure --compilation_level ADVANCED_OPTIMIZATIONS
  CLOSURE = closure
  YUI = yuicompressor
endif

.wget:
	$(WGET) -O $(OUT) $(IN)

.yui:
ifdef TYPE
	$(YUI) --type $(TYPE) -o $(OUT) $(IN)
else
	$(YUI) -o $(OUT) $(IN)
  endif

.closure:
	$(CLOSURE) --js_output_file $(OUT) --js $(IN)

.clean:
	rm -f *.{1,2}

.pragma:
	> $(OUT).1 sed 's/^\.pragma .*//' $(IN)
	make .closure IN=$(OUT).1 OUT=$(OUT).2
	> $(OUT) echo '.pragma library'
	>> $(OUT) cat $(OUT).2
	dos2unix $(OUT)
	rm $(OUT).1 $(OUT).2

# EOF
