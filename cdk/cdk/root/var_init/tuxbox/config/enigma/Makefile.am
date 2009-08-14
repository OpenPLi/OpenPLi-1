FILES = \
	config userbouquet.33fc5.tv userbouquet.94b4c.radio \
	userbouquets.radio.epl userbouquets.tv.epl

EXTRA_DIST = $(FILES)

all-local: $(FILES)

install-data-local: $(FILES)
	install -d  $(targetprefix)/var_init/tuxbox/config/enigma
	for i in $(FILES) ; do \
		install -m644 $$i $(targetprefix)/var_init/tuxbox/config/enigma; \
	done;

