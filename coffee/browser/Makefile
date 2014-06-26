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

# Append null at the end for performance reason
# http://stackoverflow.com/questions/1409522/qt4-how-to-call-javascript-functions-in-a-page-from-c-via-qtwebkit
inject.min.js: inject.js
	$(CLOSURE) --js $^ --js_output_file $@
	echo null >> $@
	dos2unix $@
	rm -f $^

clean:

# EOF
