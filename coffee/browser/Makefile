# Makefile
# 8/8/2013 jichi
include ../coffee.mk

all: Makefile compress

#SOURCE = $(shell find . -name '*.coffee')
SOURCE = $(wildcard *.coffee)

compress: $(SOURCE:.coffee=.min.js)
compile: $(SOURCE:.coffee=.js)
clean:
	rm -f $(SOURCE:.coffee=.js)

# Append null at the end for performance reason
# http://stackoverflow.com/questions/1409522/qt4-how-to-call-javascript-functions-in-a-page-from-c-via-qtwebkit
inject.min.js: inject.js
	$(MAKE) .closure IN=$^ OUT=$@
	$(MAKE) .null IN=$@


browser.js: ../share/annot.coffee browser.coffee
	$(COFFEE) -c -j $@ $^

# EOF
