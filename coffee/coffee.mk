# coffee.mk
# 8/8/2013 jichi
#include ../mk/env.mk

ifeq ($(OS),Windows_NT)
  COFFEE = coffee
  CLOSURE = closure.cmd
else
  COFFEE = coffee
  #CLOSURE = closure --compilation_level ADVANCED_OPTIMIZATIONS
  CLOSURE = closure
endif

DOS2UNIX = dos2unix

.coffee:
	$(COFFEE) -c $(IN)

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
