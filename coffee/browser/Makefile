# Makefile
# 8/8/2013 jichi

include ../coffee.mk

all: Makefile compress

compress: $(wildcard *.min.js)
compile: $(wildcard *.js)
clean:

# Append null at the end for performance reason
# http://stackoverflow.com/questions/1409522/qt4-how-to-call-javascript-functions-in-a-page-from-c-via-qtwebkit
inject.min.js: inject.js
	make .closure IN=$^ OUT=$@
	make .null IN=$@

# EOF
