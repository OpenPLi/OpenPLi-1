FILES = \
	cables.xml satellites.xml terrestrial.xml

EXTRA_DIST = $(FILES)

all-local: $(FILES)

install-data-local: $(FILES)
	install -d $(targetprefix)/share/tuxbox
	@for i in $(FILES) ; do \
		install -m755 $$i $(targetprefix)/share/tuxbox ; \
	done
