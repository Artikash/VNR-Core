# Makefile
# 8/8/2013 jichi

all: Makefile compress

ifeq ($(OS),Windows_NT)
  CLOSURE = closure.cmd
else
  #CLOSURE = closure --compilation_level ADVANCED_OPTIMIZATIONS
  CLOSURE = closure
endif

compress: $(wildcard *.min.js)

%.min.js: %.js
	> $@.1 sed 's/^\.pragma .*//'  $^
	$(CLOSURE) --js $@.1 --js_output_file $@.2
	> $@ echo '.pragma library'
	>> $@ cat $@.2
	dos2unix $@
	rm $@.1 $@.2

clean:
	rm -f *.{1,2}

# EOF
