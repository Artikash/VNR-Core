# Makefile
# 8/8/2013 jichi
# http://compass-style.org/help/tutorials/command-line/

ifeq ($(OS),Windows_NT)
  COMPASS = compass.bat
  YUI = yuicompressor.cmd
else
  COMPASS = compass
  YUI = yuicompressor
endif

all: compress

# Do not work
#compile: $(wildcard sass/*.sass) $(wildcard sass/*/*.sass)
compile:
	$(COMPASS) compile

compress: compile $(wildcard css/*.min.css)

watch:
	$(COMPASS) watch

%.min.css: %.css
	$(YUI) --type css -o $@ $^
#$(YUI) -o '.css$$:.min.css' css/*.css

%.min.js: %.js
	$(YUI) --type js -o $@ $^
#$(YUI) -o '.js$$:.min.js' js/*.js
#$(YUI) -o '.css$$:.min.css' css/*.css

clean:

# EOF
