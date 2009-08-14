if TARGET_CDK
SUBDIRS = \
	fonts \
	htdocs \
	pictures \
	resources \
	skins \
	templates \
	lirc \
	sysconfig
else
SUBDIRS = \
	fonts \
	htdocs \
	pictures \
	resources \
	skins \
	templates \
	lirc
endif

install-data-local:
	install -d $(DESTDIR)$(CONFIGDIR)/enigma
	install -m 0644 encoding.conf $(DESTDIR)$(CONFIGDIR)/enigma