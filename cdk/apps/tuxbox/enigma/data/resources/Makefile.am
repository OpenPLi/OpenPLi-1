EXTRA_DIST = rclargesilver.xml rcdm5xxx.xml \
	rcdreambox_inputdev.xml rcgeneric_inputdev.xml \
	keymappings.xml rcdreambox_keyboard.xml

install-data-local:
	install -d $(DESTDIR)$(TUXBOXDATADIR)/enigma/resources
	install -m 0644 $(EXTRA_DIST) $(DESTDIR)$(TUXBOXDATADIR)/enigma/resources
