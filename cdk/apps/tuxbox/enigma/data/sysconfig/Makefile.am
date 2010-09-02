EXTRA_DIST =  \
	Adelaide Almaty Anchorage Atlantic Auckland Azores Baghdad \
	Baku Bangkok Bogota Brisbane Calcutta Cape_Verde Caracas \
	Casablanca CET Colombo CST6CDT Danmarkshavn Darwin Dhaka Dublin \
	East EST EST5EDT Fiji Guam Harare Hobart Hong_Kong Honolulu \
	Irkutsk Istanbul Jerusalem Kabul Katmandu Kinshasa Krasnoyarsk \
	Kuala_Lumpur Kwajalein Magadan Midway Moscow MST MST7MDT Muscat \
	Nairobi Newfoundland Noronha Omsk Perth Rangoon Riyadh Santiago \
	Saskatchewan Seoul Sydney Taipei Tashkent Tehran Tijuana Tokyo \
	Tongatapu Vladivostok West Yakutsk

install-data-local:
	sh -c 'f=`pwd` ; \
		tar jxf $$f/zoneinfo.tar.bz2 '
	$(INSTALL) -d $(datadir)/zoneinfo
	$(INSTALL_DATA) $(EXTRA_DIST) $(datadir)/zoneinfo
	rm $(EXTRA_DIST)
	$(INSTALL) -d $(sysconfdir)
	ln -sf /var/etc/localtime $(sysconfdir)/localtime

uninstall-local:
	-rm $(datadir)/zoneinfo/$(EXTRA_DIST)
	-rm $(sysconfdir)/localtime
