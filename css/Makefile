# Makefile
# 8/8/2013 jichi

ifeq ($(OS),Windows_NT)
  YUI = yuicompressor.cmd
else
  YUI = yuicompressor
endif

all: compress

compress: $(wildcard *.min.css)

%.min.css: %.css
	$(YUI) --type css -o $@ $^
#$(YUI) -o '.css$$:.min.css' css/*.css

%.min.js: %.js
	$(YUI) --type js -o $@ $^
#$(YUI) -o '.js$$:.min.js' js/*.js
#$(YUI) -o '.css$$:.min.css' css/*.css

clean:

# EOF
