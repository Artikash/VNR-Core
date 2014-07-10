# coffee.mk
# 8/8/2013 jichi

ifeq ($(OS),Windows_NT)
  COFFEE = coffee
  CLOSURE = closure.cmd
else
  COFFEE = coffee
  #CLOSURE = closure --compilation_level ADVANCED_OPTIMIZATIONS
  CLOSURE = closure
endif

%.min.js: %.js
	$(CLOSURE) --js $^ --js_output_file $@

%.js: %.coffee
	$(COFFEE) -c $^

# EOF
