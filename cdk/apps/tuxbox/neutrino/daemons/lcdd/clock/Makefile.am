EXTRA_DIST = *.bmp

install-data-local:
	install -d $(DATADIR)/lcdd/clock
	install -m 0644 $(srcdir)/$(EXTRA_DIST) $(DATADIR)/lcdd/clock

uninstall-local:
	rm -rf $(DATADIR)/lcdd
