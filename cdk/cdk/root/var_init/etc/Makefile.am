FILES = \
	group hostname hosts passwd smb.conf

EXTRA_DIST = $(FILES)

all-local: $(FILES)

install-data-local: $(FILES)
	install -d $(targetprefix)/var_init/etc
	@for i in $(FILES) ; do \
		install -m644 $$i $(targetprefix)/var_init/etc ; \
	done

