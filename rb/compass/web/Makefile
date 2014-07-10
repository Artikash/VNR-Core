# Makefile
# 8/8/2013 jichi
include ../compass.mk

COMPASS_COMMANDS = compile watch clean

all: compress

$(COMPASS_COMMANDS):
	$(COMPASS) $@

compress: compile $(wildcard css/*.min.css)

# EOF
