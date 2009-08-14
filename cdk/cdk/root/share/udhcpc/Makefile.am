FILES = \
	default.script

EXTRA_DIST = $(FILES)

all-local: $(FILES)

install-data-local: $(FILES)
	install -d $(targetprefix)/share/udhcpc
	@for i in $(FILES) ; do \
		install -m755 $$i $(targetprefix)/share/udhcpc ; \
	done
