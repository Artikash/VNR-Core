# coffee.mk
# 8/8/2013 jichi
#include ../mk/env.mk

.PHONY: all compile compress clean

COFFEE = coffee
ifeq ($(OS),Windows_NT)
  CLOSURE = closure.cmd
else
  #CLOSURE = closure --compilation_level ADVANCED_OPTIMIZATIONS
  CLOSURE = closure
endif

DOS2UNIX = dos2unix

.coffee:
ifdef OUTDIR
	$(COFFEE) -c $(IN) -o $(OUTDIR)
else
	$(COFFEE) -c $(IN)
endif

.closure:
	$(CLOSURE) --js_output_file $(OUT) --js $(IN)

.null:
	echo null >> $(IN)
	$(DOS2UNIX) $(IN)

%.min.js: %.js
	$(MAKE) .closure IN=$^ OUT=$@

%.js: %.coffee
	$(MAKE) .coffee IN=$^

# EOF
