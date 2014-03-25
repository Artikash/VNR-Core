# Makefile
# 8/8/2013 jichi

# http://lackof.org/taggart/hacking/make-example/
SUBDIRS = \
	py/apps/reader/mytr \
	py/apps/reader/tr \
    qml/reader/tr \
    qml/bootstrap2 \
    qml/bootstrap3 \
    js \
    coffee \
    rb/compass
    #css
    #py/libs/sakurakit/tr # too slow to make

.PHONY: all $(SUBDIRS)

all: $(SUBDIRS)

$(SUBDIRS):
	$(MAKE) -C $@

clean:
	@for it in $(SUBDIRS); do \
	$(MAKE) -C $$it $@; \
	done

# EOF
