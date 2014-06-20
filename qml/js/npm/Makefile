# Makefile
# 8/8/2013 jichi

WGET = wget
ifeq ($(OS),Windows_NT)
  YUI = yuicompressor.cmd
  RJS = r.js.cmd
  #CLOSURE = closure.cmd
else
  YUI = yuicompressor
  RJS = r.js
  #CLOSURE = closure
  #CLOSURE = closure --compilation_level ADVANCED_OPTIMIZATIONS
endif

all: $(wildcard *.min.js)

# Close will remove the var variable
%.min.js: %.js
	> $@.1 sed 's/^exports\./var /'  $^
	#$(CLOSURE) --js $@.1 --js_output_file $@.2
	$(YUI) --type js -o $@.2 $@.1
	> $@ echo '.pragma library'
	>> $@ cat $@.2
	dos2unix $@
	rm $@.1 $@.2

# Closure is problematic to analyze haml.js
#%.min.js: %.r.js
#	$(CLOSURE) --js $^ --js_output_file $@

# https://npmjs.org/package/bbcode
bbcode.js:
	$(WGET) -O $@ http://rawgithub.com/ncb000gt/node-bbcode/master/lib/bbcode.js

# https://npmjs.org/package/dateformat
#dateformat.js:
#	$(WGET) -O $@ http://rawgithub.com/felixge/node-dateformat/master/lib/dateformat.js

# EOF
