EXTRA_DIST = \
	deutsch.locale english.locale french.locale polski.locale \
	slovensky.locale tuerkce.locale

install-data-local:
	install -d $(DATADIR)/neutrino/locale
	install -m 0644 $(srcdir)/*.locale $(DATADIR)/neutrino/locale

uninstall-local:
	-rm -rf $(DATADIR)/neutrino/locale
	-rmdir $(DATADIR)/neutrino
