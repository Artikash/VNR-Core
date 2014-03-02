# Makefile
# 8/8/2013 jichi

all: Makefile compress

ifeq ($(OS),Windows_NT)
  COFFEE = coffee
  CLOSURE = closure.cmd
else
  COFFEE = coffee
  #CLOSURE = closure --compilation_level ADVANCED_OPTIMIZATIONS
  CLOSURE = closure
endif

compress: $(wildcard *.min.js)
compile: $(wildcard *.js)

%.min.js: %.js
	$(CLOSURE) --js $^ --js_output_file $@

%.js: %.coffee
	$(COFFEE) -c $^

clean:

# EOF
