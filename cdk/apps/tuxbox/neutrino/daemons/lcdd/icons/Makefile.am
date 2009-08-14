EXTRA_DIST = neutrino_lcd.raw neutrino_power.raw neutrino_setup.raw

install-data-local:
	install -d $(DATADIR)/lcdd/icons
	install -m 0644 $(srcdir)/*.raw $(DATADIR)/lcdd/icons

uninstall-local:
	rm -rf $(DATADIR)/lcdd
