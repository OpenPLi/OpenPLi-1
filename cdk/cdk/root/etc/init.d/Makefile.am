FILES = rcS rcK start init1

EXTRA_DIST = $(FILES)

all-local: $(FILES)

install-data-local: $(FILES)
	install -d $(targetprefix)/etc/init.d
	@for i in $(FILES) ; do \
		install -m755 $$i $(targetprefix)/etc/init.d ; \
	done
